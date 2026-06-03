#include "scenariomodewidget.h"
#include "aitextgenerator.h"
#include <QSqlDatabase>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QVBoxLayout>
#include <algorithm>
#include <QDialog>
#include <QTextBrowser>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QPushButton>
#include <QLabel>
#include <QUrl>

ScenarioModeWidget::ScenarioModeWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("情境模式");
    setFixedSize(680, 700);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setSpacing(20);
    lay->setContentsMargins(30, 30, 30, 30);

    QLabel *title = new QLabel("📖 AI 情景短文学习");
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(22);
    titleFont.setBold(true);
    title->setFont(titleFont);
    lay->addWidget(title);

    btnGenerate = new QPushButton("随机抽取生词 + 生成AI短文");
    btnGenerate->setFixedHeight(50);
    lay->addWidget(btnGenerate);

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
    lay->addWidget(textBrowser);

    btnBack = new QPushButton("← 返回学习新词");
    btnBack->setFixedHeight(50);
    lay->addWidget(btnBack);

    connect(textBrowser, &QTextBrowser::anchorClicked, this, &ScenarioModeWidget::onWordClicked);
    connect(btnBack, &QPushButton::clicked, this, &ScenarioModeWidget::onBack);
    connect(btnGenerate, &QPushButton::clicked, this, &ScenarioModeWidget::startGenerateScenario);

    QSqlDatabase db = QSqlDatabase::database("wordconn");
    query = new QSqlQuery(db);

    aiGenerator = new AiTextGenerator(this);
    connect(aiGenerator, &AiTextGenerator::articleGenerated, this, &ScenarioModeWidget::onArticleGenerated);
    connect(aiGenerator, &AiTextGenerator::requestError, this, [this](QString err){
        textBrowser->setText("❌ 生成失败：" + err);
    });

    textBrowser->setText("点击上方按钮开始学习 → 每次自动学习若干全新生词\n\n由AI为你生成专属情景短文～");
}

void ScenarioModeWidget::startGenerateScenario()
{
    QStringList unlearned;

    query->exec(R"(
        SELECT v.word
        FROM vocabulary v
        LEFT JOIN learn_record r ON v.id = r.word_id
        WHERE r.word_id IS NULL
    )");

    while (query->next()) {
        unlearned << query->value(0).toString();
    }

    if (unlearned.size() < 5) {
        textBrowser->setText("✅ 恭喜！所有单词都已学习完毕！");
        return;
    }

    currentWords.clear();
    for (int i = 0; i < 5; ++i) {
        int idx = QRandomGenerator::global()->bounded(unlearned.size());
        currentWords << unlearned.takeAt(idx);
    }

    textBrowser->setText("AI正在生成情景短文，请稍候...\n\n本次学习单词：\n" + currentWords.join(", "));
    aiGenerator->generateStoryArticle(currentWords);
}

void ScenarioModeWidget::onArticleGenerated(const QString &article)
{
    QString html = article;

    for (const QString &word : currentWords) {
        QRegularExpression re("\\b" + QRegularExpression::escape(word) + "\\b");
        html.replace(re, QString("<a href='word://%1'>%1</a>").arg(word));
    }

    textBrowser->setHtml("<h3>情景短文</h3>" + html);
}

void ScenarioModeWidget::onWordClicked(const QUrl &url)
{
    QString word = url.host();
    if (url.scheme() != "word" || word.isEmpty()) return;

    query->prepare("SELECT id, meaning FROM vocabulary WHERE word = ? LIMIT 1");
    query->addBindValue(word);

    if (!query->exec() || !query->next()) {
        QMessageBox::warning(this, "提示", "数据库中无该单词！");
        return;
    }

    int wordId = query->value(0).toInt();
    QString correctMeaning = query->value(1).toString();
    showQuizDialog(wordId, word, correctMeaning);
}

void ScenarioModeWidget::showQuizDialog(int wordId, const QString &word, const QString &correctMeaning)
{
    QDialog d(this);
    d.setWindowTitle("情境单词练习");
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

    for (auto btn : opts) {
        connect(btn, &QPushButton::clicked, &d, [&, btn, wordId, correctMeaning, opts]() {
            bool correct = btn->text().endsWith(correctMeaning);
            for (auto b : opts) b->setEnabled(false);

            if (correct) {
                btn->setStyleSheet("background-color:#4CAF50; color:white; text-align:left; padding:14px; font-size:16px");
            } else {
                btn->setStyleSheet("background-color:#F44336; color:white;");
                for (auto b : opts) {
                    if (b->text().endsWith(correctMeaning)) {
                        b->setStyleSheet("background-color:#4CAF50; color:white; text-align:left; padding:14px; font-size:16px");
                    }
                }
            }
            saveRecord(wordId, correct);
        });
    }

    d.exec();
}

void ScenarioModeWidget::saveRecord(int wordId, bool correct)
{
    query->prepare(R"(
        INSERT OR REPLACE INTO learn_record
        (word_id, is_learned, study_date, is_correct)
        VALUES (?, 1, ?, ?)
    )");
    query->addBindValue(wordId);
    query->addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
    query->addBindValue(correct ? 1 : 0);
    query->exec();
}

void ScenarioModeWidget::onBack()
{
    emit backToLearnNew();
    close();
}