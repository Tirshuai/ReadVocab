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
#include <QStringList>

// 新增文本清洗函数，类域正确
QString ScenarioModeWidget::cleanMultiLineText(const QString &raw)
{
    QString text = raw.trimmed();
    text.replace("\r\n", "\n");
    text.replace("\\n", "\n");
    text.replace("\t", " ");
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

    textBrowser->setText("AI正在生成情景短文，请稍候...\n\n本次至少学习以下单词：\n" + currentWords.join(", "));
    aiGenerator->generateStoryArticle(currentWords);
}

void ScenarioModeWidget::onArticleGenerated(const QString &article)
{
    QString html = article;

    // ====== 第一层：AI 用 **...** 标记的单词 → 转为可点击链接 ======
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
        // 倒序替换，避免位置偏移
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

    // ====== 第二层：兜底 —— 大小写不敏感匹配 AI 遗漏的单词 ======
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

// 【修复1】类域改为 ScenarioModeWidget::，不再是 ReviewSituationWidget::
void ScenarioModeWidget::showQuizDialog(int wordId, const QString &word, const QString &correctMeaning)
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

    // 【修复2】lambda 捕获添加 this，才能调用成员 saveRecord
    for (auto btn : opts) {
        connect(btn, &QPushButton::clicked, &d, [this, &d, btn, wordId, cleanCorrect, opts]() {
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
            // 调用本类保存记录函数
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