#include "relatepage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDialog>
#include <QPlainTextEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>

RelatePage::RelatePage(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("单词关联组");
    setFixedSize(750,550);

    QHBoxLayout *mainLay = new QHBoxLayout(this);
    mainLay->setSpacing(15);
    mainLay->setContentsMargins(20,20,20,20);

    QVBoxLayout *leftLay = new QVBoxLayout;
    groupListWidget = new QListWidget;
    btnNewGroup = new QPushButton("新建关联分组");
    btnAddWord = new QPushButton("本组添加单词");
    btnDelGroup = new QPushButton("删除选中分组");
    QPushButton *btnReturn = new QPushButton("返回");

    leftLay->addWidget(groupListWidget);
    leftLay->addWidget(btnNewGroup);
    leftLay->addWidget(btnAddWord);
    leftLay->addWidget(btnDelGroup);
    leftLay->addWidget(btnReturn);

    QVBoxLayout *rightLay = new QVBoxLayout;
    wordDetailList = new QListWidget;
    rightLay->addWidget(wordDetailList);

    mainLay->addLayout(leftLay, 1);
    mainLay->addLayout(rightLay, 1);

    connect(btnNewGroup, &QPushButton::clicked, this, &RelatePage::onCreateNewGroup);
    connect(btnAddWord, &QPushButton::clicked, this, &RelatePage::onAddWord);
    connect(btnDelGroup, &QPushButton::clicked, this, &RelatePage::onDeleteGroup);
    connect(groupListWidget, &QListWidget::itemClicked, this, &RelatePage::onGroupItemClick);
    connect(btnReturn, &QPushButton::clicked, this, [=]() {
        emit back();
        close();
    });

    refreshGroupList();
}

RelatePage::~RelatePage()
{
}

void RelatePage::refreshGroupList()
{
    groupListWidget->clear();
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery query(db);
    query.exec("SELECT id, group_name FROM relate_group ORDER BY id");
    while (query.next()) {
        int gid = query.value(0).toInt();
        QString gname = query.value(1).toString();
        QListWidgetItem *item = new QListWidgetItem(gname, groupListWidget);
        item->setData(Qt::UserRole, gid);
    }
}

void RelatePage::refreshWordDetail(int groupId)
{
    wordDetailList->clear();
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT v.word, v.phonetic, v.meaning
        FROM relate_word rw
        JOIN vocabulary v ON rw.word_id = v.id
        WHERE rw.group_id = ?
    )");
    query.addBindValue(groupId);
    query.exec();
    while (query.next()) {
        QString w = query.value(0).toString();
        QString ph = query.value(1).toString();
        QString m = query.value(2).toString();
        wordDetailList->addItem(QString("%1 | %2 | %3").arg(w, ph, m));
    }
}

void RelatePage::onCreateNewGroup()
{
    QString name = QInputDialog::getText(this, "新建分组", "分组名称：");
    if (name.isEmpty()) return;

    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery query(db);
    query.prepare("INSERT INTO relate_group (group_name) VALUES (?)");
    query.addBindValue(name);
    query.exec();

    refreshGroupList();
}

void RelatePage::onAddWord()
{
    QListWidgetItem *item = groupListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "提示", "请先选中分组！");
        return;
    }
    int gid = item->data(Qt::UserRole).toInt();

    QDialog dlg(this);
    dlg.setWindowTitle("添加单词");
    dlg.setFixedSize(450, 340);
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    QPlainTextEdit *edit = new QPlainTextEdit;
    lay->addWidget(edit);
    QPushButton *btnFinish = new QPushButton("结束录入");
    lay->addWidget(btnFinish);

    connect(btnFinish, &QPushButton::clicked, &dlg, [&]() {
        QStringList words = edit->toPlainText().split("\n", Qt::SkipEmptyParts);
        insertWordToGroup(words, gid);
        dlg.accept();
        refreshWordDetail(gid);
    });

    dlg.exec();
}

void RelatePage::insertWordToGroup(QStringList wordList, int groupId)
{
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    for (QString w : wordList) {
        QString word = w.trimmed();
        if (word.isEmpty()) continue;

        QSqlQuery q(db);
        q.prepare("SELECT id FROM vocabulary WHERE word = ?");
        q.addBindValue(word);
        q.exec();
        int wid = -1;

        if (q.next()) {
            wid = q.value(0).toInt();
        } else {
            QString phon = QInputDialog::getText(this, "新词", "音标：");
            QString mean = QInputDialog::getText(this, "新词", "释义：");
            QSqlQuery ins(db);
            ins.prepare("INSERT INTO vocabulary (word, phonetic, meaning) VALUES (?,?,?)");
            ins.addBindValue(word);
            ins.addBindValue(phon);
            ins.addBindValue(mean);
            ins.exec();
            wid = ins.lastInsertId().toInt();
        }

        QSqlQuery rw(db);
        rw.prepare("INSERT INTO relate_word (group_id, word_id) VALUES (?,?)");
        rw.addBindValue(groupId);
        rw.addBindValue(wid);
        rw.exec();
    }
}

void RelatePage::onGroupItemClick(QListWidgetItem *item)
{
    int gid = item->data(Qt::UserRole).toInt();
    refreshWordDetail(gid);
}

void RelatePage::onDeleteGroup()
{
    QListWidgetItem *item = groupListWidget->currentItem();
    if (!item) return;
    int gid = item->data(Qt::UserRole).toInt();

    QSqlQuery q(QSqlDatabase::database("wordconn"));
    q.exec("DELETE FROM relate_word WHERE group_id = " + QString::number(gid));
    q.exec("DELETE FROM relate_group WHERE id = " + QString::number(gid));

    refreshGroupList();
    wordDetailList->clear();
}