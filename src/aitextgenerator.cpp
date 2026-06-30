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
    const QString defaultEndpoint = "https://open.bigmodel.cn/api/paas/v4/chat/completions";
    const QString defaultModel = "glm-4-flash";

    // 读取三项配置：密钥、模型、接口地址
    QSettings set("app_config.ini", QSettings::IniFormat);
    QString apiKey = set.value("API_KEY", "").toString().trimmed();
    QString modelName = set.value("MODEL_NAME", defaultModel).toString().trimmed();
    QString endpoint = set.value("API_ENDPOINT", defaultEndpoint).toString().trimmed();

    if (apiKey.isEmpty()) {
        emit requestError("请先在主界面设置 AI API Key！");
        return;
    }

    QString wordStr = words.join(", ");
    QString prompt = QString(R"(你是英语出题老师。请使用以下英文单词：%1，写一篇200词左右的日常英文小故事。

严格要求：
1. 所有指定单词必须自然出现在短文中（可以使用常见变形，如复数、过去式、现在分词等）。
2. 每个指定单词（或其变形）在文中出现时，都必须用双星号括起来标记，例如：**apple**、**apples**、**explained**。
3. 只输出一篇英语短文，不要任何额外说明、标题或注释。)")
                         .arg(wordStr);

    // 使用配置内自定义端点
    QNetworkRequest req{QUrl(endpoint)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());

    QJsonObject msgUser;
    msgUser["role"] = "user";
    msgUser["content"] = prompt;

    QJsonArray msgArr;
    msgArr.append(msgUser);

    QJsonObject reqJson;
    reqJson["model"] = modelName;
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
