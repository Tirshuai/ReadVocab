#include "reviewsituationwidget.h"
#include <QVBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QDialog>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <algorithm>
#include <QSqlDatabase>
#include <QDateTime>

ReviewSituationWidget::ReviewSituationWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("情境复习模式");
    setFixedSize(680, 780); // 统一大窗口

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setSpacing(20);
    lay->setContentsMargins(30, 30, 30, 30);

    // 标题
    title = new QLabel(" AI 情景短文复习");
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(22);
    titleFont.setBold(true);
    title->setFont(titleFont);

    // 生成按钮
    btnGenerate = new QPushButton("抽取若干单词 + 生成AI短文");
    btnGenerate->setFixedHeight(50);

    // 文本展示
    textBrowser = new QTextBrowser();
    textBrowser->setStyleSheet(R"(
        QTextBrowser {
            background-color: #000000;
            color: #ffffff;
            font-size: 17px;
            line-height: 1.7;
            padding: 16px;
            border-radius: 8px;
        }
        a {
            color: #4fc3f7;
            font-weight: bold;
            text-decoration: none;
        }
    )");
    textBrowser->setOpenLinks(false);
    textBrowser->setOpenExternalLinks(false);

    // 返回按钮
    btnBack = new QPushButton("← 返回复习主页");
    btnBack->setFixedHeight(50);

    // 布局
    lay->addWidget(title);
    lay->addWidget(btnGenerate);
    lay->addWidget(textBrowser);
    lay->addWidget(btnBack);

    // 信号槽
    connect(textBrowser, &QTextBrowser::anchorClicked, this, &ReviewSituationWidget::onWordClicked);
    connect(btnBack, &QPushButton::clicked, this, &ReviewSituationWidget::onBack);
    connect(btnGenerate, &QPushButton::clicked, this, &ReviewSituationWidget::startGenerateScenario);

    // 数据库
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    query = new QSqlQuery(db);

    // AI
    aiGenerator = new AiTextGenerator(this);
    connect(aiGenerator, &AiTextGenerator::articleGenerated, this, &ReviewSituationWidget::onArticleGenerated);
    connect(aiGenerator, &AiTextGenerator::requestError, this, [this](QString err){
        textBrowser->setText("❌ 生成失败：" + err);
    });

    textBrowser->setText("点击上方按钮开始复习 → 抽取若干单词\n\nAI将生成短文帮助你巩固记忆～");
}

// 获取最近学习的5个单词
QStringList ReviewSituationWidget::getLatest5LearnedWords()
{
    QStringList res;
    query->exec(R"(
        SELECT v.word
        FROM vocabulary v
        JOIN learn_record r ON v.id = r.word_id
        ORDER BY r.study_date DESC
        LIMIT 5
    )");

    while (query->next()) {
        res << query->value(0).toString();
    }
    return res;
}

// 开始生成
void ReviewSituationWidget::startGenerateScenario()
{
    currentWords = getLatest5LearnedWords();

    if (currentWords.size() < 1) {
        textBrowser->setText("❌ 暂无已学习单词，无法复习！");
        return;
    }

    textBrowser->setText("AI 正在生成复习短文，请稍候...\n\n本次复习单词：\n" + currentWords.join(", "));
    aiGenerator->generateStoryArticle(currentWords);
}

// 短文生成完成
void ReviewSituationWidget::onArticleGenerated(const QString &article)
{
    QString html = article;
    for (const QString &word : currentWords) {
        QRegularExpression re("\\b" + QRegularExpression::escape(word) + "\\b");
        html.replace(re, QString("<a href='word://%1'>%1</a>").arg(word));
    }
    textBrowser->setHtml("<h3>复习短文</h3>" + html);
}

// 点击单词 → 测验
void ReviewSituationWidget::onWordClicked(const QUrl &url)
{
    QString word = url.host();
    if (url.scheme() != "word" || word.isEmpty()) return;

    int wordId;
    QString meaning;
    loadWordMeaning(word, wordId, meaning);

    if (wordId == -1) {
        QMessageBox::warning(this, "错误", "未找到该单词");
        return;
    }

    showQuizDialog(wordId, word, meaning);
}

// 加载单词信息
void ReviewSituationWidget::loadWordMeaning(const QString &word, int &outWordId, QString &outMeaning)
{
    outWordId = -1;
    outMeaning.clear();

    query->prepare("SELECT id, meaning FROM vocabulary WHERE word = ? LIMIT 1");
    query->addBindValue(word);

    if (!query->exec() || !query->next()) return;

    outWordId = query->value(0).toInt();
    outMeaning = query->value(1).toString();
}

// 弹出测验对话框（完全仿照你的原版）
void ReviewSituationWidget::showQuizDialog(int wordId, const QString &word, const QString &correctMeaning)
{
    QDialog d(this);
    d.setWindowTitle("情境复习测验");
    d.setFixedSize(580, 520);

    QVBoxLayout *layout = new QVBoxLayout(&d);
    layout->setSpacing(22);
    layout->setContentsMargins(40, 40, 40, 40);

    QLabel *labWord = new QLabel(word);
    labWord->setAlignment(Qt::AlignCenter);
    QFont f;
    f.setPointSize(32);
    f.setBold(true);
    labWord->setFont(f);
    layout->addWidget(labWord);

    // 随机错误选项
    QStringList options;
    query->prepare("SELECT meaning FROM vocabulary WHERE word != ? ORDER BY RANDOM() LIMIT 3");
    query->addBindValue(word);
    query->exec();
    while (query->next()) options << query->value(0).toString();
    options << correctMeaning;
    std::shuffle(options.begin(), options.end(), *QRandomGenerator::global());

    QPushButton *optA = new QPushButton("1. " + options[0]);
    QPushButton *optB = new QPushButton("2. " + options[1]);
    QPushButton *optC = new QPushButton("3. " + options[2]);
    QPushButton *optD = new QPushButton("4. " + options[3]);

    QList<QPushButton*> opts = {optA, optB, optC, optD};
    for (auto btn : opts) {
        btn->setMinimumHeight(70);
        btn->setStyleSheet("text-align: left; padding:14px; font-size:16px;");
        layout->addWidget(btn);
    }

    // 点击判断
    for (auto btn : opts) {
        connect(btn, &QPushButton::clicked, &d, [&, btn, wordId, word, correctMeaning, opts]() {
            bool correct = btn->text().endsWith(correctMeaning);
            for (auto b : opts) b->setEnabled(false);

            if (correct) {
                btn->setStyleSheet("background-color:#4CAF50; color:white; padding:14px; font-size:16px");
            } else {
                btn->setStyleSheet("background-color:#F44336; color:white;");
                for (auto b : opts) {
                    if (b->text().endsWith(correctMeaning)) {
                        b->setStyleSheet("background-color:#4CAF50; color:white; padding:14px; font-size:16px");
                    }
                }
            }
            // ✅ 保存到【复习记录表】（和快速测试一样）
            saveReviewRecord(wordId, word, correct);
        });
    }

    d.exec();
}

// ✅ 保存复习记录（关键：存入 review_history 表）
void ReviewSituationWidget::saveReviewRecord(int wordId, const QString &word, bool correct)
{
    query->prepare(R"(
        INSERT INTO review_history
        (word, is_correct, review_time, mode)
        VALUES (?, ?, datetime('now','localtime'), 'situation')
    )");
    query->addBindValue(word);
    query->addBindValue(correct ? 1 : 0);
    query->exec();
}

// 返回
void ReviewSituationWidget::onBack()
{
    emit backToReview();
    close();
}