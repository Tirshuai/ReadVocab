#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
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
    void onWordListClicked();
    void onWordRelaterClicked();   // ✅ 单词关联器
    void onResetRecord();
    void onReviewButtonClicked();
    void onWordBookClicked();

private:
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;

    QPushButton *btnLearnNew;
    QPushButton *btnReview;
    QPushButton *btnWordList;
    QPushButton *btnLinker;
    QPushButton *btnBook;
    QPushButton *btnData;
    QPushButton *btnReset;
};

#endif // MAINWINDOW_H