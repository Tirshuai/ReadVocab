#include "learnnewwidget.h"
#include "fastmodewidget.h"
#include "scenariomodewidget.h"
#include <QFont>

LearnNewWidget::LearnNewWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("学习新词");
    setFixedSize(500, 700);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setSpacing(40);
    lay->setContentsMargins(80, 100, 80, 100);

    QFont f;
    f.setPointSize(16);
    f.setBold(true);

    QPushButton *btnFast = new QPushButton("快速模式");
    QPushButton *btnScenario = new QPushButton("情境模式");
    QPushButton *btnBack = new QPushButton("返回主页面");

    for (auto *btn : {btnFast, btnScenario, btnBack})
    {
        btn->setFont(f);
        btn->setFixedHeight(80);
        lay->addWidget(btn);
    }

    connect(btnFast, &QPushButton::clicked, this, &LearnNewWidget::onFastMode);
    connect(btnScenario, &QPushButton::clicked, this, &LearnNewWidget::onScenarioMode);
    connect(btnBack, &QPushButton::clicked, this, &LearnNewWidget::onBack);
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