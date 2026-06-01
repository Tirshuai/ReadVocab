#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLearnNewClicked();

private:
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;

    QPushButton *btnLearnNew;
    QPushButton *btnReview;
    QPushButton *btnWordList;
    QPushButton *btnLinker;
    QPushButton *btnBook;
    QPushButton *btnData;
};

#endif