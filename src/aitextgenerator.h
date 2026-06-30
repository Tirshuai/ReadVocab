#ifndef AITEXTGENERATOR_H
#define AITEXTGENERATOR_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QStringList>

class AiTextGenerator : public QObject
{
    Q_OBJECT
public:
    explicit AiTextGenerator(QObject *parent = nullptr);

    // 正确的成员函数（和调用方匹配）
    void generateStoryArticle(const QStringList& words);

private:
    QNetworkAccessManager *netMgr;

signals:
    void articleGenerated(QString article);
    void requestError(QString error);

private slots:
    void onReplyFinished(QNetworkReply *reply);
};

#endif
