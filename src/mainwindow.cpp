#include "mainwindow.h"
#include "learnnewwidget.h"
#include "wordlistwidget.h"
#include "wordbookmanager.h"
#include "reviewwidget.h"
#include "fastreviewwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QSettings>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("单词背诵软件");
    setFixedSize(500, 750);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(40, 25, 40, 25);

    QFont btnFont;
    btnFont.setPointSize(14);
    btnFont.setBold(true);
    QFont smallBtnFont;
    smallBtnFont.setPointSize(12);
    smallBtnFont.setBold(true);

    // ========== 词书信息栏 ==========
    lblWordbook = new QLabel();
    lblWordbook->setAlignment(Qt::AlignCenter);
    lblWordbook->setStyleSheet("color:#4fc3f7; font-size:13px; padding:4px;");
    lblWordbook->setText(QString::fromUtf8("📖 当前词书：")
                         + WordbookManager::displayName(WordbookManager::currentWordbook()));
    mainLayout->addWidget(lblWordbook);

    // 更换词书按钮（中等高度）
    btnChangeBook = new QPushButton("🔄 选择 / 更换词书");
    btnChangeBook->setFixedHeight(42);
    btnChangeBook->setFont(smallBtnFont);
    btnChangeBook->setStyleSheet(R"(
        QPushButton {
            background-color: #1a6dd4;
            color: white;
            font-weight: bold;
            border: none;
            border-radius: 6px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #1e88e5;
        }
    )");
    mainLayout->addWidget(btnChangeBook);


    // ========== 核心四大功能按钮（最大高度，主体） ==========
    btnLearnNew = new QPushButton("学习新词");
    btnReview = new QPushButton("复习旧词");
    btnWordList = new QPushButton("已学单词表");
    btnBook = new QPushButton("单词本");

    // 核心按钮统一高70，视觉突出
    const int mainBtnH = 70;
    btnLearnNew->setFixedHeight(mainBtnH);
    btnReview->setFixedHeight(mainBtnH);
    btnWordList->setFixedHeight(mainBtnH);
    btnBook->setFixedHeight(mainBtnH);

    btnLearnNew->setFont(btnFont);
    btnReview->setFont(btnFont);
    btnWordList->setFont(btnFont);
    btnBook->setFont(btnFont);


    // ✅ 设置API Key 按钮（中等高度，和换词书一档）
    btnSetApi = new QPushButton("🔑 设置 AI API 接口");
    btnSetApi->setFixedHeight(46);
    btnSetApi->setFont(smallBtnFont);
    btnSetApi->setStyleSheet(R"(
        QPushButton {
            background-color: #2196F3;
            color: white;
            font-weight: bold;
            border:none;
            border-radius:6px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
    )");

    // 重置记录（最小高度，弱化危险操作）
    btnReset = new QPushButton("重置学习记录");
    btnReset->setFixedHeight(38);
    btnReset->setFont(smallBtnFont);
    btnReset->setStyleSheet(R"(
        QPushButton {
            background-color: #ff4444;
            color: white;
            font-weight: bold;
            border: none;
            border-radius:6px;
        }
        QPushButton:hover {
            background-color: #cc0000;
        }
    )");


    // 按层级顺序添加布局
    mainLayout->addWidget(btnLearnNew);
    mainLayout->addWidget(btnReview);
    mainLayout->addWidget(btnWordList);
    mainLayout->addWidget(btnBook);

    mainLayout->addSpacing(8); // 主功能与次要按钮分隔一小段
    mainLayout->addWidget(btnSetApi);

    mainLayout->addStretch(); // 弹性留白，把重置按钮压到底部
    mainLayout->addWidget(btnReset);

    // 信号绑定（完全保留你原来的，无需改动）
    connect(btnChangeBook, &QPushButton::clicked, this, &MainWindow::onChangeWordbook);
    connect(btnLearnNew, &QPushButton::clicked, this, &MainWindow::onLearnNewClicked);
    connect(btnWordList, &QPushButton::clicked, this, &MainWindow::onWordListClicked);
    connect(btnReset,   &QPushButton::clicked, this, &MainWindow::onResetRecord);
    connect(btnReview, &QPushButton::clicked, this, &MainWindow::onReviewButtonClicked);
    connect(btnBook, &QPushButton::clicked, this, &MainWindow::onWordBookClicked);
    connect(btnSetApi, &QPushButton::clicked, this, &MainWindow::onSetApiKeyClicked);
}

MainWindow::~MainWindow()
{
}

// ==============================
// 配置弹窗：新增API端点输入框，保存三项配置：密钥/模型/接口地址
// ==============================
void MainWindow::onSetApiKeyClicked()
{
    QDialog d(this);
    d.setWindowTitle("AI 接口配置");
    d.setFixedSize(520, 320);
    QVBoxLayout *lay = new QVBoxLayout(&d);
    lay->setSpacing(12);
    lay->setContentsMargins(24,24,24,24);

    const QString defaultEndpoint = "https://open.bigmodel.cn/api/paas/v4/chat/completions";
    const QString defaultModel = "glm-4-flash";

    // 读取本地已有配置
    QSettings set("app_config.ini", QSettings::IniFormat);
    QString oldKey = set.value("API_KEY").toString();
    QString oldModel = set.value("MODEL_NAME", defaultModel).toString();
    QString oldEndpoint = set.value("API_ENDPOINT", defaultEndpoint).toString();

    QLineEdit *editKey = new QLineEdit(oldKey);
    editKey->setPlaceholderText("填入平台API Key");
    editKey->setEchoMode(QLineEdit::Password);

    QLineEdit *editModel = new QLineEdit(oldModel);
    editModel->setPlaceholderText("模型名称，默认 glm-4-flash");

    QLineEdit *editEndpoint = new QLineEdit(oldEndpoint);
    editEndpoint->setPlaceholderText("API接口地址，默认智谱官方地址");

    lay->addWidget(new QLabel("API Key："));
    lay->addWidget(editKey);
    lay->addWidget(new QLabel("模型名称："));
    lay->addWidget(editModel);
    lay->addWidget(new QLabel("API 端点地址："));
    lay->addWidget(editEndpoint);

    QHBoxLayout *btnLay = new QHBoxLayout;
    QPushButton *btnOk = new QPushButton("保存");
    QPushButton *btnCancel = new QPushButton("取消");
    btnLay->addWidget(btnOk);
    btnLay->addWidget(btnCancel);
    lay->addLayout(btnLay);

    connect(btnCancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(btnOk, &QPushButton::clicked, &d, &QDialog::accept);

    if(d.exec() != QDialog::Accepted)
        return;

    QString newKey = editKey->text().trimmed();
    QString newModel = editModel->text().trimmed();
    QString newEndpoint = editEndpoint->text().trimmed();

    if(newKey.isEmpty())
    {
        QMessageBox::warning(this,"提示","API Key不能为空！");
        return;
    }
    // 空值自动填充默认
    if(newModel.isEmpty()) newModel = defaultModel;
    if(newEndpoint.isEmpty()) newEndpoint = defaultEndpoint;

    // 写入三项配置
    set.setValue("API_KEY", newKey);
    set.setValue("MODEL_NAME", newModel);
    set.setValue("API_ENDPOINT", newEndpoint);
    QMessageBox::information(this, "✅ 成功", "API配置已保存！永久生效");
}

// ========== 更换词书 ==========
void MainWindow::onChangeWordbook()
{
    QStringList items;
    QStringList tags = WordbookManager::availableWordbooks();
    for (const QString &tag : tags) {
        items << WordbookManager::displayName(tag);
    }

    int currentIdx = tags.indexOf(WordbookManager::currentWordbook());
    if (currentIdx < 0) currentIdx = 0;

    bool ok = false;
    QString choice = QInputDialog::getItem(
        this,
        "选择 / 更换词书",
        "请选择要使用的词书：\n（学习记录与词书绑定，切换后需重新学习）",
        items,
        currentIdx,
        false,
        &ok
        );

    if (!ok || choice.isEmpty()) return;

    int idx = items.indexOf(choice);
    if (idx < 0 || idx >= tags.size()) return;

    QString newBook = tags[idx];
    if (newBook == WordbookManager::currentWordbook()) {
        QMessageBox::information(this, "提示", "已经是当前词书，无需更换。");
        return;
    }

    int confirm = QMessageBox::question(this,
                                        "确认更换词书",
                                        QString("确定要更换为 %1 吗？\n\n⚠ 学习记录与词书绑定，切换后使用新词书记录。")
                                            .arg(WordbookManager::displayName(newBook)),
                                        QMessageBox::Yes | QMessageBox::No);

    if (confirm != QMessageBox::Yes) return;

    if (WordbookManager::switchWordbook(newBook)) {
        lblWordbook->setText("📖 当前词书：" + WordbookManager::displayName(WordbookManager::currentWordbook()));
        QMessageBox::information(this, "✅ 成功", "已切换到：" + WordbookManager::displayName(newBook));
    } else {
        QMessageBox::critical(this, "❌ 失败", "词书切换失败");
    }
}

void MainWindow::onLearnNewClicked()
{
    this->hide();
    LearnNewWidget *w = new LearnNewWidget();
    w->show();
    connect(w, &LearnNewWidget::backToMain, this, [=]() {
        this->show();
        w->deleteLater();
    });
}

void MainWindow::onWordListClicked()
{
    this->hide();
    WordListWidget *w = new WordListWidget();
    connect(w, &WordListWidget::returnToMainWindow, this, [=]() {
        this->show();
        w->deleteLater();
    });
    w->show();
}

void MainWindow::onResetRecord()
{
    int choice1 = QMessageBox::warning(this,
                                       "⚠️  危险操作：重置学习记录",
                                       "本操作会清空【所有学习记录】\n\n• 单词库不会被删除\n• 清空：学习/复习/分组记录\n• 不可撤销！\n\n确定继续？",
                                       QMessageBox::Yes | QMessageBox::No);

    if (choice1 != QMessageBox::Yes) return;

    int choice2 = QMessageBox::critical(this,
                                        "❌ 最终确认",
                                        "确定清空所有学习记录？\n无法恢复！！",
                                        QMessageBox::Yes | QMessageBox::No);

    if (choice2 != QMessageBox::Yes) return;

    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery query(db);
    bool ok1 = query.exec("DELETE FROM learn_record");
    bool ok2 = query.exec("DELETE FROM review_history");
    bool ok3 = query.exec("DELETE FROM relate_word");

    if (ok1 && ok2 && ok3)
        QMessageBox::information(this, "✅ 成功", "已清空所有学习/复习/分组关联记录！");
    else
        QMessageBox::critical(this, "❌ 失败", "清空失败！");
}

void MainWindow::onReviewButtonClicked()
{
    this->hide();
    ReviewWidget *rw = new ReviewWidget;
    rw->show();
    connect(rw, &ReviewWidget::backToMain, this, [=](){
        rw->close();
        this->show();
    });
    connect(rw, &ReviewWidget::openFastReview, this, [=](){
        rw->hide();
        FastReviewWidget *fw = new FastReviewWidget;
        fw->startReview();
        fw->show();
        connect(fw, &FastReviewWidget::backToReview, this, [=](){
            fw->close();
            rw->show();
        });
    });
    connect(rw, &ReviewWidget::openScenarioReview, this, [=](){
        rw->hide();
        rw->show();
    });
}

void MainWindow::onWordBookClicked()
{
    this->hide();
    WordBookWidget *w = new WordBookWidget;
    w->show();
    connect(w, &WordBookWidget::returnToMainWindow, this, [=](){
        this->show();
        w->deleteLater();
    });
}
