#include "mainwindow.h"
#include "learnnewwidget.h"
#include "wordlistwidget.h"
#include "wordrelaterwidget.h"
#include "wordbookmanager.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QSettings> // 必须加

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("单词背诵软件");
    setFixedSize(500, 750);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(18);
    mainLayout->setContentsMargins(50, 30, 50, 30);

    QFont btnFont;
    btnFont.setPointSize(14);
    btnFont.setBold(true);

    // ========== 词书信息栏 ==========
    lblWordbook = new QLabel();
    lblWordbook->setAlignment(Qt::AlignCenter);
    lblWordbook->setStyleSheet("color:#4fc3f7; font-size:13px; padding:4px;");
    lblWordbook->setText(QString::fromUtf8("📖 当前词书：")
                         + WordbookManager::displayName(WordbookManager::currentWordbook()));
    mainLayout->addWidget(lblWordbook);

    // 更换词书按钮
    btnChangeBook = new QPushButton("🔄 选择 / 更换词书");
    btnChangeBook->setFixedHeight(40);
    btnChangeBook->setStyleSheet(R"(
        QPushButton {
            background-color: #1a6dd4;
            color: white;
            font-weight: bold;
            border: none;
            border-radius: 6px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #1e88e5;
        }
    )");
    mainLayout->addWidget(btnChangeBook);

    // ========== 功能按钮 ==========
    btnLearnNew = new QPushButton("学习新词");
    btnReview = new QPushButton("复习旧词");
    btnWordList = new QPushButton("已学单词表");
    btnLinker = new QPushButton("单词关联器");
    btnBook = new QPushButton("单词本");
    btnData = new QPushButton("学习数据");

    // ✅ 新增：设置 API Key 按钮
    btnSetApi = new QPushButton("🔑 设置 AI API Key");
    btnSetApi->setFixedHeight(50);
    btnSetApi->setStyleSheet(R"(
        QPushButton {
            background-color: #2196F3;
            color: white;
            font-weight: bold;
            border:none;
            border-radius:6px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
    )");

    // 重置学习记录
    btnReset = new QPushButton("重置学习记录");
    btnReset->setStyleSheet(R"(
        QPushButton {
            background-color: #ff4444;
            color: white;
            font-weight: bold;
            border: none;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #cc0000;
        }
    )");

    // 统一高度
    btnLearnNew->setFixedHeight(60);
    btnReview->setFixedHeight(60);
    btnWordList->setFixedHeight(60);
    btnLinker->setFixedHeight(60);
    btnBook->setFixedHeight(60);
    btnData->setFixedHeight(60);
    btnReset->setFixedHeight(50);

    // 设置字体
    btnLearnNew->setFont(btnFont);
    btnReview->setFont(btnFont);
    btnWordList->setFont(btnFont);
    btnLinker->setFont(btnFont);
    btnBook->setFont(btnFont);
    btnData->setFont(btnFont);
    btnSetApi->setFont(btnFont);

    // 添加到布局
    mainLayout->addWidget(btnLearnNew);
    mainLayout->addWidget(btnReview);
    mainLayout->addWidget(btnWordList);
    mainLayout->addWidget(btnLinker);
    mainLayout->addWidget(btnBook);
    mainLayout->addWidget(btnData);
    mainLayout->addWidget(btnSetApi); // ✅ 加在这里
    mainLayout->addStretch();
    mainLayout->addWidget(btnReset);

    // 信号绑定
    connect(btnChangeBook, &QPushButton::clicked, this, &MainWindow::onChangeWordbook);
    connect(btnLearnNew, &QPushButton::clicked, this, &MainWindow::onLearnNewClicked);
    connect(btnWordList, &QPushButton::clicked, this, &MainWindow::onWordListClicked);
    connect(btnLinker,  &QPushButton::clicked, this, &MainWindow::onWordRelaterClicked);
    connect(btnReset,   &QPushButton::clicked, this, &MainWindow::onResetRecord);
    connect(btnReview, &QPushButton::clicked, this, &MainWindow::onReviewButtonClicked);
    connect(btnBook, &QPushButton::clicked, this, &MainWindow::onWordBookClicked);
    connect(btnSetApi, &QPushButton::clicked, this, &MainWindow::onSetApiKeyClicked); // ✅
}

MainWindow::~MainWindow()
{
}

// ==============================
// ✅ 设置 API Key（保存到文件）
// ==============================
void MainWindow::onSetApiKeyClicked()
{
    bool ok;
    QString oldKey = QSettings("app_config.ini", QSettings::IniFormat).value("API_KEY").toString();

    QString key = QInputDialog::getText(this,
                                        "设置 AI API Key",
                                        "请输入你的 API Key：",
                                        QLineEdit::Password,
                                        oldKey,
                                        &ok);

    if (!ok || key.trimmed().isEmpty()) return;

    QSettings set("app_config.ini", QSettings::IniFormat);
    set.setValue("API_KEY", key.trimmed());

    QMessageBox::information(this, "✅ 成功", "API Key 已保存！\n永久有效，无需重复输入！");
}

// ========== 更换词书 ==========
void MainWindow::onChangeWordbook()
{
    QStringList items;
    QStringList tags = WordbookManager::availableWordbooks();
    for (const QString &tag : tags) {
        items << WordbookManager::displayName(tag);
    }

    int currentIdx = tags.indexOf(WordbookManager::currentWordbook());
    if (currentIdx < 0) currentIdx = 0;

    bool ok = false;
    QString choice = QInputDialog::getItem(
        this,
        "选择 / 更换词书",
        "请选择要使用的词书：\n（学习记录与词书绑定，切换后需重新学习）",
        items,
        currentIdx,
        false,
        &ok
        );

    if (!ok || choice.isEmpty()) return;

    int idx = items.indexOf(choice);
    if (idx < 0 || idx >= tags.size()) return;

    QString newBook = tags[idx];
    if (newBook == WordbookManager::currentWordbook()) {
        QMessageBox::information(this, "提示", "已经是当前词书，无需更换。");
        return;
    }

    int confirm = QMessageBox::question(this,
                                        "确认更换词书",
                                        QString("确定要更换为 %1 吗？\n\n⚠ 学习记录与词书绑定，切换后使用新词书记录。")
                                            .arg(WordbookManager::displayName(newBook)),
                                        QMessageBox::Yes | QMessageBox::No);

    if (confirm != QMessageBox::Yes) return;

    if (WordbookManager::switchWordbook(newBook)) {
        lblWordbook->setText("📖 当前词书：" + WordbookManager::displayName(WordbookManager::currentWordbook()));
        QMessageBox::information(this, "✅ 成功", "已切换到：" + WordbookManager::displayName(newBook));
    } else {
        QMessageBox::critical(this, "❌ 失败", "词书切换失败");
    }
}

void MainWindow::onLearnNewClicked()
{
    this->hide();
    LearnNewWidget *w = new LearnNewWidget();
    w->show();
    connect(w, &LearnNewWidget::backToMain, this, [=]() {
        this->show();
        w->deleteLater();
    });
}

void MainWindow::onWordListClicked()
{
    this->hide();
    WordListWidget *w = new WordListWidget();
    connect(w, &WordListWidget::returnToMainWindow, this, [=]() {
        this->show();
        w->deleteLater();
    });
    w->show();
}

void MainWindow::onWordRelaterClicked()
{
    this->hide();
    WordRelaterWidget *w = new WordRelaterWidget();
    w->show();
    connect(w, &WordRelaterWidget::back, this, [=]() {
        this->show();
        w->deleteLater();
    });
}

void MainWindow::onResetRecord()
{
    int choice1 = QMessageBox::warning(this,
                                       "⚠️  危险操作：重置学习记录",
                                       "本操作会清空【所有学习记录】\n\n• 单词库不会被删除\n• 清空：学习/复习/分组记录\n• 不可撤销！\n\n确定继续？",
                                       QMessageBox::Yes | QMessageBox::No);

    if (choice1 != QMessageBox::Yes) return;

    int choice2 = QMessageBox::critical(this,
                                        "❌ 最终确认",
                                        "确定清空所有学习记录？\n无法恢复！！",
                                        QMessageBox::Yes | QMessageBox::No);

    if (choice2 != QMessageBox::Yes) return;

    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery query(db);
    bool ok1 = query.exec("DELETE FROM learn_record");
    bool ok2 = query.exec("DELETE FROM review_history");
    bool ok3 = query.exec("DELETE FROM relate_word");

    if (ok1 && ok2 && ok3)
        QMessageBox::information(this, "✅ 成功", "已清空所有学习/复习/分组关联记录！");
    else
        QMessageBox::critical(this, "❌ 失败", "清空失败！");
}

void MainWindow::onReviewButtonClicked()
{
    this->hide();
    ReviewWidget *rw = new ReviewWidget;
    rw->show();
    connect(rw, &ReviewWidget::backToMain, this, [=](){
        rw->close();
        this->show();
    });
    connect(rw, &ReviewWidget::openFastReview, this, [=](){
        rw->hide();
        FastReviewWidget *fw = new FastReviewWidget;
        fw->startReview();
        fw->show();
        connect(fw, &FastReviewWidget::backToReview, this, [=](){
            fw->close();
            rw->show();
        });
    });
    connect(rw, &ReviewWidget::openScenarioReview, this, [=](){
        rw->hide();
        rw->show();
    });
}

void MainWindow::onWordBookClicked()
{
    this->hide();
    WordBookWidget *w = new WordBookWidget;
    w->show();
    connect(w, &WordBookWidget::returnToMainWindow, this, [=](){
        this->show();
        w->deleteLater();
    });
}
