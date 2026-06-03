#ifndef WORDRELATERWIDGET_H
#define WORDRELATERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

class WordRelaterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WordRelaterWidget(QWidget *parent = nullptr);

signals:
    void back();

private slots:
    void openRelatePage();

private:
    QPushButton *btnRelate;
    QPushButton *btnBack;
};

#endif