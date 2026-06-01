#include "learnpage.h"
#include <QCoreApplication>
#include <QDir>
#include <QSqlError>
#include <QMessageBox>
#include <algorithm>

LearnPage::LearnPage(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowTitle("快速模式");
    this->setFixedSize(580, 780);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(22);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    QHBoxLayout *topLayout = new QHBoxLayout();
    btnBack = new QPushButton("← 返回主页面");
    btnBack->setStyleSheet("padding:6px 12px; font-size:14px;");
    topLayout->addWidget(btnBack);
    topLayout->addStretch();

    labelWord = new QLabel();
    labelWord->setAlignment(Qt::AlignCenter);
    QFont f;
    f.setPointSize(34);
    f.setBold(true);
    labelWord->setFont(f);

    labelPhonetic = new QLabel();
    labelPhonetic->setAlignment(Qt::AlignCenter);
    labelPhonetic->setStyleSheet("color: #666; font-size:17px;");

    labelPart = new QLabel();
    labelPart->setAlignment(Qt::AlignCenter);
    labelPart->setStyleSheet("color: #0066cc; font-size:18px;");

    labelExample = new QLabel();
    labelExample->setAlignment(Qt::AlignCenter);
    labelExample->setWordWrap(true);
    labelExample->setStyleSheet("font-size:16px; color:#555; padding:8px;");

    QVBoxLayout *optLayout = new QVBoxLayout();
    optLayout->setSpacing(14);

    optA = new QPushButton();
    optB = new QPushButton();
    optC = new QPushButton();
    optD = new QPushButton();

    for (auto btn : {optA, optB, optC, optD}) {
        btn->setMinimumHeight(70);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setStyleSheet("QPushButton{text-align:left; padding:12px; font-size:15px;}");
    }

    optLayout->addWidget(optA);
    optLayout->addWidget(optB);
    optLayout->addWidget(optC);
    optLayout->addWidget(optD);

    btnNext = new QPushButton("下一题");
    btnNext->setFixedHeight(65);
    btnNext->setStyleSheet("background:#4CAF50; color:white; font-size:17px;");

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(labelWord);
    mainLayout->addWidget(labelPhonetic);
    mainLayout->addWidget(labelPart);
    mainLayout->addWidget(labelExample);
    mainLayout->addLayout(optLayout);
    mainLayout->addWidget(btnNext);

    if (!initDB()) {
        QMessageBox::critical(this, "错误", "无法打开 vocabulary.db");
        close();
        return;
    }

    loadNewCard();

    connect(optA, &QPushButton::clicked, this, [=]() { checkAnswer(optA); });
    connect(optB, &QPushButton::clicked, this, [=]() { checkAnswer(optB); });
    connect(optC, &QPushButton::clicked, this, [=]() { checkAnswer(optC); });
    connect(optD, &QPushButton::clicked, this, [=]() { checkAnswer(optD); });

    connect(btnNext, &QPushButton::clicked, this, &LearnPage::loadNewCard);
    connect(btnBack, &QPushButton::clicked, this, [=]() {
        emit backToMain();
        this->close();
    });
}

bool LearnPage::initDB()
{
    if (QSqlDatabase::contains("vocab_conn"))
        return QSqlDatabase::database("vocab_conn").isOpen();

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "vocab_conn");
    db.setDatabaseName(QCoreApplication::applicationDirPath() + "/vocabulary.db");
    return db.open();
}

Word LearnPage::getRandomWord()
{
    Word w{};
    QSqlQuery q(QSqlDatabase::database("vocab_conn"));
    q.exec("SELECT id,word,phonetic,part,meaning,example FROM vocabulary ORDER BY RANDOM() LIMIT 1");
    if (q.next()) {
        w.id = q.value("id").toInt();
        w.word = q.value("word").toString();
        w.phonetic = q.value("phonetic").toString();
        w.part = q.value("part").toString();
        w.meaning = q.value("meaning").toString();
        w.example = q.value("example").toString();
    }
    return w;
}

// ====================== 【已修复：只选带中文的释义】 ======================
QStringList LearnPage::getRandomMeanings(const QString &exceptWord)
{
    QStringList res;
    QSqlQuery q(QSqlDatabase::database("vocab_conn"));

    q.prepare(R"(
        SELECT meaning FROM vocabulary
        WHERE word != ?
          AND meaning REGEXP '[一-龯]'
        ORDER BY RANDOM()
        LIMIT 3
    )");

    q.addBindValue(exceptWord);
    q.exec();

    while (q.next()) {
        QString meaning = q.value(0).toString().trimmed();
        if (!meaning.isEmpty()) {
            res << meaning;
        }
    }

    if (res.size() < 3) {
        QSqlQuery q2(QSqlDatabase::database("vocab_conn"));
        q2.prepare("SELECT meaning FROM vocabulary WHERE word != ? AND meaning != '' ORDER BY RANDOM() LIMIT ?");
        q2.addBindValue(exceptWord);
        q2.addBindValue(3 - res.size());
        q2.exec();
        while (q2.next()) {
            QString m = q2.value(0).toString().trimmed();
            if (!m.isEmpty() && !res.contains(m)) {
                res << m;
            }
        }
    }

    return res;
}
// ========================================================================

void LearnPage::loadNewCard()
{
    for (auto btn : {optA, optB, optC, optD}) {
        btn->setEnabled(true);
        btn->setStyleSheet("QPushButton{text-align:left; padding:12px; font-size:15px;}");
    }

    currentWord = getRandomWord();
    if (currentWord.word.isEmpty()) return;

    correctMeaning = currentWord.meaning;

    labelWord->setText(currentWord.word);
    labelPhonetic->setText(currentWord.phonetic);
    labelPart->setText(currentWord.part);

    if (!currentWord.example.isEmpty())
        labelExample->setText("💡 " + currentWord.example);
    else
        labelExample->setText("");

    QStringList options = getRandomMeanings(currentWord.word);
    options << correctMeaning;
    std::shuffle(options.begin(), options.end(), *QRandomGenerator::global());

    optA->setText("1. " + options[0]);
    optB->setText("1. " + options[1]);
    optC->setText("1. " + options[2]);
    optD->setText("1. " + options[3]);

    correctOpt = nullptr;
    for (auto btn : {optA, optB, optC, optD}) {
        if (btn->text().endsWith(correctMeaning)) {
            correctOpt = btn;
            break;
        }
    }
}

void LearnPage::checkAnswer(QPushButton *userBtn)
{
    for (auto btn : {optA, optB, optC, optD}) btn->setEnabled(false);

    if (userBtn == correctOpt) {
        userBtn->setStyleSheet("background-color:#4CAF50; color:white; text-align:left; padding:12px; font-size:15px;");
    } else {
        userBtn->setStyleSheet("background-color:#F44336; color:white; text-align:left; padding:12px; font-size:15px;");
        correctOpt->setStyleSheet("background-color:#4CAF50; color:white; text-align:left; padding:12px; font-size:15px;");
    }
}