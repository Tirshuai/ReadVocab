#ifndef REVIEWSITUATIONWIDGET_H
#define REVIEWSITUATIONWIDGET_H

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>
#include <QSqlQuery>
#include <QUrl>
#include "aitextgenerator.h"
#include "memoryhelper.h"

class ReviewSituationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReviewSituationWidget(QWidget *parent = nullptr);

private:
    QString cleanMultiLineText(const QString &raw);
    // 替换原取最新单词函数，改为艾宾浩斯抽取5个待复习单词
    QStringList getScenarioReviewWords(int takeCount = 5);

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
    void loadWordMeaning(const QString &word, int &outWordId, QString &outMeaning);

    QLabel *title;
    QPushButton *btnGenerate;
    QTextBrowser *textBrowser;
    QPushButton *btnBack;

    QStringList currentWords;
    QSqlQuery *query;
    AiTextGenerator *aiGenerator;
    MemoryHelper *m_memoryHelper; // 新增记忆算法工具类
};

#endif // REVIEWSITUATIONWIDGET_H