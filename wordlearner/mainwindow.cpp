#include "mainwindow.h"
#include "learnpage.h"
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle("单词背诵软件");
    this->setFixedSize(500, 700);

    centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(50, 50, 50, 50);

    QFont btnFont;
    btnFont.setPointSize(14);
    btnFont.setBold(true);

    btnLearnNew = new QPushButton("学习新词");
    btnReview = new QPushButton("复习旧词");
    btnWordList = new QPushButton("已学单词表查看");
    btnLinker = new QPushButton("单词关联器");
    btnBook = new QPushButton("查看词书");
    btnData = new QPushButton("记忆数据");

    btnLearnNew->setFont(btnFont);
    btnReview->setFont(btnFont);
    btnWordList->setFont(btnFont);
    btnLinker->setFont(btnFont);
    btnBook->setFont(btnFont);
    btnData->setFont(btnFont);

    btnLearnNew->setFixedHeight(70);
    btnReview->setFixedHeight(70);
    btnWordList->setFixedHeight(70);
    btnLinker->setFixedHeight(70);
    btnBook->setFixedHeight(70);
    btnData->setFixedHeight(70);

    mainLayout->addWidget(btnLearnNew);
    mainLayout->addWidget(btnReview);
    mainLayout->addWidget(btnWordList);
    mainLayout->addWidget(btnLinker);
    mainLayout->addWidget(btnBook);
    mainLayout->addWidget(btnData);

    connect(btnLearnNew, &QPushButton::clicked, this, &MainWindow::onLearnNewClicked);
}

MainWindow::~MainWindow() {}

void MainWindow::onLearnNewClicked()
{
    // 隐藏主窗口，不关闭！
    this->hide();

    LearnPage *page = new LearnPage();
    page->show();

    // 返回时重新显示主界面
    connect(page, &LearnPage::backToMain, this, [=]() {
        this->show();
        page->deleteLater();
    });
}