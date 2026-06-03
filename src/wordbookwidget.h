#ifndef WORDBOOKWIDGET_H
#define WORDBOOKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSqlQuery>
#include <QMap>

class WordBookWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WordBookWidget(QWidget *parent = nullptr);

signals:
    void returnToMainWindow();

private slots:
    // 顶部切换按钮
    void onBtnAllClicked();
    void onBtnLearnedClicked();
    void onBtnUnlearnedClicked();
    void onSearch();
    void onBack();

    // 数据加载
    void loadAllWords(const QString &searchFilter = "");
    void loadLearnedWords(const QString &searchFilter = "");
    void loadUnlearnedWords(const QString &searchFilter = "");

    // 懒加载滚动
    void onScrollChanged(int val);
    void loadNextPage();

    // 弹窗与工具函数
    void showWordFullDetail(int wordId);
    void clearLayout(QLayout *layout);
    void toggleStar(int wordId, bool star);

private:
    // 界面控件
    QLineEdit*      searchEdit;
    QPushButton*    btnAll;
    QPushButton*    btnLearned;
    QPushButton*    btnUnlearned;
    QVBoxLayout*    contentLayout;
    QWidget*        contentWidget;

    // 数据库
    QSqlQuery*      query;

    // 筛选标记
    QString         currentMode;

    // 懒加载参数（仅本类使用）
    int                     m_pageSize;
    int                     m_curLoadedCnt;
    bool                    m_isLoading;
    QList<QMap<QString,QString>> m_allWordCache;
};

#endif // WORDBOOKWIDGET_H