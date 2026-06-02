#include "wordrelaterwidget.h"
#include "relatepage.h"
#include <QFont>
#include <QVBoxLayout>

WordRelaterWidget::WordRelaterWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("单词关联器");
    setFixedSize(500, 700);
    setStyleSheet("background-color: #000000; color: white;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(25);
    layout->setContentsMargins(50, 50, 50, 50);

    QFont font;
    font.setPointSize(14);
    font.setBold(true);

    // 统一只有一个：单词关联组
    btnRelate = new QPushButton("单词关联组");
    btnBack   = new QPushButton("返回主界面");

    btnRelate->setFixedHeight(70);
    btnBack->setFixedHeight(70);

    btnRelate->setFont(font);
    btnBack->setFont(font);

    btnRelate->setStyleSheet("color: white;");
    btnBack->setStyleSheet("color: white;");

    layout->addWidget(btnRelate);
    layout->addWidget(btnBack);

    connect(btnBack, &QPushButton::clicked, this, [=]() {
        emit back();
    });

    connect(btnRelate, &QPushButton::clicked, this, &WordRelaterWidget::openRelatePage);
}

void WordRelaterWidget::openRelatePage()
{
    this->hide();
    RelatePage *w = new RelatePage();
    w->show();

    connect(w, &RelatePage::back, this, [=]() {
        this->show();
        w->deleteLater();
    });
}