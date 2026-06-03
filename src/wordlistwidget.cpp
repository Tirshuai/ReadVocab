#include "wordlistwidget.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <algorithm>
#include <QDate>
#include <QScrollBar>
#include <QDialog>
#include <QTextBrowser>

WordListWidget::WordListWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("已学单词表");
    setFixedSize(900, 800);
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

    // 全部 / 星标
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnAll = new QPushButton("📚 全部单词");
    btnStarred = new QPushButton("⭐ 仅着重单词");

    btnAll->setFixedHeight(40);
    btnStarred->setFixedHeight(40);
    btnAll->setStyleSheet("background:#3671e9; color:white; font-weight:bold;");
    btnStarred->setStyleSheet("background:#222; color:white; font-weight:bold;");

    btnLayout->addWidget(btnAll);
    btnLayout->addWidget(btnStarred);
    mainLayout->addLayout(btnLayout);

    connect(btnAll, &QPushButton::clicked, this, &WordListWidget::showAllWords);
    connect(btnStarred, &QPushButton::clicked, this, &WordListWidget::showOnlyStarred);

    // 布局：日历 + 单词
    QHBoxLayout *centerLayout = new QHBoxLayout;
    calendar = new QCalendarWidget;
    calendar->setFixedWidth(260);
    calendar->setStyleSheet(R"(
        QCalendarWidget { background:#111; color:white; }
        QCalendarWidget QTableView { color:white; }
    )");
    centerLayout->addWidget(calendar);

    QScrollArea *scroll = new QScrollArea;
    scroll->setObjectName("scrollArea");
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("border:none;");
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    centerLayout->addWidget(scroll);
    mainLayout->addLayout(centerLayout);

    contentWidget = new QWidget;
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(8,8,8,8);
    scroll->setWidget(contentWidget);

    // 懒加载初始化
    m_pageSize = 20;
    m_curLoadedCnt = 0;
    m_isLoading = false;

    // 滚动监听
    connect(scroll->verticalScrollBar(), &QScrollBar::valueChanged, this, &WordListWidget::onScrollChanged);
    connect(calendar, &QCalendarWidget::clicked, this, &WordListWidget::onCalendarClick);

    // 数据库
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    query = new QSqlQuery(db);

    loadLearnedWords("");
}

// ===================== 懒加载 =====================
void WordListWidget::onScrollChanged(int val)
{
    QScrollArea* scroll = findChild<QScrollArea*>("scrollArea");
    if (!scroll || m_isLoading) return;
    if (m_curLoadedCnt >= m_allWordCache.size()) return;

    QScrollBar* bar = scroll->verticalScrollBar();
    int remain = bar->maximum() - val;
    if (remain < 150) loadNextPage();
}

void WordListWidget::loadNextPage()
{
    m_isLoading = true;
    int total = m_allWordCache.size();
    int end = qMin(m_curLoadedCnt + m_pageSize, total);

    for (int i = m_curLoadedCnt; i < end; ++i) {
        auto w = m_allWordCache[i];
        int wordId = w["id"].toInt();
        bool starred = w["starred"] == "1";

        QWidget *card = new QWidget;
        card->setStyleSheet("background:#111; border-radius:8px; padding:14px;");
        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        QVBoxLayout *leftLayout = new QVBoxLayout;

        leftLayout->addWidget(new QLabel("<b>" + w["word"] + "</b>"));
        leftLayout->addWidget(new QLabel("词性：" + w["part"]));
        leftLayout->addWidget(new QLabel("释义：" + w["meaning"]));
        if (!w["example"].isEmpty())
            leftLayout->addWidget(new QLabel("例句：" + w["example"]));

        cardLayout->addLayout(leftLayout);
        cardLayout->addStretch();

        QVBoxLayout *btnRight = new QVBoxLayout;
        QPushButton *starBtn = new QPushButton(starred ? "取消\n着重" : "添加\n着重");
        starBtn->setFixedSize(70,50);
        starBtn->setStyleSheet("background:#3671e9; color:white;");
        btnRight->addWidget(starBtn);

        QPushButton *detailBtn = new QPushButton("查看\n详情");
        detailBtn->setFixedSize(70,50);
        detailBtn->setStyleSheet("background:#28a745; color:white;");
        btnRight->addWidget(detailBtn);
        cardLayout->addLayout(btnRight);

        connect(starBtn, &QPushButton::clicked, this, [=](){ toggleStar(wordId, !starred); });
        connect(detailBtn, &QPushButton::clicked, this, [=](){ showWordFullDetail(wordId); });

        contentLayout->addWidget(card);
    }

    m_curLoadedCnt = end;
    m_isLoading = false;
    contentWidget->updateGeometry();
}

// ===================== 加载数据（缓存版） =====================
void WordListWidget::loadLearnedWords(const QString &filter)
{
    clearLayout(contentLayout);
    m_allWordCache.clear();
    m_curLoadedCnt = 0;

    QString sql;
    if (filter == "starredOnly") {
        sql = R"(
            SELECT v.id, v.word, v.part, v.meaning, v.example, r.starred
            FROM vocabulary v JOIN learn_record r ON v.id = r.word_id
            WHERE r.starred = 1
        )";
    } else {
        sql = R"(
            SELECT v.id, v.word, v.part, v.meaning, v.example, r.starred
            FROM vocabulary v JOIN learn_record r ON v.id = r.word_id
        )";
    }

    if (!query->exec(sql)) return;

    while (query->next()) {
        QMap<QString, QString> w;
        w["id"] = query->value("id").toString();
        w["word"] = query->value("word").toString();
        w["part"] = query->value("part").toString();
        w["meaning"] = query->value("meaning").toString();
        w["example"] = query->value("example").toString();
        w["starred"] = query->value("starred").toString();

        if (!filter.isEmpty() && filter != "starredOnly" && !w["word"].contains(filter, Qt::CaseInsensitive))
            continue;

        m_allWordCache.append(w);
    }

    if (m_allWordCache.isEmpty()) {
        QLabel *empty = new QLabel(filter == "starredOnly" ? "⭐ 暂无着重单词" : "📭 无已学单词");
        contentLayout->addWidget(empty);
        return;
    }

    loadNextPage();
}

// ===================== 点击日历 → 弹窗显示当天单词 =====================
void WordListWidget::onCalendarClick(const QDate &dt)
{
    QString date = dt.toString("yyyy-MM-dd");

    QDialog d(this);
    d.setWindowTitle("🗓 " + date + " 学习单词");
    d.setFixedSize(700, 600);
    d.setStyleSheet("background:#111; color:white;");

    QVBoxLayout *lay = new QVBoxLayout(&d);

    QLabel *title = new QLabel("📅 " + date + " 学习的单词");
    title->setStyleSheet("font-size:18px; font-weight:bold; color:#4fc3f7;");
    lay->addWidget(title);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    lay->addWidget(scroll);

    QWidget *w = new QWidget;
    QVBoxLayout *vl = new QVBoxLayout(w);
    scroll->setWidget(w);

    QString sql = R"(
        SELECT v.id, v.word, v.part, v.meaning, v.example, r.starred
        FROM vocabulary v
        JOIN learn_record r ON v.id = r.word_id
        WHERE DATE(r.study_date) = ?
    )";

    query->prepare(sql);
    query->addBindValue(date);
    if (!query->exec()) return;

    bool has = false;
    while (query->next()) {
        has = true;
        int id = query->value("id").toInt();
        bool starred = query->value("starred").toBool();

        QWidget *card = new QWidget;
        card->setStyleSheet("background:#222; padding:10px; border-radius:6px;");
        QHBoxLayout *cl = new QHBoxLayout(card);

        QVBoxLayout *left = new QVBoxLayout;
        left->addWidget(new QLabel("<b>" + query->value("word").toString() + "</b>"));
        left->addWidget(new QLabel("词性：" + query->value("part").toString()));
        left->addWidget(new QLabel("释义：" + query->value("meaning").toString()));
        cl->addLayout(left);
        cl->addStretch();

        QPushButton *starBtn = new QPushButton(starred ? "⭐" : "☆");
        starBtn->setStyleSheet("color:yellow;");
        cl->addWidget(starBtn);
        connect(starBtn, &QPushButton::clicked, this, [=](){
            toggleStar(id, !starred);
        });

        QPushButton *detailBtn = new QPushButton("详情");
        cl->addWidget(detailBtn);
        connect(detailBtn, &QPushButton::clicked, this, [=](){
            showWordFullDetail(id);
        });

        vl->addWidget(card);
    }

    if (!has) {
        vl->addWidget(new QLabel("📭 当天无学习记录"));
    }

    d.exec();
    loadLearnedWords(searchEdit->text().trimmed());
}

// ===================== 工具函数 =====================
void WordListWidget::showAllWords() {
    btnAll->setStyleSheet("background:#3671e9;");
    btnStarred->setStyleSheet("background:#222;");
    loadLearnedWords("");
}

void WordListWidget::showOnlyStarred() {
    btnStarred->setStyleSheet("background:#3671e9;");
    btnAll->setStyleSheet("background:#222;");
    loadLearnedWords("starredOnly");
}

void WordListWidget::clearLayout(QLayout *layout) {
    QLayoutItem *item;
    while ((item = layout->takeAt(0))) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void WordListWidget::backToMain() {
    emit returnToMainWindow();
    close();
}

void WordListWidget::toggleStar(int wordId, bool star) {
    query->prepare("UPDATE learn_record SET starred = ? WHERE word_id = ?");
    query->addBindValue(star ? 1 : 0);
    query->addBindValue(wordId);
    query->exec();
    loadLearnedWords(searchEdit->text().trimmed());
}

void WordListWidget::showWordDetail(int wordId) {
    showWordFullDetail(wordId);
}

void WordListWidget::showWordFullDetail(int wordId)
{
    // 1.查询单词基础信息：单词、音标、词性、释义、例句
    query->prepare(R"(SELECT word, phonetic, part, meaning, example FROM vocabulary WHERE id = ?)");
    query->addBindValue(wordId);
    if (!query->exec() || !query->next()) return;

    QString word = query->value("word").toString();
    QString phonetic = query->value("phonetic").toString();
    QString part = query->value("part").toString();
    QString meaning = query->value("meaning").toString();
    QString example = query->value("example").toString();

    // 2.查询首次学习日期
    QString firstStudyDate = "暂无学习记录";
    query->prepare(R"(SELECT study_date FROM learn_record WHERE word_id = ?)");
    query->addBindValue(wordId);
    if (query->exec() && query->next())
        firstStudyDate = query->value("study_date").toString();

    // 3.查询全部复习记录
    QStringList reviewList;
    query->prepare(R"(SELECT review_time,is_correct,mode FROM review_history WHERE word = ? ORDER BY review_time DESC)");
    query->addBindValue(word);
    if (query->exec())
    {
        while (query->next())
        {
            QString revTime = query->value("review_time").toString();
            bool correct = query->value("is_correct").toBool();
            QString mode = query->value("mode").toString();
            QString modeName = mode == "fast" ? "快速自测" : "情境复习";
            QString res = correct ? "✅ 回答正确" : "❌ 回答错误";
            reviewList << QString("%1  |  %2  |  %3").arg(revTime,modeName,res);
        }
    }

    // 弹窗构造
    QDialog d(this);
    d.setWindowTitle("单词详细信息");
    d.setFixedSize(680,720);
    d.setStyleSheet("background:#121212;color:#ffffff;");
    QVBoxLayout *mainLay = new QVBoxLayout(&d);
    mainLay->setSpacing(12);
    mainLay->setContentsMargins(28,28,28,28);

    auto addLab = [&](QString txt,int fs=14,bool bold=false){
        QLabel *lab = new QLabel(txt);
        lab->setWordWrap(true);
        lab->setStyleSheet(QString("font-size:%1px;%2").arg(fs).arg(bold?"font-weight:bold;":""));
        mainLay->addWidget(lab);
    };

    addLab(QString("单词：%1").arg(word),22,true);
    addLab(QString("音标：%1").arg(phonetic.isEmpty()?"无":phonetic),15);
    addLab(QString("词性：%1").arg(part));
    addLab(QString("释义：%1").arg(meaning));
    addLab(QString("例句：%1").arg(example.isEmpty()?"无":example));
    addLab(QString("首次学习时间：%1").arg(firstStudyDate));

    addLab("\n===== 历次复习记录 =====",16,true);
    QTextBrowser *br = new QTextBrowser;
    br->setStyleSheet("background:#1e1e1e;color:#fff;padding:6px;");
    br->setReadOnly(true);
    br->setText(reviewList.empty() ? "暂无任何复习记录" : reviewList.join("\n"));
    mainLay->addWidget(br);

    QPushButton *closeBtn = new QPushButton("关闭");
    closeBtn->setFixedHeight(46);
    closeBtn->setStyleSheet("background:#333;color:#fff;");
    connect(closeBtn,&QPushButton::clicked,&d,&QDialog::close);
    mainLay->addWidget(closeBtn);

    d.exec();
}