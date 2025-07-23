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
#include <QVariant>

// Тип для обновлений задач
using TaskUpdates = QMap<QString, QVariant>;

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
    int currentUserId() const { return m_currentUserId; }

    // Task methods - улучшенный интерфейс для TaskWidget
    bool saveTask(const QString& text, const QDate& date = QDate(),
                  const QTime& time = QTime(), const QString& tag = QString(),
                  bool completed = false);
    QList<QMap<QString, QVariant>> getTasks() const;
    Q_INVOKABLE bool updateTask(int taskId, const TaskUpdates& updates);
    Q_INVOKABLE bool removeTask(int taskId);
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

    // Debug methods
    void debugCheckDatabase();
    void dropAllTables();

signals:
    // Улучшенные сигналы для TaskWidget
    void tasksUpdated();
    void taskUpdateFailed(int taskId);
    void taskAdded(int taskId);
    void taskRemoved(int taskId);

    // Существующие сигналы
    void notesUpdated();
    void loggedIn();
    void loggedOut();
    void groupChanged();
    void notesChanged(const QList<QMap<QString, QVariant>>& notes);
    void taskUpdated(int taskId, bool success);
    void taskRemoved(int taskId, bool success);

private:
    DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    void initializeMainDatabase();
    void initializeCurrentDatabase();
    bool switchToPersonalDatabase();
    bool switchToGroupDatabase(const QString& groupId);
    QString getPersonalDbPath() const;
    QString getGroupDbPath(const QString& groupId) const;

    QSqlDatabase m_mainDb;
    QSqlDatabase m_currentDb;

    bool m_loggedIn;
    QString m_currentUser;
    QString m_currentUserGroup;
    int m_currentUserId;
    DbMode m_currentDbMode;
};

#endif // DATABASEMANAGER_H
