#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include "reviewwidget.h"
#include "fastreviewwidget.h"
#include "wordbookwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLearnNewClicked();
    void onWordListClicked();  // ✅ 单词关联器
    void onResetRecord();
    void onReviewButtonClicked();
    void onWordBookClicked();
    void onChangeWordbook();

private:
    QPushButton *btnSetApi;

private slots:
    void onSetApiKeyClicked();

private:
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;

    QLabel *lblWordbook;        // 显示当前词书
    QPushButton *btnChangeBook; // 更换词书
    QPushButton *btnLearnNew;
    QPushButton *btnReview;
    QPushButton *btnWordList;
    QPushButton *btnBook;
    QPushButton *btnReset;

};

#endif // MAINWINDOW_H
