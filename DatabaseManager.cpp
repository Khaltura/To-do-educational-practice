#include "DatabaseManager.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDate>
#include <QTime>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) :
    QObject(parent), m_loggedIn(false), m_currentUserId(-1)
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    dbPath += "/todo_app.db";



    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Database connection error:" << m_db.lastError().text();
        return;
    }

    initializeDatabase();
}

void DatabaseManager::initializeDatabase()
{
    QSqlQuery query;

    // Включаем поддержку внешних ключей
    query.exec("PRAGMA foreign_keys = ON");

    // Создаем таблицы
    QStringList tables = {
        // Пользователи
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL)",

        // Группы
        "CREATE TABLE IF NOT EXISTS groups ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL)",

        // Связь пользователей с группами
        "ALTER TABLE users ADD COLUMN group_id TEXT REFERENCES groups(id) ON DELETE SET NULL",

        // Задачи
        "CREATE TABLE IF NOT EXISTS tasks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "text TEXT NOT NULL,"
        "date TEXT,"
        "time TEXT,"
        "tag TEXT,"
        "completed BOOLEAN DEFAULT 0,"
        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE)",

        // Заметки
        "CREATE TABLE IF NOT EXISTS notes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "content TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE)"
    };

    for (const QString &table : tables) {
        if (!query.exec(table)) {
            qCritical() << "Table creation failed:" << query.lastError().text();
        }
    }
}

bool DatabaseManager::isConnected() const
{
    return m_db.isOpen();
}

bool DatabaseManager::registerUser(const QString& username, const QString& password)
{
    if (!isConnected()) return false;

    QString trimmedUser = username.trimmed();
    if (trimmedUser.length() < 3 || trimmedUser.length() > 20) return false;
    if (password.length() < 6) return false;

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password_hash) VALUES (?, ?)");
    query.addBindValue(trimmedUser);
    query.addBindValue(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    return query.exec();
}

bool DatabaseManager::loginUser(const QString& username, const QString& password)
{
    if (!isConnected()) return false;

    QSqlQuery query;
    query.prepare("SELECT id, password_hash, group_id FROM users WHERE username = ?");
    query.addBindValue(username.trimmed());

    if (!query.exec() || !query.next()) return false;

    m_currentUserId = query.value(0).toInt();
    QString storedHash = query.value(1).toString();

    if (QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex() == storedHash) {
        m_loggedIn = true;
        m_currentUser = username.trimmed();
        m_currentUserGroup = query.value(2).toString();
        emit loggedIn();
        return true;
    }
    return false;
}

void DatabaseManager::logout()
{
    m_loggedIn = false;
    m_currentUser.clear();
    m_currentUserGroup.clear();
    m_currentUserId = -1;
    emit loggedOut();
}

bool DatabaseManager::isLoggedIn() const { return m_loggedIn; }
QString DatabaseManager::currentUser() const { return m_currentUser; }
QString DatabaseManager::currentUserGroup() const { return m_currentUserGroup; }
QSqlError DatabaseManager::lastError() const { return m_db.lastError(); }

// Методы для работы с задачами
bool DatabaseManager::saveTask(const QString& text, const QDate& date, const QTime& time,
                               const QString& tag, bool completed)
{
    if (!isConnected() || !isLoggedIn()) return false;

    QSqlQuery query;
    query.prepare("INSERT INTO tasks (user_id, text, date, time, tag, completed) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(m_currentUserId);
    query.addBindValue(text);
    query.addBindValue(date.isValid() ? date.toString(Qt::ISODate) : QVariant());
    query.addBindValue(time.isValid() ? time.toString("HH:mm") : QVariant());
    query.addBindValue(tag);
    query.addBindValue(completed);

    bool success = query.exec();
    if (success) emit tasksUpdated();
    return success;
}

QList<QMap<QString, QVariant>> DatabaseManager::getTasks() const
{
    QList<QMap<QString, QVariant>> tasks;
    if (!isConnected() || !isLoggedIn()) return tasks;

    QSqlQuery query;
    query.prepare("SELECT id, text, date, time, tag, completed FROM tasks "
                  "WHERE user_id = ? ORDER BY date, time");
    query.addBindValue(m_currentUserId);

    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> task;
            task["id"] = query.value(0);
            task["text"] = query.value(1);
            task["date"] = query.value(2);
            task["time"] = query.value(3);
            task["tag"] = query.value(4);
            task["completed"] = query.value(5);
            tasks.append(task);
        }
    }
    return tasks;
}

bool DatabaseManager::updateTask(int taskId, const QMap<QString, QVariant>& updates)
{
    if (!isConnected() || !isLoggedIn() || updates.isEmpty()) return false;

    QStringList fields;
    QList<QVariant> values;

    for (auto it = updates.begin(); it != updates.end(); ++it) {
        fields.append(it.key() + " = ?");
        values.append(it.value());
    }

    QSqlQuery query;
    query.prepare("UPDATE tasks SET " + fields.join(", ") +
                  " WHERE id = ? AND user_id = ?");

    for (const QVariant& value : values) {
        query.addBindValue(value);
    }

    query.addBindValue(taskId);
    query.addBindValue(m_currentUserId);

    bool success = query.exec();
    if (success) emit tasksUpdated();
    return success;
}

bool DatabaseManager::removeTask(int taskId)
{
    if (!isConnected() || !isLoggedIn()) return false;

    QSqlQuery query;
    query.prepare("DELETE FROM tasks WHERE id = ? AND user_id = ?");
    query.addBindValue(taskId);
    query.addBindValue(m_currentUserId);

    bool success = query.exec();
    if (success) emit tasksUpdated();
    return success;
}

QList<QString> DatabaseManager::getTasksForDate(const QDate& date) const
{
    QList<QString> tasks;
    if (!isConnected() || !isLoggedIn() || !date.isValid()) return tasks;

    QSqlQuery query;
    query.prepare("SELECT text FROM tasks WHERE user_id = ? AND date = ?");
    query.addBindValue(m_currentUserId);
    query.addBindValue(date.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            tasks.append(query.value(0).toString());
        }
    }
    return tasks;
}

QList<QString> DatabaseManager::getAvailableTags() const
{
    QList<QString> tags;
    if (!isConnected() || !isLoggedIn()) return tags;

    QSqlQuery query;
    query.prepare("SELECT DISTINCT tag FROM tasks WHERE user_id = ? AND tag IS NOT NULL");
    query.addBindValue(m_currentUserId);

    if (query.exec()) {
        while (query.next()) {
            QString tag = query.value(0).toString();
            if (!tag.isEmpty()) tags.append(tag);
        }
    }
    return tags;
}

QList<QDate> DatabaseManager::getDatesWithTasks() const
{
    QList<QDate> dates;
    if (!isConnected() || !isLoggedIn()) return dates;

    QSqlQuery query;
    query.prepare("SELECT DISTINCT date FROM tasks WHERE user_id = ? AND date IS NOT NULL");
    query.addBindValue(m_currentUserId);

    if (query.exec()) {
        while (query.next()) {
            QDate date = QDate::fromString(query.value(0).toString(), Qt::ISODate);
            if (date.isValid()) dates.append(date);
        }
    }
    return dates;
}

// Методы для работы с группами
bool DatabaseManager::createGroup(const QString& groupName, QString &groupCode)
{
    if (!isConnected() || !isLoggedIn()) return false;

    groupCode = QString::number(QRandomGenerator::global()->bounded(100000, 999999));

    QSqlQuery query;
    query.prepare("INSERT INTO groups (id, name) VALUES (?, ?)");
    query.addBindValue(groupCode);
    query.addBindValue(groupName);

    if (!query.exec()) return false;

    query.prepare("UPDATE users SET group_id = ? WHERE id = ?");
    query.addBindValue(groupCode);
    query.addBindValue(m_currentUserId);

    if (!query.exec()) return false;

    m_currentUserGroup = groupCode;
    return true;
}

bool DatabaseManager::joinGroup(const QString& groupCode)
{
    if (!isConnected() || !isLoggedIn()) return false;

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT 1 FROM groups WHERE id = ?");
    checkQuery.addBindValue(groupCode);

    if (!checkQuery.exec() || !checkQuery.next()) return false;

    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE users SET group_id = ? WHERE id = ?");
    updateQuery.addBindValue(groupCode);
    updateQuery.addBindValue(m_currentUserId);

    if (!updateQuery.exec()) return false;

    m_currentUserGroup = groupCode;
    return true;
}

bool DatabaseManager::leaveGroup()
{
    if (!isConnected() || !isLoggedIn() || m_currentUserGroup.isEmpty()) return false;

    QSqlQuery query;
    query.prepare("UPDATE users SET group_id = NULL WHERE id = ?");
    query.addBindValue(m_currentUserId);

    if (!query.exec()) return false;

    m_currentUserGroup.clear();
    return true;
}

QString DatabaseManager::getGroupName(const QString& groupId) const
{
    if (!isConnected()) return QString();

    QSqlQuery query;
    query.prepare("SELECT name FROM groups WHERE id = ?");
    query.addBindValue(groupId);

    return query.exec() && query.next() ? query.value(0).toString() : QString();
}

QStringList DatabaseManager::getGroupMembers(const QString& groupId) const
{
    QStringList members;
    if (!isConnected()) return members;

    QSqlQuery query;
    query.prepare("SELECT username FROM users WHERE group_id = ?");
    query.addBindValue(groupId);

    if (query.exec()) {
        while (query.next()) {
            members << query.value(0).toString();
        }
    }
    return members;
}

// Методы для работы с заметками
bool DatabaseManager::saveNote(const QString& content)
{
    if (!isConnected() || !isLoggedIn()) return false;

    QSqlQuery query;
    query.prepare("INSERT INTO notes (user_id, content) VALUES (?, ?)");
    query.addBindValue(m_currentUserId);
    query.addBindValue(content);

    return query.exec();
}

QList<QMap<QString, QVariant>> DatabaseManager::getNotes() const
{
    QList<QMap<QString, QVariant>> notes;
    if (!isConnected() || !isLoggedIn()) return notes;

    QSqlQuery query;
    query.prepare("SELECT id, content FROM notes WHERE user_id = ? ORDER BY created_at DESC");
    query.addBindValue(m_currentUserId);

    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> note;
            note["id"] = query.value(0);
            note["content"] = query.value(1);
            notes.append(note);
        }
    }
    return notes;
}

bool DatabaseManager::updateNote(int noteId, const QString& content)
{
    if (!isConnected() || !isLoggedIn()) return false;

    QSqlQuery query;
    query.prepare("UPDATE notes SET content = ? WHERE id = ? AND user_id = ?");
    query.addBindValue(content);
    query.addBindValue(noteId);
    query.addBindValue(m_currentUserId);

    return query.exec();
}

bool DatabaseManager::deleteNote(int noteId)
{
    if (!isConnected() || !isLoggedIn()) return false;

    QSqlQuery query;
    query.prepare("DELETE FROM notes WHERE id = ? AND user_id = ?");
    query.addBindValue(noteId);
    query.addBindValue(m_currentUserId);

    return query.exec();
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}
bool DatabaseManager::userExists(const QString &login) const
{
    if (!isConnected()) return false;

    QSqlQuery query(m_db);  // Используем m_db вместо m_database
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(login.trimmed());

    if (!query.exec()) {
        qDebug() << "Ошибка при проверке существования пользователя:" << query.lastError().text();
        return false;
    }

    return query.next() && query.value(0).toInt() > 0;
}
