#ifndef WORDLISTWIDGET_H
#define WORDLISTWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QMap>

class WordListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WordListWidget(QWidget *parent = nullptr);

signals:
    void backToMain();

private slots:
    void loadLearnedWords(const QString &filter = "");
    void toggleStar(int wordId, bool star);
    void showWordDetail(int wordId);

private:
    void clearLayout(QLayout *layout);

    QLineEdit *searchEdit;
    QVBoxLayout *contentLayout;
    QWidget *contentWidget;
    QSqlQuery *query;
    QMap<QString, QList<QMap<QString, QString>>> groupedWords;
};

#endif