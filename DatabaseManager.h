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

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();

    // Все существующие методы остаются без изменений
    bool isConnected() const;
    bool registerUser(const QString& username, const QString& password);
    bool loginUser(const QString& username, const QString& password);
    void logout();
    bool isLoggedIn() const;
    QString currentUser() const;
    QString currentUserGroup() const;
    QSqlError lastError() const;

    // Методы для работы с задачами (остаются без изменений)
    bool saveTask(const QString& text, const QDate& date, const QTime& time,
                  const QString& tag, bool completed);
    QList<QMap<QString, QVariant>> getTasks() const;
    bool updateTask(int taskId, const QMap<QString, QVariant>& updates);
    bool removeTask(int taskId);
    QList<QString> getTasksForDate(const QDate& date) const;
    QList<QString> getAvailableTags() const;
    QList<QDate> getDatesWithTasks() const;

    // Методы для работы с заметками (остаются без изменений)
    bool saveNote(const QString& content);
    QList<QMap<QString, QVariant>> getNotes() const;
    bool updateNote(int noteId, const QString& content);
    bool deleteNote(int noteId);

    // Методы для работы с группами (модифицируются)
    bool createGroup(const QString& groupName, QString &groupCode);
    bool joinGroup(const QString& groupCode);
    bool leaveGroup();
    QString getGroupName(const QString& groupId) const;
    QStringList getGroupMembers(const QString& groupId) const;
    bool userExists(const QString &login) const;

    // Новый метод для очистки базы данных
    void dropAllTables();

signals:
    void tasksUpdated();
    void notesUpdated();
    void loggedIn();
    void loggedOut();
    void groupChanged();

private:
    DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    enum DbMode {
        PersonalDb,
        GroupDb
    };

    void initializeMainDatabase();
    void initializeCurrentDatabase();
    bool switchToPersonalDatabase();
    bool switchToGroupDatabase(const QString& groupId);
    QString getPersonalDbPath() const;
    QString getGroupDbPath(const QString& groupId) const;

    QSqlDatabase m_mainDb;         // Основная база (пользователи и группы)
    QSqlDatabase m_currentDb;      // Текущая активная база (персональная или групповая)

    // Все существующие поля остаются
    bool m_loggedIn;
    QString m_currentUser;
    QString m_currentUserGroup;
    int m_currentUserId;
    DbMode m_currentDbMode;
};

#endif // DATABASEMANAGER_H
