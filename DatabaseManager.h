#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QStringList>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();

    bool isConnected() const;
    QSqlError lastError() const;

    bool registerUser(const QString& username, const QString& password);
    bool loginUser(const QString& username, const QString& password);
    void logout();
    bool isLoggedIn() const;
    QString currentUser() const;
    QString currentUserGroup() const;
    bool userExists(const QString& username) const;

    bool createGroup(const QString& groupName, QString &groupCode);
    bool joinGroup(const QString& groupCode);
    bool leaveGroup();
    QString getGroupName(const QString& groupId) const;
    QStringList getGroupMembers(const QString& groupId) const;

signals:
    void authStateChanged();

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool checkDatabaseStructure() const;
    void initializeDatabase();

    QSqlDatabase m_db;
    bool m_loggedIn;
    QString m_currentUser;
    QString m_currentUserGroup;

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
};

#endif // DATABASEMANAGER_H
