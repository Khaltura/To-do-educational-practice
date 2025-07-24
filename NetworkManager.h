#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

// NetworkManager.h
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void login(const QString &username, const QString &password);
    void registerUser(const QString &username, const QString &password);
    void getTasks();
    void saveTask(const QMap<QString, QVariant> &task);
    void updateTask(int id, const QMap<QString, QVariant> &updates);
    void deleteTask(int id);

signals:
    void loginSuccess(const QJsonObject &userData);
    void loginFailed(const QString &error);
    void tasksReceived(const QJsonArray &tasks);
    void taskUpdated(int id);
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager *m_manager;
    QString m_baseUrl = "http://your-python-server:5000/api";
    QString m_authToken;

    QNetworkRequest createRequest(const QString &path);
    void handleError(QNetworkReply *reply);
};

#endif // NETWORKMANAGER_H
