#include "fastmodewidget.h"
#include <QSqlDatabase>
#include <QRandomGenerator>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <algorithm>

QString FastModeWidget::cleanMultiLineText(const QString &raw)
{
    QString text = raw.trimmed();
    // 1. 统一 Windows / Linux 换行符
    text.replace("\r\n", "\n");
    // 2. 处理存成字符串 "\\n" 的情况（数据库直接存文字\n）
    text.replace("\\n", "\n");
    // 3. 合并连续多个空行，避免大量空白
    while (text.contains("\n\n\n"))
        text.replace("\n\n\n", "\n\n");

    // QLabel 支持原生 \n 自动换行；QPushButton 需要 HTML <br> 才能换行
    return text;
}

FastModeWidget::FastModeWidget(QWidget *parent)
    : QWidget(parent), currentWordId(-1)
{
    setWindowTitle("快速模式");
    setFixedSize(580, 780);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setSpacing(22);
    mainLay->setContentsMargins(40, 40, 40, 40);

    QHBoxLayout *top = new QHBoxLayout();
    QPushButton *btnBack = new QPushButton("← 返回");
    top->addWidget(btnBack);
    top->addStretch();

    lblWord = new QLabel();
    lblWord->setAlignment(Qt::AlignCenter);
    QFont f;
    f.setPointSize(34);
    f.setBold(true);
    lblWord->setFont(f);

    lblPhonetic = new QLabel();
    lblPhonetic->setAlignment(Qt::AlignCenter);
    lblPart = new QLabel();
    lblPart->setAlignment(Qt::AlignCenter);
    lblExample = new QLabel();
    lblExample->setWordWrap(true);
    lblExample->setAlignment(Qt::AlignCenter);

    QVBoxLayout *optLay = new QVBoxLayout();
    optLay->setSpacing(14);
    btnA = new QPushButton();
    btnB = new QPushButton();
    btnC = new QPushButton();
    btnD = new QPushButton();

    for (auto *btn : {btnA, btnB, btnC, btnD}) {
        btn->setMinimumHeight(70);
        btn->setStyleSheet("text-align:left;padding:12px;font-size:15px");
        optLay->addWidget(btn);
    }

    QPushButton *btnNext = new QPushButton("下一题");
    btnNext->setFixedHeight(65);

    mainLay->addLayout(top);
    mainLay->addWidget(lblWord);
    mainLay->addWidget(lblPhonetic);
    mainLay->addWidget(lblPart);
    mainLay->addWidget(lblExample);
    mainLay->addLayout(optLay);
    mainLay->addWidget(btnNext);

    connect(btnBack, &QPushButton::clicked, this, [this]() { emit backToLearnNew(); close(); });
    connect(btnA, &QPushButton::clicked, this, [this]() { checkAnswer(btnA); });
    connect(btnB, &QPushButton::clicked, this, [this]() { checkAnswer(btnB); });
    connect(btnC, &QPushButton::clicked, this, [this]() { checkAnswer(btnC); });
    connect(btnD, &QPushButton::clicked, this, [this]() { checkAnswer(btnD); });
    connect(btnNext, &QPushButton::clicked, this, &FastModeWidget::loadNextWord);

    QSqlDatabase db = QSqlDatabase::database("wordconn");
    query = new QSqlQuery(db);

    loadNextWord();
}

void FastModeWidget::loadNextWord()
{
    for (auto *btn : {btnA, btnB, btnC, btnD}) {
        btn->setEnabled(true);
        btn->setStyleSheet(R"(
    QPushButton {
        text-align:left;
        padding:12px;
        font-size:15px;
        white-space: pre-wrap; /* 关键：识别 \n 自动换行 */
    }
)");
    }

    QString sql = R"(
        SELECT v.id, v.word, v.phonetic, v.part, v.meaning, v.example
        FROM vocabulary v
        LEFT JOIN learn_record r ON v.id = r.word_id
        WHERE r.is_learned = 0 OR r.is_learned IS NULL
        ORDER BY RANDOM() LIMIT 1
    )";

    if (!query->exec(sql) || !query->next()) {
        QMessageBox::information(this, "完成", "🎉 所有单词已学习完毕！");
        emit backToLearnNew();
        close();
        return;
    }

    currentWordId = query->value("id").toInt();
    currentWord = query->value("word").toString();
    // 清洗所有带换行的字段
    currentPhonetic = cleanMultiLineText(query->value("phonetic").toString());
    currentPart = cleanMultiLineText(query->value("part").toString());
    currentMeaning = cleanMultiLineText(query->value("meaning").toString());
    currentExample = cleanMultiLineText(query->value("example").toString());

    lblWord->setText(currentWord);
    lblPhonetic->setText(currentPhonetic);
    lblPart->setText(currentPart);
    // 例句支持换行
    if (!currentExample.isEmpty())
        lblExample->setText("💡 " + currentExample);
    else
        lblExample->clear();


    QStringList options;
    query->exec("SELECT meaning FROM vocabulary WHERE word != '" + currentWord + "' ORDER BY RANDOM() LIMIT 3");
    while (query->next())
    {
        QString rawOpt = query->value(0).toString();
        QString cleanOpt = cleanMultiLineText(rawOpt);
        // 按钮用HTML换行
        options << cleanOpt;
    }
    // 正确释义也要转<br>
    QString dispCorrectMean = currentMeaning;
    options << dispCorrectMean;

    std::shuffle(options.begin(), options.end(), *QRandomGenerator::global());

    btnA->setText("1. " + options[0]);
    btnB->setText("2. " + options[1]);
    btnC->setText("3. " + options[2]);
    btnD->setText("4. " + options[3]);

    correctBtn = (options[0] == dispCorrectMean) ? btnA :
                     (options[1] == dispCorrectMean) ? btnB :
                     (options[2] == dispCorrectMean) ? btnC : btnD;
}

void FastModeWidget::checkAnswer(QPushButton *btn)
{
    for (auto *b : {btnA, btnB, btnC, btnD}) b->setEnabled(false);
    bool correct = (btn == correctBtn);

    query->prepare("INSERT OR REPLACE INTO learn_record (word_id, is_learned, study_date, is_correct) VALUES (?,1,?,?)");
    query->addBindValue(currentWordId);
    query->addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
    query->addBindValue(correct ? 1 : 0);
    query->exec();

    qDebug() << "=====================================";
    qDebug() << "✅ 已保存学习记录：";
    qDebug() << "单词ID：" << currentWordId << " | 时间：" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm") << " | 答对：" << (correct ? "是" : "否");
    qDebug() << "=====================================\n";

    if (correct) {
        btn->setStyleSheet(R"(
        QPushButton {
            background:#4CAF50;
            color:white;
            text-align:left;
            padding:12px;
            font-size:15px;
            white-space: pre-wrap;
        }
    )");
    } else {
        btn->setStyleSheet(R"(
        QPushButton {
            background:red;
            color:white;
            text-align:left;
            padding:12px;
            font-size:15px;
            white-space: pre-wrap;
        }
    )");
        correctBtn->setStyleSheet(R"(
        QPushButton {
            background:#4CAF50;
            color:white;
            text-align:left;
            padding:12px;
            font-size:15px;
            white-space: pre-wrap;
        }
    )");
    }
}