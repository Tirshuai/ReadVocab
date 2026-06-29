#ifndef FASTREVIEWWIDGET_H
#define FASTREVIEWWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QVariantMap>
#include "memoryhelper.h"

class FastReviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FastReviewWidget(QWidget *parent = nullptr);
    void startReview();

signals:
    void backToReview();

private:
    void initUI();
    QString cleanMultiLineText(const QString &raw);
    void loadCurrentWord();
    void showCurrentQuestion();
    void nextQuestion();
    void showFinishDialog();
    QStringList getRandomWrongMeanings(const QString &correct, int count);

private slots:
    void onOptionClicked();

private:
    QLabel* labelProgress;
    QLabel* labelWord;
    QPushButton* options[4];
    QPushButton* btnNext;
    QPushButton* correctBtn;

    MemoryHelper* m_memoryHelper;
    QList<int> m_reviewWordQueue;
    int m_currentIndex;
    QList<QVariantMap> wordList;
    QString correctMeaning;
    int correctCount;
};

#endif // FASTREVIEWWIDGET_H