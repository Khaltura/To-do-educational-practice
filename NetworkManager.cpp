// NetworkManager.cpp
#include "NetworkManager.h"
#include "qjsonobject.h"

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
    m_manager = new QNetworkAccessManager(this);
}

void NetworkManager::login(const QString &username, const QString &password) {
    QNetworkRequest request = createRequest("/login");
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;

    QNetworkReply *reply = m_manager->post(
        request,
        QJsonDocument(body).toJson()
        );

    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonObject response = QJsonDocument::fromJson(reply->readAll()).object();
            m_authToken = response["token"].toString();
            emit loginSuccess(response["user"].toObject());
        } else {
            emit loginFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

QNetworkRequest NetworkManager::createRequest(const QString &path) {
    QNetworkRequest request;
    request.setUrl(QUrl(m_baseUrl + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", "Bearer " + m_authToken.toUtf8());
    }
    return request;
}
