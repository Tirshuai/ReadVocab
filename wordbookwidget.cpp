#include "wordbookwidget.h"
#include <QMessageBox>
#include <QTextBrowser>
#include <QDialog>
#include <QScrollBar>
#include <QSqlError>
#include <algorithm>

WordBookWidget::WordBookWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("单词本");
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
    connect(btnBack, &QPushButton::clicked, this, &WordBookWidget::onBack);

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

    connect(btnSearch, &QPushButton::clicked, this, &WordBookWidget::onSearch);
    connect(searchEdit, &QLineEdit::returnPressed, this, &WordBookWidget::onSearch);

    // 切换按钮：全部 / 已学 / 未学
    QHBoxLayout *tabLayout = new QHBoxLayout;
    btnAll = new QPushButton("📚 全部单词");
    btnLearned = new QPushButton("✅ 已学单词");
    btnUnlearned = new QPushButton("📝 未学单词");

    btnAll->setFixedHeight(42);
    btnLearned->setFixedHeight(42);
    btnUnlearned->setFixedHeight(42);

    btnAll->setStyleSheet("background:#3671e9; color:white; font-weight:bold;");
    btnLearned->setStyleSheet("background:#222; color:white; font-weight:bold;");
    btnUnlearned->setStyleSheet("background:#222; color:white; font-weight:bold;");

    tabLayout->addWidget(btnAll);
    tabLayout->addWidget(btnLearned);
    tabLayout->addWidget(btnUnlearned);
    mainLayout->addLayout(tabLayout);

    connect(btnAll, &QPushButton::clicked, this, &WordBookWidget::onBtnAllClicked);
    connect(btnLearned, &QPushButton::clicked, this, &WordBookWidget::onBtnLearnedClicked);
    connect(btnUnlearned, &QPushButton::clicked, this, &WordBookWidget::onBtnUnlearnedClicked);

    // 滚动区域
    QScrollArea *scroll = new QScrollArea;
    scroll->setObjectName("scrollArea");
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("border:none;");
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainLayout->addWidget(scroll);

    contentWidget = new QWidget;
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(8,8,8,8);
    scroll->setWidget(contentWidget);

    // 懒加载：滚动监听
    connect(scroll->verticalScrollBar(), &QScrollBar::valueChanged, this, &WordBookWidget::onScrollChanged);

    // 数据库
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    query = new QSqlQuery(db);

    // 懒加载参数初始化
    m_pageSize = 20;
    m_curLoadedCnt = 0;
    m_isLoading = false;

    currentMode = "all";
    loadAllWords();
}

// ================== 分类切换 ==================
void WordBookWidget::onBtnAllClicked() {
    currentMode = "all";
    btnAll->setStyleSheet("background:#3671e9; color:white;");
    btnLearned->setStyleSheet("background:#222; color:white;");
    btnUnlearned->setStyleSheet("background:#222; color:white;");
    loadAllWords(searchEdit->text().trimmed());
}

void WordBookWidget::onBtnLearnedClicked() {
    currentMode = "learned";
    btnAll->setStyleSheet("background:#222; color:white;");
    btnLearned->setStyleSheet("background:#3671e9; color:white;");
    btnUnlearned->setStyleSheet("background:#222; color:white;");
    loadLearnedWords(searchEdit->text().trimmed());
}

void WordBookWidget::onBtnUnlearnedClicked() {
    currentMode = "unlearned";
    btnAll->setStyleSheet("background:#222; color:white;");
    btnLearned->setStyleSheet("background:#222; color:white;");
    btnUnlearned->setStyleSheet("background:#3671e9; color:white;");
    loadUnlearnedWords(searchEdit->text().trimmed());
}

void WordBookWidget::onSearch() {
    QString key = searchEdit->text().trimmed();
    if (currentMode == "all") loadAllWords(key);
    else if (currentMode == "learned") loadLearnedWords(key);
    else loadUnlearnedWords(key);
}

// ================== 滚动懒加载 ==================
void WordBookWidget::onScrollChanged(int val)
{
    QScrollArea* scroll = findChild<QScrollArea*>("scrollArea");
    if (!scroll) return;
    if (m_isLoading) return;
    if (m_curLoadedCnt >= m_allWordCache.size()) return;

    QScrollBar* bar = scroll->verticalScrollBar();
    int remain = bar->maximum() - val;

    if (remain < 150) {
        loadNextPage();
    }
}

void WordBookWidget::loadNextPage()
{
    m_isLoading = true;
    int total = m_allWordCache.size();
    int end = qMin(m_curLoadedCnt + m_pageSize, total);

    for (int i = m_curLoadedCnt; i < end; ++i)
    {
        auto w = m_allWordCache[i];
        int id = w["id"].toInt();

        QWidget *card = new QWidget;
        card->setStyleSheet("background:#111; border-radius:8px; padding:14px;");
        QHBoxLayout *cardLayout = new QHBoxLayout(card);

        QVBoxLayout *left = new QVBoxLayout;
        left->addWidget(new QLabel("<b>" + w["word"] + "</b>"));
        left->addWidget(new QLabel("词性：" + w["part"]));
        left->addWidget(new QLabel("释义：" + w["meaning"]));

        if (!w["example"].isEmpty())
            left->addWidget(new QLabel("例句：" + w["example"]));

        if (currentMode == "learned")
            left->addWidget(new QLabel("首次学习：" + w["study_date"]));

        cardLayout->addLayout(left);
        cardLayout->addStretch();

        QPushButton *detailBtn = new QPushButton("详情");
        detailBtn->setFixedSize(70, 50);
        detailBtn->setStyleSheet("background:#28a745; color:white;");
        cardLayout->addWidget(detailBtn);

        connect(detailBtn, &QPushButton::clicked, this, [=]() {
            showWordFullDetail(id);
        });

        contentLayout->addWidget(card);
    }

    m_curLoadedCnt = end;
    m_isLoading = false;
    contentWidget->updateGeometry();
}

// ================== 三种数据查询（全量查入缓存，分页渲染） ==================
void WordBookWidget::loadAllWords(const QString &searchFilter)
{
    clearLayout(contentLayout);
    m_allWordCache.clear();
    m_curLoadedCnt = 0;

    QString sql = "SELECT id, word, part, meaning, example FROM vocabulary";
    if (!searchFilter.isEmpty())
        sql += " WHERE word LIKE '%" + searchFilter + "%'";
    sql += " ORDER BY word ASC";

    if (!query->exec(sql)) {
        contentLayout->addWidget(new QLabel("查询失败：" + query->lastError().text()));
        return;
    }

    while (query->next()) {
        QMap<QString, QString> w;
        w["id"] = query->value("id").toString();
        w["word"] = query->value("word").toString();
        w["part"] = query->value("part").toString();
        w["meaning"] = query->value("meaning").toString();
        w["example"] = query->value("example").toString();
        w["study_date"] = "";
        m_allWordCache.append(w);
    }

    if (m_allWordCache.isEmpty()) {
        contentLayout->addWidget(new QLabel("暂无数据"));
        return;
    }
    loadNextPage();
}

void WordBookWidget::loadLearnedWords(const QString &searchFilter)
{
    clearLayout(contentLayout);
    m_allWordCache.clear();
    m_curLoadedCnt = 0;

    QString sql = R"(
        SELECT v.id, v.word, v.part, v.meaning, v.example, r.study_date
        FROM vocabulary v
        JOIN learn_record r ON v.id = r.word_id
        WHERE r.is_learned = 1
    )";

    if (!searchFilter.isEmpty())
        sql += " AND v.word LIKE '%" + searchFilter + "%'";
    sql += " ORDER BY v.word ASC";

    if (!query->exec(sql)) return;

    while (query->next()) {
        QMap<QString, QString> w;
        w["id"] = query->value("id").toString();
        w["word"] = query->value("word").toString();
        w["part"] = query->value("part").toString();
        w["meaning"] = query->value("meaning").toString();
        w["example"] = query->value("example").toString();
        w["study_date"] = query->value("study_date").toString();
        m_allWordCache.append(w);
    }

    if (m_allWordCache.isEmpty()) {
        contentLayout->addWidget(new QLabel("暂无已学单词"));
        return;
    }
    loadNextPage();
}

void WordBookWidget::loadUnlearnedWords(const QString &searchFilter)
{
    clearLayout(contentLayout);
    m_allWordCache.clear();
    m_curLoadedCnt = 0;

    QString sql = R"(
        SELECT v.id, v.word, v.part, v.meaning, v.example
        FROM vocabulary v
        LEFT JOIN learn_record r ON v.id = r.word_id
        WHERE r.word_id IS NULL OR r.is_learned = 0
    )";

    if (!searchFilter.isEmpty())
        sql += " AND v.word LIKE '%" + searchFilter + "%'";
    sql += " ORDER BY v.word ASC";

    if (!query->exec(sql)) return;

    while (query->next()) {
        QMap<QString, QString> w;
        w["id"] = query->value("id").toString();
        w["word"] = query->value("word").toString();
        w["part"] = query->value("part").toString();
        w["meaning"] = query->value("meaning").toString();
        w["example"] = query->value("example").toString();
        w["study_date"] = "";
        m_allWordCache.append(w);
    }

    if (m_allWordCache.isEmpty()) {
        contentLayout->addWidget(new QLabel("暂无未学单词"));
        return;
    }
    loadNextPage();
}

// ================== 详情弹窗 ==================
void WordBookWidget::showWordFullDetail(int wordId)
{
    query->prepare("SELECT word, part, meaning, example FROM vocabulary WHERE id = ?");
    query->addBindValue(wordId);
    if (!query->exec() || !query->next()) return;

    QString word = query->value(0).toString();
    QString part = query->value(1).toString();
    QString meaning = query->value(2).toString();
    QString example = query->value(3).toString();

    QString firstLearn = "未学习";
    query->prepare("SELECT study_date FROM learn_record WHERE word_id = ?");
    query->addBindValue(wordId);
    if (query->exec() && query->next())
        firstLearn = query->value(0).toString();

    QStringList reviews;
    query->prepare("SELECT review_time, is_correct, mode FROM review_history WHERE word = ? ORDER BY review_time DESC");
    query->addBindValue(word);
    if (query->exec()) {
        while (query->next()) {
            QString t = query->value(0).toString();
            bool ok = query->value(1).toBool();
            QString m = query->value(2).toString();
            reviews << t + " | " + (m == "fast" ? "快速" : "情境") + " | " + (ok ? "✅" : "❌");
        }
    }

    QDialog d(this);
    d.setFixedSize(650, 700);
    d.setStyleSheet("background:#121212; color:white;");
    QVBoxLayout *l = new QVBoxLayout(&d);

    auto add = [&](QString s, int size = 14, bool b = 0) {
        QLabel *lab = new QLabel(s);
        lab->setStyleSheet(QString("color:white;font-size:%1px;%2").arg(size).arg(b ? "font-weight:bold;" : ""));
        lab->setWordWrap(true);
        l->addWidget(lab);
    };

    add("单词：" + word, 22, true);
    add("词性：" + part);
    add("释义：" + meaning);
    add("例句：" + example);
    add("首次学习：" + firstLearn);
    add("\n复习记录：", 16, true);

    QTextBrowser *br = new QTextBrowser;
    br->setStyleSheet("background:#1e1e1e;color:white;");
    br->setText(reviews.isEmpty() ? "无" : reviews.join("\n"));
    l->addWidget(br);

    QPushButton *closeBtn = new QPushButton("关闭");
    closeBtn->setFixedHeight(48);
    closeBtn->setStyleSheet("background:#333;color:white;");
    l->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, &d, &QDialog::close);
    d.exec();
}

// ================== 工具函数 ==================
void WordBookWidget::clearLayout(QLayout *layout) {
    QLayoutItem *item;
    while ((item = layout->takeAt(0))) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void WordBookWidget::onBack() {
    emit returnToMainWindow();
    close();
}

void WordBookWidget::toggleStar(int wordId, bool star)
{
    query->prepare("UPDATE learn_record SET starred = ? WHERE word_id = ?");
    query->addBindValue(star ? 1 : 0);
    query->addBindValue(wordId);
    query->exec();

    if (currentMode == "all") loadAllWords(searchEdit->text().trimmed());
    else if (currentMode == "learned") loadLearnedWords(searchEdit->text().trimmed());
    else loadUnlearnedWords(searchEdit->text().trimmed());
}