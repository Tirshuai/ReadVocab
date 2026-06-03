#ifndef WORDLISTWIDGET_H
#define WORDLISTWIDGET_H

#include <QWidget>
#include <QCalendarWidget>
#include <QMap>
#include <QSqlQuery>

class QVBoxLayout;
class QScrollArea;
class QLineEdit;
class QPushButton;

class WordListWidget : public QWidget
{
    Q_OBJECT
signals:
    void returnToMainWindow();

public:
    explicit WordListWidget(QWidget *parent = nullptr);

public slots:
    void backToMain();

private slots:
    void loadLearnedWords(const QString &filter);
    void toggleStar(int wordId, bool star);
    void showWordDetail(int wordId);
    void onCalendarClick(const QDate &dt);       // 点击日历 → 弹窗
    void showOnlyStarred();
    void showAllWords();
    void showWordFullDetail(int wordId);

    // 懒加载
    void loadNextPage();
    void onScrollChanged(int val);

private:
    void clearLayout(QLayout *layout);

    QLineEdit *searchEdit;
    QWidget *contentWidget;
    QVBoxLayout *contentLayout;
    QSqlQuery *query;

    // 懒加载缓存
    QList<QMap<QString, QString>> m_allWordCache;
    int m_pageSize;
    int m_curLoadedCnt;
    bool m_isLoading;

    QCalendarWidget *calendar;

    QPushButton *btnStarred;
    QPushButton *btnAll;
};

#endif