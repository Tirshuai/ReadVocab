#include "mainwindow.h"
#include "learnnewwidget.h"
#include "wordlistwidget.h"
#include "wordrelaterwidget.h"   // ✅ 新的关联器
#include <QPushButton>
#include <QVBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("单词背诵软件");
    setFixedSize(500, 700);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(50, 50, 50, 50);

    QFont btnFont;
    btnFont.setPointSize(14);
    btnFont.setBold(true);

    // 功能按钮
    btnLearnNew = new QPushButton("学习新词");
    btnReview = new QPushButton("复习旧词");
    btnWordList = new QPushButton("已学单词表");
    btnLinker = new QPushButton("单词关联器");  // ✅ 正确名称
    btnBook = new QPushButton("单词本");
    btnData = new QPushButton("学习数据");

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
    btnLearnNew->setFixedHeight(70);
    btnReview->setFixedHeight(70);
    btnWordList->setFixedHeight(70);
    btnLinker->setFixedHeight(70);
    btnBook->setFixedHeight(70);
    btnData->setFixedHeight(70);
    btnReset->setFixedHeight(60);

    // 设置字体
    btnLearnNew->setFont(btnFont);
    btnReview->setFont(btnFont);
    btnWordList->setFont(btnFont);
    btnLinker->setFont(btnFont);
    btnBook->setFont(btnFont);
    btnData->setFont(btnFont);

    // 添加到布局
    mainLayout->addWidget(btnLearnNew);
    mainLayout->addWidget(btnReview);
    mainLayout->addWidget(btnWordList);
    mainLayout->addWidget(btnLinker);   // ✅ 单词关联器
    mainLayout->addWidget(btnBook);
    mainLayout->addWidget(btnData);
    mainLayout->addStretch();
    mainLayout->addWidget(btnReset);

    // 信号绑定
    connect(btnLearnNew, &QPushButton::clicked, this, &MainWindow::onLearnNewClicked);
    connect(btnWordList, &QPushButton::clicked, this, &MainWindow::onWordListClicked);
    connect(btnLinker,  &QPushButton::clicked, this, &MainWindow::onWordRelaterClicked); // ✅ 绑定
    connect(btnReset,   &QPushButton::clicked, this, &MainWindow::onResetRecord);
    // mainwindow.cpp构造里必须加连接
    connect(btnReview, &QPushButton::clicked, this, &MainWindow::onReviewButtonClicked);
    connect(btnBook, &QPushButton::clicked, this, &MainWindow::onWordBookClicked);
}

MainWindow::~MainWindow()
{
}

// 学习新词
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

// 已学单词表
void MainWindow::onWordListClicked()
{
    this->hide();
    WordListWidget *w = new WordListWidget();

    // 关键：监听单词表的返回信号 → 显示主窗口
    connect(w, &WordListWidget::returnToMainWindow, this, [=]() {
        this->show();       // 重新显示主窗口
        w->deleteLater();   // 安全销毁单词表
    });

    w->show();
}

// ================================
// ✅ 单词关联器（你要的新功能）
// ================================
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

// 重置学习记录（清空三个表）
void MainWindow::onResetRecord()
{
    int choice1 = QMessageBox::warning(this,
                                       "⚠️  危险操作：重置学习记录",
                                       "本操作会清空【所有学习记录】\n\n"
                                       "• 单词库 vocabulary 不会被删除\n"
                                       "• 清空：学习记录、复习记录、关联分组单词\n"
                                       "• 不可撤销！\n\n确定继续？",
                                       QMessageBox::Yes | QMessageBox::No);

    if (choice1 != QMessageBox::Yes) return;

    int choice2 = QMessageBox::critical(this,
                                        "❌ 最终确认",
                                        "确定清空所有学习记录？\n无法恢复！！",
                                        QMessageBox::Yes | QMessageBox::No);

    if (choice2 != QMessageBox::Yes) return;

    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery query(db);

    // 👇 一次性清空三个表（真正的重置）
    bool ok1 = query.exec("DELETE FROM learn_record");
    bool ok2 = query.exec("DELETE FROM review_history");
    bool ok3 = query.exec("DELETE FROM relate_word");

    if (ok1 && ok2 && ok3)
        QMessageBox::information(this, "✅ 成功", "已清空所有学习/复习/分组关联记录！");
    else
        QMessageBox::critical(this, "❌ 失败", "清空失败！");
}

// 主窗口点击 “复习旧词” 按钮
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
        // 以后你做情境模式在这里打开
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