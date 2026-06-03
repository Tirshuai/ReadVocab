#include "learnnewwidget.h"
#include "fastmodewidget.h"
#include "scenariomodewidget.h"
#include <QFont>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

LearnNewWidget::LearnNewWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("学习新词");
    setFixedSize(580, 780);  // 统一大窗口

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setSpacing(30);
    lay->setContentsMargins(50, 80, 50, 80);
    lay->setAlignment(Qt::AlignCenter);

    // 标题
    QLabel *title = new QLabel("学习新词");
    QFont titleFont;
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);

    // 按钮样式（和复习页完全统一）
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

    QPushButton *btnFast = new QPushButton("快速模式");
    QPushButton *btnScenario = new QPushButton("情境模式");
    QPushButton *btnBack = new QPushButton("返回主页面");

    btnFast->setStyleSheet(btnStyle);
    btnScenario->setStyleSheet(btnStyle);
    btnBack->setStyleSheet(btnStyle);

    lay->addWidget(title);
    lay->addWidget(btnFast);
    lay->addWidget(btnScenario);
    lay->addWidget(btnBack);

    connect(btnFast, &QPushButton::clicked, this, &LearnNewWidget::onFastMode);
    connect(btnScenario, &QPushButton::clicked, this, &LearnNewWidget::onScenarioMode);
    connect(btnBack, &QPushButton::clicked, this, &LearnNewWidget::onBack);

    // 暗黑全局样式（统一整个软件）
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

void LearnNewWidget::onFastMode()
{
    this->hide();
    FastModeWidget *w = new FastModeWidget();
    w->show();

    connect(w, &FastModeWidget::backToLearnNew, this, [=]() {
        this->show();
        w->deleteLater();
    });
}

void LearnNewWidget::onScenarioMode()
{
    this->hide();
    ScenarioModeWidget *w = new ScenarioModeWidget();
    w->show();

    connect(w, &ScenarioModeWidget::backToLearnNew, this, [=]() {
        this->show();
        w->deleteLater();
    });
}

void LearnNewWidget::onBack()
{
    emit backToMain();
}