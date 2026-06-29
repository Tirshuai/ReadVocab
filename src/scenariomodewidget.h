#ifndef SCENARIOMODEWIDGET_H
#define SCENARIOMODEWIDGET_H

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QSqlQuery>
#include <QStringList>
#include <QUrl>
#include "aitextgenerator.h"

class ScenarioModeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScenarioModeWidget(QWidget *parent = nullptr);

signals:
    void backToLearnNew();

private slots:
    void startGenerateScenario();
    void onArticleGenerated(const QString &article);
    void onWordClicked(const QUrl &url);
    void showQuizDialog(int wordId, const QString &word, const QString &correctMeaning);
    void onBack();

private:
    // 文本换行清洗工具函数
    QString cleanMultiLineText(const QString &raw);
    // 保存学习答题记录
    void saveRecord(int wordId, bool correct);

    QTextBrowser *textBrowser;
    QPushButton *btnGenerate;
    QPushButton *btnBack;
    QSqlQuery *query;
    AiTextGenerator *aiGenerator;
    QStringList currentWords;
};

#endif // SCENARIOMODEWIDGET_H