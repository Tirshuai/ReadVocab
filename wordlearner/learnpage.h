#ifndef LEARNPAGE_H
#define LEARNPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QRandomGenerator>

struct Word {
    int id;
    QString word;
    QString phonetic;
    QString part;
    QString meaning;
    QString example;
};

class LearnPage : public QWidget
{
    Q_OBJECT
public:
    explicit LearnPage(QWidget *parent = nullptr);

signals:
    void backToMain(); // 发给主窗口：我要回去

private:
    bool initDB();
    Word getRandomWord();
    QStringList getRandomMeanings(const QString& exceptWord);

    QLabel *labelWord;
    QLabel *labelPhonetic;
    QLabel *labelPart;
    QLabel *labelExample;
    QPushButton *optA, *optB, *optC, *optD;
    QPushButton *btnNext;
    QPushButton *btnBack; // 返回主页面按钮

    Word currentWord;
    QString correctMeaning;
    QPushButton *correctOpt;

    void loadNewCard();
    void checkAnswer(QPushButton *userBtn);
};

#endif // LEARNPAGE_H