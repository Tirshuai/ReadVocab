#ifndef SCENARIOMODEWIDGET_H
#define SCENARIOMODEWIDGET_H

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QSqlQuery>
#include "aitextgenerator.h"

class ScenarioModeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScenarioModeWidget(QWidget *parent = nullptr);

    // 新增信号声明
signals:
    void backToLearnNew();

private slots:
    void startGenerateScenario();
    void onArticleGenerated(const QString &article);
    void onWordClicked(const QUrl &url);
    void showQuizDialog(int wordId, const QString &word, const QString &correctMeaning);
    void saveRecord(int wordId, bool correct);
    void onBack();

private:
    QTextBrowser *textBrowser;
    QPushButton *btnGenerate;
    QPushButton *btnBack;
    QSqlQuery *query;
    AiTextGenerator *aiGenerator;
    QStringList currentWords;
};

#endif // SCENARIOMODEWIDGET_H