#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QRandomGenerator>
#include <QDate>
#include <QTime>
#include <QSqlError>
#include <QUuid>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    enum DbMode {
        PersonalDb,
        GroupDb
    };

    static DatabaseManager& instance();

    // Database mode methods
    DbMode currentDbMode() const { return m_currentDbMode; }
    bool isGroupMode() const { return m_currentDbMode == GroupDb; }

    // Core methods
    bool isConnected() const;
    bool registerUser(const QString& username, const QString& password);
    bool loginUser(const QString& username, const QString& password);
    void logout();
    bool isLoggedIn() const { return m_loggedIn; }
    QString currentUser() const { return m_currentUser; }
    QString currentUserGroup() const { return m_currentUserGroup; }
    QSqlError lastError() const;

    // Task methods
    bool saveTask(const QString& text, const QDate& date, const QTime& time,
                  const QString& tag, bool completed);
    QList<QMap<QString, QVariant>> getTasks() const;
    bool updateTask(int taskId, const QMap<QString, QVariant>& updates);
    bool removeTask(int taskId);
    QList<QString> getTasksForDate(const QDate& date) const;
    QList<QString> getAvailableTags() const;
    QList<QDate> getDatesWithTasks() const;

    // Note methods
    bool saveNote(const QString& content);
    QList<QMap<QString, QVariant>> getNotes() const;
    bool updateNote(int noteId, const QString& content);
    bool deleteNote(int noteId);

    // Group methods
    bool createGroup(const QString& groupName, QString& groupCode);
    bool joinGroup(const QString& groupCode);
    bool leaveGroup();
    QString getGroupName(const QString& groupId) const;
    QStringList getGroupMembers(const QString& groupId) const;
    bool userExists(const QString &login) const;
    int currentUserId() const { return m_currentUserId; }

    // Debug methods
    void debugCheckDatabase();
    void dropAllTables();

signals:
    void tasksUpdated();
    void notesUpdated();
    void loggedIn();
    void loggedOut();
    void groupChanged();
    void notesChanged(const QList<QMap<QString, QVariant>>& notes);

private:
    DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    void initializeMainDatabase();
    void initializeCurrentDatabase();
    bool switchToPersonalDatabase();
    bool switchToGroupDatabase(const QString& groupId);
    QString getPersonalDbPath() const;
    QString getGroupDbPath(const QString& groupId) const;

    QSqlDatabase m_mainDb;         // Main database (users and groups)
    QSqlDatabase m_currentDb;      // Current active database (personal or group)

    bool m_loggedIn;
    QString m_currentUser;
    QString m_currentUserGroup;
    int m_currentUserId;
    DbMode m_currentDbMode;
};

#endif // DATABASEMANAGER_H
