#ifndef SCENARIOMODEWIDGET_H
#define SCENARIOMODEWIDGET_H

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QUrl>
#include <QDialog>
#include <QLabel>
#include <QRandomGenerator>

class ScenarioModeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScenarioModeWidget(QWidget *parent = nullptr);

signals:
    void backToLearnNew();

private slots:
    void loadScenario();
    void onWordClicked(const QUrl &url);
    void saveRecord(int wordId, bool ok);
    void onBack();

private:
    QTextBrowser *textBrowser;
    QPushButton *btnBack;
    QSqlQuery *query;

    void showQuizDialog(int wordId, const QString& word, const QString& correctMeaning);
};

#endif