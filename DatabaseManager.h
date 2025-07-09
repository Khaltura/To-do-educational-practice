#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>  // Добавлен этот include
#include <QString>
#include <QStringList>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();

    bool openDatabase(const QString &path);
    bool isOpen() const;
    QSqlError lastError() const;

    bool userExists(const QString &login);
    bool registerUser(const QString &login, const QString &password);
    bool loginUser(const QString &login, const QString &password);
    void logout();
    bool isLoggedIn() const;
    QString currentUser() const;

    bool createGroup(const QString &groupName, QString &groupCode);
    bool joinGroup(const QString &groupCode);
    QString currentUserGroup() const;
    QString getGroupName(const QString &groupCode) const;
    QStringList getGroupMembers(const QString &groupCode) const;
    bool leaveGroup();

signals:
    void authStateChanged(bool isLoggedIn);

private:
    DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool createTables();

    QSqlDatabase m_db;
    QString m_currentUser;
    bool m_isLoggedIn;
};

#endif // DATABASEMANAGER_H
