#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QDate>
#include <QTime>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();

    // Основные методы
    bool isConnected() const;
    bool registerUser(const QString& username, const QString& password);
    bool loginUser(const QString& username, const QString& password);
    void logout();
    bool isLoggedIn() const;
    QString currentUser() const;
    QString currentUserGroup() const;
    bool userExists(const QString& username) const;
    QSqlError lastError() const;

    // Методы для работы с задачами
    bool saveTask(const QString& text, const QDate& date = QDate(),
                  const QTime& time = QTime(), const QString& tag = QString(),
                  bool completed = false);
    QList<QMap<QString, QVariant>> getTasks() const;
    bool updateTask(int taskId, const QMap<QString, QVariant>& updates);
    bool removeTask(int taskId);
    QList<QString> getTasksForDate(const QDate& date) const;
    QList<QString> getAvailableTags() const;
    QList<QDate> getDatesWithTasks() const;

    // Методы для работы с группами
    bool createGroup(const QString& groupName, QString &groupCode);
    bool joinGroup(const QString& groupCode);
    bool leaveGroup();
    QString getGroupName(const QString& groupId) const;
    QStringList getGroupMembers(const QString& groupId) const;

    // Методы для работы с заметками
    bool saveNote(const QString& content);
    QList<QMap<QString, QVariant>> getNotes() const;
    bool updateNote(int noteId, const QString& content);
    bool deleteNote(int noteId);


signals:
    void loggedIn();
    void loggedOut();
    void tasksUpdated();

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    void initializeDatabase();

    QSqlDatabase m_db;
    bool m_loggedIn;
    QString m_currentUser;
    QString m_currentUserGroup;
    int m_currentUserId;
};

#endif // DATABASEMANAGER_H
