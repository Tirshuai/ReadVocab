#ifndef REVIEWSITUATIONWIDGET_H
#define REVIEWSITUATIONWIDGET_H

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>
#include <QSqlQuery>
#include "aitextgenerator.h"

class ReviewSituationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReviewSituationWidget(QWidget *parent = nullptr);

signals:
    void backToReview();

private slots:
    void startGenerateScenario();
    void onArticleGenerated(const QString &article);
    void onWordClicked(const QUrl &url);
    void showQuizDialog(int wordId, const QString &word, const QString &correctMeaning);
    void saveReviewRecord(int wordId, const QString &word, bool correct);
    void onBack();

private:
    QStringList getLatest5LearnedWords();
    void loadWordMeaning(const QString &word, int &outWordId, QString &outMeaning);

    QLabel *title;
    QPushButton *btnGenerate;
    QTextBrowser *textBrowser;
    QPushButton *btnBack;

    QStringList currentWords;
    QSqlQuery *query;
    AiTextGenerator *aiGenerator;
};

#endif // REVIEWSITUATIONWIDGET_H