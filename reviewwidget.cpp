#include "reviewwidget.h"
#include "reviewsituationwidget.h"  // 1. 加头文件
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QWidget>

ReviewWidget::ReviewWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("复习旧词");
    setFixedSize(580, 780);
    initUI();
}

void ReviewWidget::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(30);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(50, 50, 50, 50);

    QLabel *title = new QLabel("复习旧词");
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QString btnStyle = R"(
        QPushButton{
            font-size:16px;
            padding:18px;
            border-radius:8px;
            background-color:#1E1E1E;
            color:white;
            min-width:220px;
        }
        QPushButton:hover{
            background-color:#333333;
        }
    )";

    btnFast = new QPushButton("快速测试");
    btnScenario = new QPushButton("情境模式");
    btnBack = new QPushButton("返回主页面");

    btnFast->setStyleSheet(btnStyle);
    btnScenario->setStyleSheet(btnStyle);
    btnBack->setStyleSheet(btnStyle);

    layout->addWidget(title);
    layout->addWidget(btnFast);
    layout->addWidget(btnScenario);
    layout->addWidget(btnBack);

    connect(btnFast, &QPushButton::clicked, this, &ReviewWidget::onButtonClicked);
    connect(btnScenario, &QPushButton::clicked, this, &ReviewWidget::onButtonClicked);
    connect(btnBack, &QPushButton::clicked, this, &ReviewWidget::onButtonClicked);

    setStyleSheet(R"(
        QWidget {
            background-color: #121212;
            color: #ffffff;
        }
        QLabel {
            color: #ffffff;
        }
    )");
}

void ReviewWidget::onButtonClicked()
{
    QObject *s = sender();
    if (s == btnFast) {
        emit openFastReview();
    }
    else if (s == btnScenario) {
        // 2. 在这里直接打开情境复习窗口
        this->hide();
        ReviewSituationWidget *w = new ReviewSituationWidget;
        w->show();

        // 3. 返回时显示当前页面
        connect(w, &ReviewSituationWidget::backToReview, this, [=](){
            w->close();
            this->show();
        });
    }
    else if (s == btnBack) {
        emit backToMain();
    }
}