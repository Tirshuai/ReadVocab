#ifndef FASTMODEWIDGET_H
#define FASTMODEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSqlQuery>

class FastModeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FastModeWidget(QWidget *parent = nullptr);

signals:
    void backToLearnNew();

private slots:
    void loadNextWord();
    void checkAnswer(QPushButton *btn);

private:
    QLabel *lblWord;
    QLabel *lblPhonetic;
    QLabel *lblPart;
    QLabel *lblExample;
    QPushButton *btnA, *btnB, *btnC, *btnD;
    QSqlQuery *query;

    int currentWordId;
    QString currentWord;
    QString currentPhonetic;
    QString currentPart;
    QString currentMeaning;
    QString currentExample;
    QPushButton *correctBtn;
};

#endif