#include "fastreviewwidget.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <algorithm>
#include <cmath>

QString FastReviewWidget::cleanMultiLineText(const QString &raw)
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

FastReviewWidget::FastReviewWidget(QWidget *parent)
    : QWidget(parent), m_memoryHelper(new MemoryHelper(this)), m_currentIndex(0), correctCount(0)
{
    setWindowTitle("快速复习");
    setFixedSize(580, 780);
    initUI();
}

void FastReviewWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(22);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    QHBoxLayout *topLayout = new QHBoxLayout;
    QPushButton *btnBack = new QPushButton("← 返回");
    topLayout->addWidget(btnBack);
    topLayout->addStretch();

    labelProgress = new QLabel("进度：0/0");
    labelProgress->setAlignment(Qt::AlignCenter);

    labelWord = new QLabel("WORD");
    labelWord->setAlignment(Qt::AlignCenter);
    QFont wordFont;
    wordFont.setPointSize(34);
    wordFont.setBold(true);
    labelWord->setFont(wordFont);

    QVBoxLayout *optionLayout = new QVBoxLayout;
    optionLayout->setSpacing(14);
    for (int i = 0; i < 4; i++) {
        options[i] = new QPushButton();
        options[i]->setMinimumHeight(85);
        optionLayout->addWidget(options[i]);
        connect(options[i], &QPushButton::clicked, this, &FastReviewWidget::onOptionClicked);
    }

    btnNext = new QPushButton("下一题");
    btnNext->setFixedHeight(65);
    btnNext->setVisible(false);
    connect(btnNext, &QPushButton::clicked, this, &FastReviewWidget::nextQuestion);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(labelProgress);
    mainLayout->addWidget(labelWord);
    mainLayout->addLayout(optionLayout);
    mainLayout->addWidget(btnNext);

    setStyleSheet(R"(
        QWidget {
            background-color: #121212;
            color: #ffffff;
        }
        QLabel {
            color: #ffffff;
        }
        QPushButton {
            background-color: #1E1E1E;
            color: white;
            border: none;
            border-radius: 6px;
            text-align: left;
            padding: 12px;
            font-size: 15px;
            white-space: pre-wrap;
        }
        QPushButton:hover {
            background-color: #333333;
        }
    )");

    connect(btnBack, &QPushButton::clicked, this, [this]() {
        emit backToReview();
        close();
    });
}

// 启动复习：艾宾浩斯算法抽取单词列表
void FastReviewWidget::startReview()
{
    correctCount = 0;
    m_currentIndex = 0;
    wordList.clear();

    // 调用遗忘曲线算法获取待复习单词ID，最小间隔1天仅作为分层条件，不再过滤删除
    QList<int> reviewWordIds = m_memoryHelper->getReviewWordList(20, 1);
    // 修改判断：无任何已学单词才提示，有单词直接开始复习
    if (reviewWordIds.isEmpty())
    {
        QMessageBox::information(this, "提示", "你还没有学习任何单词，无法复习！");
        return;
    }

    // 根据ID批量查询单词文本、释义
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery q(db);
    QString idStr;
    for (int wid : reviewWordIds)
        idStr += QString::number(wid) + ",";
    idStr.chop(1);

    q.exec(QString("SELECT id, word, meaning FROM vocabulary WHERE id IN (%1)").arg(idStr));
    while (q.next()) {
        QVariantMap m;
        m["id"] = q.value("id").toInt();
        m["word"] = q.value("word").toString();
        m["meaning"] = cleanMultiLineText(q.value("meaning").toString());
        wordList.append(m);
    }

    m_reviewWordQueue = reviewWordIds;
    loadCurrentWord();
}

// 加载当前序号单词
void FastReviewWidget::loadCurrentWord()
{
    if (m_currentIndex >= wordList.size()) {
        showFinishDialog();
        return;
    }
    showCurrentQuestion();
}

void FastReviewWidget::showCurrentQuestion()
{
    btnNext->setVisible(false);
    auto map = wordList[m_currentIndex];
    QString word = map["word"].toString();
    correctMeaning = map["meaning"].toString();

    labelWord->setText(word);
    labelProgress->setText(QString("进度：%1/%2").arg(m_currentIndex + 1).arg(wordList.size()));

    for (int i = 0; i < 4; i++) {
        options[i]->setEnabled(true);
        options[i]->setStyleSheet("");
    }

    QStringList wrongOptions = getRandomWrongMeanings(correctMeaning, 3);
    QStringList allOptions = wrongOptions << correctMeaning;
    std::shuffle(allOptions.begin(), allOptions.end(), *QRandomGenerator::global());

    options[0]->setText("1. " + allOptions[0]);
    options[1]->setText("2. " + allOptions[1]);
    options[2]->setText("3. " + allOptions[2]);
    options[3]->setText("4. " + allOptions[3]);

    correctBtn = nullptr;
    for (auto *btn : options) {
        if (btn->text().endsWith(correctMeaning)) {
            correctBtn = btn;
            break;
        }
    }
}

void FastReviewWidget::onOptionClicked()
{
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
    bool isCorrect = (clickedBtn == correctBtn);
    auto currentWordMap = wordList[m_currentIndex];
    QString wordText = currentWordMap["word"].toString();

    for (auto *btn : options) btn->setEnabled(false);

    if (isCorrect) {
        clickedBtn->setStyleSheet(R"(
    QPushButton {
        background-color:#4CAF50;
        color:white;
        text-align:left;
        padding:12px;
        font-size:15px;
        white-space: pre-wrap;
    }
)");
        correctCount++;
    } else {
        clickedBtn->setStyleSheet(R"(
    QPushButton {
        background-color:#F44336;
        color:white;
        text-align:left;
        padding:12px;
        font-size:15px;
        white-space: pre-wrap;
    }
)");
        correctBtn->setStyleSheet(R"(
    QPushButton {
        background-color:#4CAF50;
        color:white;
        text-align:left;
        padding:12px;
        font-size:15px;
        white-space: pre-wrap;
    }
)");
    }

    // 调用工具类写入复习记录（替换原生SQL插入，统一复用算法模块）
    m_memoryHelper->saveReviewRecord(wordText, isCorrect, "fast");
    btnNext->setVisible(true);
}

void FastReviewWidget::nextQuestion()
{
    m_currentIndex++;
    loadCurrentWord();
}

void FastReviewWidget::showFinishDialog()
{
    int rate = wordList.isEmpty() ? 0 : (correctCount * 100 / wordList.size());
    QMessageBox::information(this, "复习完成",
                             QString("✅ 复习结束！\n正确：%1\n总数：%2\n正确率：%3%")
                                 .arg(correctCount)
                                 .arg(wordList.size())
                                 .arg(rate));

    emit backToReview();
    close();
}

QStringList FastReviewWidget::getRandomWrongMeanings(const QString &correct, int count)
{
    QStringList list;
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery q(db);
    QString temp = correct;
    temp.replace("'", "''");

    q.exec(QString("SELECT meaning FROM vocabulary WHERE meaning != '%1' ORDER BY random() LIMIT %2")
               .arg(temp).arg(count));

    while (q.next())
    {
        QString raw = q.value(0).toString();
        list << cleanMultiLineText(raw);
    }
    while (list.size() < count) list << "错误释义";
    return list;
}