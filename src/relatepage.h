#ifndef RELATEPAGE_H
#define RELATEPAGE_H

#include <QWidget>
#include <QListWidgetItem>

class QListWidget;
class QPushButton;

class RelatePage : public QWidget
{
    Q_OBJECT
public:
    explicit RelatePage(QWidget *parent = nullptr);
    ~RelatePage();

signals:
    void back();

private slots:
    void onCreateNewGroup();
    void onAddWord();
    void onDeleteGroup();
    void onGroupItemClick(QListWidgetItem *item);

private:
    void refreshGroupList();
    void refreshWordDetail(int groupId);
    void insertWordToGroup(QStringList wordList, int groupId);

    QListWidget *groupListWidget;
    QListWidget *wordDetailList;
    QPushButton *btnNewGroup;
    QPushButton *btnAddWord;
    QPushButton *btnDelGroup;
};

#endif