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
#include <QUrl>

// 文本清洗工具函数
QString ReviewSituationWidget::cleanMultiLineText(const QString &raw)
{
    QString text = raw.trimmed();
    // 统一各类换行格式
    text.replace("\r\n", "\n");
    text.replace("\\n", "\n");
    text.replace("\t", " ");
    // 压缩连续空行
    while (text.contains("\n\n\n"))
        text.replace("\n\n\n", "\n\n");

    QStringList lines = text.split("\n");
    QStringList newLines;
    for (auto line : lines)
    {
        QString l = line.trimmed();
        if (l.isEmpty()) continue;

        bool startWithNum = false;
        if (!l.isEmpty() && l.at(0).isDigit())
        {
            if (l.size() >= 2)
            {
                QChar second = l.at(1);
                if (second == '.' || second == ')')
                    startWithNum = true;
                else if (second == QString("、").at(0))
                    startWithNum = true;
            }
        }
        if (startWithNum)
            l = "    " + l;
        newLines << l;
    }
    return newLines.join("\n");
}

// 构造函数
ReviewSituationWidget::ReviewSituationWidget(QWidget *parent)
    : QWidget(parent), m_memoryHelper(new MemoryHelper(this))
{
    setWindowTitle("情境复习模式");
    setFixedSize(680, 780);

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

    // 文本展示框
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

    // 布局组装
    lay->addWidget(title);
    lay->addWidget(btnGenerate);
    lay->addWidget(textBrowser);
    lay->addWidget(btnBack);

    // 信号槽绑定
    connect(textBrowser, &QTextBrowser::anchorClicked, this, &ReviewSituationWidget::onWordClicked);
    connect(btnBack, &QPushButton::clicked, this, &ReviewSituationWidget::onBack);
    connect(btnGenerate, &QPushButton::clicked, this, &ReviewSituationWidget::startGenerateScenario);

    // 数据库查询对象
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    query = new QSqlQuery(db);

    // AI文本生成器
    aiGenerator = new AiTextGenerator(this);
    connect(aiGenerator, &AiTextGenerator::articleGenerated, this, &ReviewSituationWidget::onArticleGenerated);
    connect(aiGenerator, &AiTextGenerator::requestError, this, [this](QString err){
        textBrowser->setText("❌ 生成失败：" + err);
    });

    textBrowser->setText("点击上方按钮开始复习 → 基于遗忘曲线抽取单词\n\nAI将生成短文帮助你巩固记忆～");
}

// ========= 替换原有函数：艾宾浩斯分层抽取单词（核心修改）=========
QStringList ReviewSituationWidget::getScenarioReviewWords(int takeCount)
{
    QStringList wordTextList;
    // 调用分层抽取逻辑：优先遗忘单词，不足则兜底所有已学词，永远有数据
    QList<int> wordIdList = m_memoryHelper->getReviewWordList(takeCount, 1);
    if(wordIdList.isEmpty())
        return {};

    // 根据ID批量查询单词原文
    QString idStr;
    for(int wid : wordIdList)
        idStr += QString::number(wid) + ",";
    idStr.chop(1);

    query->exec(QString("SELECT word FROM vocabulary WHERE id IN (%1)").arg(idStr));
    while(query->next())
    {
        wordTextList << query->value("word").toString();
    }
    return wordTextList;
}

// 开始生成AI短文
void ReviewSituationWidget::startGenerateScenario()
{
    // 调用艾宾浩斯算法抽取5个单词
    currentWords = getScenarioReviewWords(5);

    if (currentWords.isEmpty()) {
        textBrowser->setText("❌ 你还未学习任何单词，无法开展情境复习！");
        return;
    }

    textBrowser->setText("AI 正在生成复习短文，请稍候...\n\n本次复习单词至少包括：\n" + currentWords.join(", "));
    aiGenerator->generateStoryArticle(currentWords);
}

// AI短文生成完成回调
void ReviewSituationWidget::onArticleGenerated(const QString &article)
{
    QString html = article;

    // 第一层：替换**单词**为可点击链接
    {
        QRegularExpression boldRe("\\*\\*(.+?)\\*\\*");
        QRegularExpressionMatchIterator bit = boldRe.globalMatch(html);
        QList<QPair<int, int>> boldMatches;
        QStringList boldTexts;
        while (bit.hasNext()) {
            auto m = bit.next();
            boldMatches.append({m.capturedStart(), m.capturedLength()});
            boldTexts.append(m.captured(1));
        }
        // 倒序替换防止偏移
        for (int i = boldMatches.size() - 1; i >= 0; --i) {
            const auto &bm = boldMatches[i];
            QString markedText = boldTexts[i];
            QString baseWord;
            bool matched = false;
            for (const QString &cw : currentWords) {
                if (markedText.compare(cw, Qt::CaseInsensitive) == 0) {
                    baseWord = cw;
                    matched = true;
                    break;
                }
                if (markedText.length() >= cw.length() &&
                    markedText.left(cw.length()).compare(cw, Qt::CaseInsensitive) == 0) {
                    baseWord = cw;
                    matched = true;
                    break;
                }
            }
            if (matched) {
                QString link = QString("<a href='word://%1'>%2</a>").arg(baseWord, markedText);
                html.replace(bm.first, bm.second, link);
            } else {
                html.replace(bm.first, bm.second, markedText);
            }
        }
    }

    // 第二层：兜底匹配未标记的单词原形
    for (const QString &word : currentWords) {
        QRegularExpression re("\\b" + QRegularExpression::escape(word) + "\\b",
                              QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator it = re.globalMatch(html);
        QList<QRegularExpressionMatch> matches;
        while (it.hasNext()) matches.append(it.next());

        for (int i = matches.size() - 1; i >= 0; --i) {
            const auto &m = matches[i];
            int pos = m.capturedStart();
            int lastOpen = html.lastIndexOf("<a ", pos);
            int lastClose = html.lastIndexOf("</a>", pos);
            if (lastOpen != -1 && lastOpen > lastClose)
                continue;

            QString matchedText = m.captured();
            html.replace(pos, matchedText.length(),
                         QString("<a href='word://%1'>%2</a>").arg(word, matchedText));
        }
    }

    textBrowser->setHtml("<h3>复习短文</h3>" + html);
}

// 点击短文内单词弹窗测验
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

// 根据单词名查询id和释义
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

// 弹出多行释义选择题弹窗（换行方案完整实现）
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

    QStringList options;
    query->prepare("SELECT meaning FROM vocabulary WHERE word != ? ORDER BY RANDOM() LIMIT 3");
    query->addBindValue(word);
    query->exec();
    while (query->next())
    {
        QString rawMean = query->value(0).toString();
        QString cleanOpt = cleanMultiLineText(rawMean);
        options << cleanOpt;
    }
    QString cleanCorrect = cleanMultiLineText(correctMeaning);
    options << cleanCorrect;
    std::shuffle(options.begin(), options.end(), *QRandomGenerator::global());

    QPushButton *optA = new QPushButton("1. " + options[0]);
    QPushButton *optB = new QPushButton("2. " + options[1]);
    QPushButton *optC = new QPushButton("3. " + options[2]);
    QPushButton *optD = new QPushButton("4. " + options[3]);

    QList<QPushButton*> opts = {optA, optB, optC, optD};
    for (auto btn : opts) {
        btn->setMinimumHeight(85);
        btn->setStyleSheet(R"(
            QPushButton {
                text-align: left;
                padding:14px;
                font-size:16px;
                white-space: pre-wrap;
            }
        )");
        layout->addWidget(btn);
    }

    for (auto btn : opts) {
        connect(btn, &QPushButton::clicked, &d, [this, &d, btn, wordId, word, cleanCorrect, opts]() {
            bool correct = btn->text().endsWith(cleanCorrect);
            for (auto b : opts) b->setEnabled(false);

            if (correct) {
                btn->setStyleSheet(R"(
                    QPushButton {
                        background-color:#4CAF50;
                        color:white;
                        text-align: left;
                        padding:14px;
                        font-size:16px;
                        white-space: pre-wrap;
                    }
                )");
            } else {
                btn->setStyleSheet(R"(
                    QPushButton {
                        background-color:#F44336;
                        color:white;
                        text-align: left;
                        padding:14px;
                        font-size:16px;
                        white-space: pre-wrap;
                    }
                )");
                for (auto b : opts) {
                    if (b->text().endsWith(cleanCorrect)) {
                        b->setStyleSheet(R"(
                            QPushButton {
                                background-color:#4CAF50;
                                color:white;
                                text-align: left;
                                padding:14px;
                                font-size:16px;
                                white-space: pre-wrap;
                            }
                        )");
                    }
                }
            }
            // 调用统一工具类保存复习记录，mode固定为情境模式
            m_memoryHelper->saveReviewRecord(word, correct, "scenario");
        });
    }

    d.exec();
}

// 废弃原生SQL插入，统一使用MemoryHelper，此函数仅做兼容占位（可删除）
void ReviewSituationWidget::saveReviewRecord(int wordId, const QString &word, bool correct)
{
    m_memoryHelper->saveReviewRecord(word, correct, "scenario");
}

// 返回复习主页
void ReviewSituationWidget::onBack()
{
    emit backToReview();
    close();
}