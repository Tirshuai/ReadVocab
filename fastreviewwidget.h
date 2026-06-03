#ifndef FASTREVIEWWIDGET_H
#define FASTREVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QList>
#include <QVariantMap>

class FastReviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FastReviewWidget(QWidget *parent = nullptr);
    void startReview();

signals:
    void backToReview();

private slots:
    void onOptionClicked();
    void nextQuestion();
    void showFinishDialog();

private:
    void initUI();
    void loadLast5Words();
    void showCurrentQuestion();
    QStringList getRandomWrongMeanings(const QString &correct, int count);

    QLabel *labelWord, *labelProgress;
    QPushButton *options[4];
    QPushButton *btnNext;
    QPushButton *correctBtn;

    QList<QVariantMap> wordList;
    int currentIndex = 0;
    int correctCount = 0;
    QString correctMeaning; // 加这行
};
#endif // FASTREVIEWWIDGET_H