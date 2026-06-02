#include "wordlistwidget.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <algorithm>
#include <QMap>

WordListWidget::WordListWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("已学单词表");
    setFixedSize(700, 800);
    setStyleSheet("background-color: #000; color: white;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // 返回按钮
    QPushButton *btnBack = new QPushButton("← 返回主界面");
    btnBack->setFixedHeight(48);
    btnBack->setStyleSheet("background:#333; color:white; font-weight:bold;");
    mainLayout->addWidget(btnBack);
    connect(btnBack, &QPushButton::clicked, this, &WordListWidget::backToMain);

    // 搜索栏
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("🔍 搜索单词...");
    searchEdit->setFixedHeight(44);
    searchEdit->setStyleSheet("background:#222; color:white; padding:8px;");

    QPushButton *btnSearch = new QPushButton("搜索");
    btnSearch->setFixedHeight(44);
    btnSearch->setStyleSheet("background:#007acc; color:white; font-weight:bold;");
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(btnSearch);
    mainLayout->addLayout(searchLayout);

    connect(btnSearch, &QPushButton::clicked, this, [=]() {
        loadLearnedWords(searchEdit->text().trimmed());
    });
    connect(searchEdit, &QLineEdit::returnPressed, this, [=]() {
        loadLearnedWords(searchEdit->text().trimmed());
    });

    // 滚动区域
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("border:none;");
    mainLayout->addWidget(scroll);

    contentWidget = new QWidget;
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    scroll->setWidget(contentWidget);

    // 数据库
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    query = new QSqlQuery(db);
    query->exec("ALTER TABLE learn_record ADD COLUMN starred INTEGER DEFAULT 0");

    loadLearnedWords("");
}

void WordListWidget::clearLayout(QLayout *layout) {
    QLayoutItem *item;
    while ((item = layout->takeAt(0))) {
        if (item->widget()) item->widget()->deleteLater();
        if (item->layout()) clearLayout(item->layout());
        delete item;
    }
}

void WordListWidget::loadLearnedWords(const QString &filter)
{
    clearLayout(contentLayout);
    groupedWords.clear();

    QString sql = R"(
        SELECT v.id, v.word, v.part, v.meaning, v.example,
               r.study_date, r.starred
        FROM vocabulary v
        JOIN learn_record r ON v.id = r.word_id
        ORDER BY r.study_date ASC
    )";

    if (!query->exec(sql)) {
        QLabel *err = new QLabel("查询失败：" + query->lastError().text());
        err->setStyleSheet("color:red;");
        contentLayout->addWidget(err);
        return;
    }

    while (query->next()) {
        QMap<QString, QString> w;
        w["id"] = query->value("id").toString();
        w["word"] = query->value("word").toString();
        w["part"] = query->value("part").toString();
        w["meaning"] = query->value("meaning").toString();
        w["example"] = query->value("example").toString();
        w["study_date"] = query->value("study_date").toString().split(" ").first();
        w["starred"] = query->value("starred").toString();

        if (!filter.isEmpty() && !w["word"].contains(filter, Qt::CaseInsensitive))
            continue;

        groupedWords[w["study_date"]].append(w);
    }

    if (groupedWords.isEmpty()) {
        QLabel *empty = new QLabel("📭 目前无已学习单词");
        empty->setStyleSheet("color:#ccc; font-size:16px;");
        contentLayout->addWidget(empty);
        return;
    }

    QList<QString> dates = groupedWords.keys();
    std::sort(dates.begin(), dates.end());

    for (const QString &date : dates) {
        QLabel *dateLab = new QLabel("🗓 " + date);
        dateLab->setStyleSheet("color:#4fc3f7; font-size:18px; font-weight:bold;");
        contentLayout->addWidget(dateLab);

        for (const auto &w : groupedWords[date]) {
            int wordId = w["id"].toInt();
            bool starred = w["starred"] == "1";

            QWidget *card = new QWidget;
            card->setStyleSheet("background:#111; border-radius:8px; padding:14px;");
            QHBoxLayout *cardLayout = new QHBoxLayout(card);
            cardLayout->setSpacing(12);

            // 左侧文字区域
            QVBoxLayout *leftLayout = new QVBoxLayout;
            leftLayout->setSpacing(6);
            QLabel *wordLab = new QLabel(w["word"]);
            wordLab->setStyleSheet("font-size:16px; font-weight:bold; color:white;");
            leftLayout->addWidget(wordLab);
            leftLayout->addWidget(new QLabel("词性：" + w["part"]));
            leftLayout->addWidget(new QLabel("释义：" + w["meaning"]));
            if (!w["example"].isEmpty()) {
                leftLayout->addWidget(new QLabel("例句：" + w["example"]));
            }
            cardLayout->addLayout(leftLayout);
            cardLayout->addStretch();

            // 右侧蓝色按钮（现在不会被遮挡）
            QPushButton *starBtn = new QPushButton(starred ? "取消\n着重" : "添加\n着重");
            starBtn->setFixedWidth(72);
            starBtn->setStyleSheet(R"(
                QPushButton{
                    background:#3671e9;
                    color:white;
                    font-size:13px;
                    border-radius:6px;
                    padding:6px 2px;
                }
            )");
            cardLayout->addWidget(starBtn);

            connect(starBtn, &QPushButton::clicked, this, [=](){
                bool newState = !starred;
                toggleStar(wordId, newState);
                qDebug() << "[着重操作] 单词:" << w["word"]
                         << "原状态:" << starred << "→新状态:" << newState;
            });

            // 只给左侧文字区加点击事件，点左侧才弹详情
            wordLab->setCursor(Qt::PointingHandCursor);
            connect(wordLab, &QLabel::linkActivated, this, [=](){
                showWordDetail(wordId);
            });
            // 让整个左侧区域可点击
            QPushButton *leftClick = new QPushButton;
            leftClick->setStyleSheet("background:transparent; border:none;");
            leftClick->setCursor(Qt::PointingHandCursor);
            connect(leftClick, &QPushButton::clicked, this, [=](){
                showWordDetail(wordId);
            });
            leftLayout->addWidget(leftClick);

            contentLayout->addWidget(card);
        }
    }
}

void WordListWidget::toggleStar(int wordId, bool star)
{
    query->prepare("UPDATE learn_record SET starred = ? WHERE word_id = ?");
    query->addBindValue(star ? 1 : 0);
    query->addBindValue(wordId);
    query->exec();
    loadLearnedWords(searchEdit->text().trimmed());
}

void WordListWidget::showWordDetail(int wordId)
{
    query->prepare(R"(
        SELECT v.word, r.study_date, r.starred
        FROM vocabulary v
        JOIN learn_record r ON v.id = r.word_id
        WHERE v.id = ?
    )");
    query->addBindValue(wordId);
    if (!query->exec() || !query->next()) return;

    QString word = query->value("word").toString();
    QString date = query->value("study_date").toString();
    bool star = query->value("starred").toBool();

    QMessageBox::information(this, "📊 详情",
                             "单词：" + word + "\n"
                                               "首次学习：" + date + "\n"
                                          "着重标记：" + (star ? "是" : "否"));
}