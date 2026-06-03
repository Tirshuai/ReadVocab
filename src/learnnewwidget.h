#ifndef LEARNNEWWIDGET_H
#define LEARNNEWWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

class LearnNewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LearnNewWidget(QWidget *parent = nullptr);

signals:
    void backToMain();

private slots:
    void onFastMode();
    void onScenarioMode();
    void onBack();
};

#endif