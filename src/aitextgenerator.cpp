#include "aitextgenerator.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

AiTextGenerator::AiTextGenerator(QObject *parent)
    : QObject{parent}, netMgr(new QNetworkAccessManager(this))
{
    connect(netMgr, &QNetworkAccessManager::finished, this, &AiTextGenerator::onReplyFinished);
}

void AiTextGenerator::generateStoryArticle(const QStringList &words)
{
    // ✅ 从配置文件自动读取 API Key
    QSettings set("app_config.ini", QSettings::IniFormat);
    QString apiKey = set.value("API_KEY", "").toString().trimmed();

    if (apiKey.isEmpty()) {
        emit requestError("请先在主界面设置 AI API Key！");
        return;
    }

    QString wordStr = words.join(", ");
    QString prompt = QString(R"(你是英语出题老师，使用下面英文单词：%1，写一篇200词左右日常英文小故事，所有指定单词必须自然出现在正文（需要是常规用法）；只有一篇英语短文，不要多余内容。)")
                         .arg(wordStr);

    QNetworkRequest req{QUrl(URL)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());

    QJsonObject msgUser;
    msgUser["role"] = "user";
    msgUser["content"] = prompt;

    QJsonArray msgArr;
    msgArr.append(msgUser);

    QJsonObject reqJson;
    reqJson["model"] = "glm-4-flash";
    reqJson["messages"] = msgArr;

    netMgr->post(req, QJsonDocument(reqJson).toJson(QJsonDocument::Compact));
}

void AiTextGenerator::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit requestError("API请求失败：" + reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    QJsonArray choices = obj["choices"].toArray();

    if (choices.isEmpty()) {
        emit requestError("AI返回内容为空");
        reply->deleteLater();
        return;
    }

    QString content = choices[0].toObject()["message"].toObject()["content"].toString();
    emit articleGenerated(content);
    reply->deleteLater();
}
