#ifndef REVIEWWIDGET_H
#define REVIEWWIDGET_H

#include <QWidget>
#include <QPushButton>

class ReviewWidget : public QWidget
{
    Q_OBJECT
public:
    // 补上构造函数声明，必须和cpp参数匹配
    explicit ReviewWidget(QWidget *parent = nullptr);

signals:
    void backToMain();
    void openFastReview();
    void openScenarioReview();

private slots:
    void onButtonClicked();

private:
    QPushButton *btnFast;
    QPushButton *btnScenario;
    QPushButton *btnBack;
    void initUI();
};

#endif // REVIEWWIDGET_H