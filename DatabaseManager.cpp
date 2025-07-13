#include "DatabaseManager.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDate>
#include <QTime>
#include <QSqlError> // Добавляем этот заголовочный файл
#include <QUuid>     // Добавляем для генерации UUID
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) :
    QObject(parent), m_loggedIn(false), m_currentUserId(-1), m_currentDbMode(PersonalDb)
{
    // Удаляем все таблицы перед инициализацией
    dropAllTables();

    initializeMainDatabase();

    if (m_loggedIn) {
        switchToPersonalDatabase();
    }
}

DatabaseManager::~DatabaseManager()
{
    if (m_mainDb.isOpen()) {
        m_mainDb.close();
    }
    if (m_currentDb.isOpen()) {
        m_currentDb.close();
    }
}

void DatabaseManager::dropAllTables()
{
    // Удаляем файлы всех баз данных
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dbDir(dbPath);

    // Удаляем основную базу
    if (QFile::exists(dbPath + "/todo_app_main.db")) {
        QFile::remove(dbPath + "/todo_app_main.db");
    }

    // Удаляем персональные базы
    QStringList personalDbs = dbDir.entryList(QStringList() << "user_*.db", QDir::Files);
    for (const QString &dbName : personalDbs) {
        QFile::remove(dbPath + "/" + dbName);
    }

    // Удаляем групповые базы
    QStringList groupDbs = dbDir.entryList(QStringList() << "group_*.db", QDir::Files);
    for (const QString &dbName : groupDbs) {
        QFile::remove(dbPath + "/" + dbName);
    }

    // Создаем директорию заново
    QDir().mkpath(dbPath);
}

void DatabaseManager::initializeMainDatabase()
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    dbPath += "/todo_app_main.db";

    m_mainDb = QSqlDatabase::addDatabase("QSQLITE", "main_connection");
    m_mainDb.setDatabaseName(dbPath);

    if (!m_mainDb.open()) {
        qCritical() << "Main database connection error:" << m_mainDb.lastError().text();
        return;
    }

    QSqlQuery query(m_mainDb);
    query.exec("PRAGMA foreign_keys = ON");

    // Создаем таблицы пользователей и групп
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "username TEXT UNIQUE NOT NULL,"
               "password_hash TEXT NOT NULL,"
               "group_id TEXT)");

    query.exec("CREATE TABLE IF NOT EXISTS groups ("
               "id TEXT PRIMARY KEY,"
               "name TEXT NOT NULL)");
}

// ... Остальные методы остаются без изменений ...
// (Все остальные методы из вашего исходного кода должны быть сохранены как есть)

bool DatabaseManager::switchToPersonalDatabase()
{
    if (m_currentDb.isOpen()) {
        QString connectionName = m_currentDb.connectionName();
        m_currentDb.close();
        QSqlDatabase::removeDatabase(connectionName);
    }

    QString dbPath = getPersonalDbPath();
    m_currentDb = QSqlDatabase::addDatabase("QSQLITE", "personal_connection");
    m_currentDb.setDatabaseName(dbPath);

    if (!m_currentDb.open()) {
        qCritical() << "Personal database connection error:" << m_currentDb.lastError().text();
        return false;
    }

    initializeCurrentDatabase();
    m_currentDbMode = PersonalDb;
    return true;
}

bool DatabaseManager::switchToGroupDatabase(const QString& groupId)
{
    if (m_currentDb.isOpen()) {
        QString connectionName = m_currentDb.connectionName();
        m_currentDb.close();
        QSqlDatabase::removeDatabase(connectionName);
    }

    QString dbPath = getGroupDbPath(groupId);
    m_currentDb = QSqlDatabase::addDatabase("QSQLITE", "group_connection_" + groupId);
    m_currentDb.setDatabaseName(dbPath);

    if (!m_currentDb.open()) {
        qCritical() << "Group database connection error:" << m_currentDb.lastError().text();
        return false;
    }

    initializeCurrentDatabase();
    m_currentDbMode = GroupDb;
    return true;
}

QString DatabaseManager::getPersonalDbPath() const
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    dbPath += QString("/user_%1.db").arg(m_currentUserId);
    return dbPath;
}

QString DatabaseManager::getGroupDbPath(const QString& groupId) const
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    dbPath += QString("/group_%1.db").arg(groupId);
    return dbPath;
}

void DatabaseManager::initializeCurrentDatabase()
{
    QSqlQuery query(m_currentDb);
    query.exec("PRAGMA foreign_keys = ON");

    if (m_currentDbMode == PersonalDb) {
        // Персональные таблицы
        query.exec("CREATE TABLE IF NOT EXISTS tasks ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "text TEXT NOT NULL,"
                   "date TEXT,"
                   "time TEXT,"
                   "tag TEXT,"
                   "completed BOOLEAN DEFAULT 0)");

        query.exec("CREATE TABLE IF NOT EXISTS notes ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "content TEXT NOT NULL,"
                   "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)");
    } else {
        // Групповые таблицы
        query.exec("CREATE TABLE IF NOT EXISTS tasks ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "user_id INTEGER NOT NULL,"
                   "text TEXT NOT NULL,"
                   "date TEXT,"
                   "time TEXT,"
                   "tag TEXT,"
                   "completed BOOLEAN DEFAULT 0,"
                   "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE)");

        query.exec("CREATE TABLE IF NOT EXISTS notes ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "user_id INTEGER NOT NULL,"
                   "content TEXT NOT NULL,"
                   "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
                   "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE)");
    }
}

bool DatabaseManager::isConnected() const
{
    return m_mainDb.isOpen() && m_currentDb.isOpen();
}

bool DatabaseManager::registerUser(const QString& username, const QString& password)
{
    if (!isConnected()) return false;

    QString trimmedUser = username.trimmed();
    if (trimmedUser.length() < 3 || trimmedUser.length() > 20) return false;
    if (password.length() < 6) return false;

    QSqlQuery query(m_mainDb);
    query.prepare("INSERT INTO users (username, password_hash) VALUES (?, ?)");
    query.addBindValue(trimmedUser);
    query.addBindValue(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (!query.exec()) {
        qWarning() << "Registration failed:" << query.lastError();
        return false;
    }

    // Получаем ID нового пользователя
    query.exec("SELECT last_insert_rowid()");
    if (query.next()) {
        m_currentUserId = query.value(0).toInt();
        m_currentUser = trimmedUser;
        m_loggedIn = true;

        // Создаем персональную базу данных
        if (!switchToPersonalDatabase()) {
            qCritical() << "Failed to create personal database";
            return false;
        }

        emit loggedIn();
        return true;
    }

    return false;
}

bool DatabaseManager::loginUser(const QString& username, const QString& password)
{
    if (!isConnected()) return false;

    QSqlQuery query(m_mainDb);
    query.prepare("SELECT id, password_hash, group_id FROM users WHERE username = ?");
    query.addBindValue(username.trimmed());

    if (!query.exec() || !query.next()) return false;

    m_currentUserId = query.value(0).toInt();
    QString storedHash = query.value(1).toString();
    QString groupId = query.value(2).toString();

    if (QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex() == storedHash) {
        m_loggedIn = true;
        m_currentUser = username.trimmed();

        // Переключаемся на соответствующую базу данных
        if (!groupId.isEmpty()) {
            if (!switchToGroupDatabase(groupId)) {
                qCritical() << "Failed to switch to group database";
                return false;
            }
            m_currentUserGroup = groupId;
        } else {
            if (!switchToPersonalDatabase()) {
                qCritical() << "Failed to switch to personal database";
                return false;
            }
        }

        emit loggedIn();
        return true;
    }
    return false;
}

void DatabaseManager::logout()
{
    if (m_currentDb.isOpen()) {
        QString connectionName = m_currentDb.connectionName();
        m_currentDb.close();
        QSqlDatabase::removeDatabase(connectionName);
    }

    m_loggedIn = false;
    m_currentUser.clear();
    m_currentUserGroup.clear();
    m_currentUserId = -1;
    m_currentDbMode = PersonalDb;

    emit loggedOut();
}

bool DatabaseManager::isLoggedIn() const { return m_loggedIn; }
QString DatabaseManager::currentUser() const { return m_currentUser; }
QString DatabaseManager::currentUserGroup() const { return m_currentUserGroup; }

// Методы для работы с задачами
bool DatabaseManager::saveTask(const QString& text, const QDate& date, const QTime& time,
                               const QString& tag, bool completed)
{
    if (!isConnected() || !isLoggedIn()) return false;

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("INSERT INTO tasks (text, date, time, tag, completed) "
                      "VALUES (?, ?, ?, ?, ?)");
    } else {
        query.prepare("INSERT INTO tasks (user_id, text, date, time, tag, completed) "
                      "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(m_currentUserId);
    }

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

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("SELECT id, text, date, time, tag, completed FROM tasks "
                      "ORDER BY date, time");
    } else {
        query.prepare("SELECT id, text, date, time, tag, completed FROM tasks "
                      "WHERE user_id = ? ORDER BY date, time");
        query.addBindValue(m_currentUserId);
    }

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

    QSqlQuery query(m_currentDb);
    QString queryStr = "UPDATE tasks SET " + fields.join(", ") + " WHERE id = ?";

    if (m_currentDbMode == GroupDb) {
        queryStr += " AND user_id = ?";
    }

    query.prepare(queryStr);

    for (const QVariant& value : values) {
        query.addBindValue(value);
    }

    query.addBindValue(taskId);

    if (m_currentDbMode == GroupDb) {
        query.addBindValue(m_currentUserId);
    }

    bool success = query.exec();
    if (success) emit tasksUpdated();
    return success;
}

bool DatabaseManager::removeTask(int taskId)
{
    if (!isConnected() || !isLoggedIn()) return false;

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("DELETE FROM tasks WHERE id = ?");
    } else {
        query.prepare("DELETE FROM tasks WHERE id = ? AND user_id = ?");
        query.addBindValue(m_currentUserId);
    }

    query.addBindValue(taskId);

    bool success = query.exec();
    if (success) emit tasksUpdated();
    return success;
}

QList<QString> DatabaseManager::getTasksForDate(const QDate& date) const
{
    QList<QString> tasks;
    if (!isConnected() || !isLoggedIn() || !date.isValid()) return tasks;

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("SELECT text FROM tasks WHERE date = ?");
    } else {
        query.prepare("SELECT text FROM tasks WHERE user_id = ? AND date = ?");
        query.addBindValue(m_currentUserId);
    }

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

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("SELECT DISTINCT tag FROM tasks WHERE tag IS NOT NULL");
    } else {
        query.prepare("SELECT DISTINCT tag FROM tasks WHERE user_id = ? AND tag IS NOT NULL");
        query.addBindValue(m_currentUserId);
    }

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

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("SELECT DISTINCT date FROM tasks WHERE date IS NOT NULL");
    } else {
        query.prepare("SELECT DISTINCT date FROM tasks WHERE user_id = ? AND date IS NOT NULL");
        query.addBindValue(m_currentUserId);
    }

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

    // Создаем запись о группе в основной базе
    QSqlQuery query(m_mainDb);
    query.prepare("INSERT INTO groups (id, name) VALUES (?, ?)");
    query.addBindValue(groupCode);
    query.addBindValue(groupName);

    if (!query.exec()) return false;

    // Создаем отдельную базу данных для группы
    QString groupDbPath = getGroupDbPath(groupCode);
    QSqlDatabase groupDb = QSqlDatabase::addDatabase("QSQLITE", "group_" + groupCode);
    groupDb.setDatabaseName(groupDbPath);

    if (!groupDb.open()) {
        qCritical() << "Cannot create group database:" << groupDb.lastError();
        return false;
    }

    // Инициализируем структуру групповой базы
    QSqlQuery groupQuery(groupDb);
    groupQuery.exec("PRAGMA foreign_keys = ON");
    groupQuery.exec("CREATE TABLE IF NOT EXISTS tasks ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "user_id INTEGER NOT NULL,"
                    "text TEXT NOT NULL,"
                    "date TEXT,"
                    "time TEXT,"
                    "tag TEXT,"
                    "completed BOOLEAN DEFAULT 0,"
                    "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE)");

    groupQuery.exec("CREATE TABLE IF NOT EXISTS notes ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "user_id INTEGER NOT NULL,"
                    "content TEXT NOT NULL,"
                    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
                    "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE)");

    groupDb.close();

    // Присоединяем пользователя к группе
    return joinGroup(groupCode);
}

bool DatabaseManager::joinGroup(const QString& groupCode)
{
    if (!isConnected() || !isLoggedIn()) return false;

    // Проверяем существование группы
    QSqlQuery checkQuery(m_mainDb);
    checkQuery.prepare("SELECT 1 FROM groups WHERE id = ?");
    checkQuery.addBindValue(groupCode);

    if (!checkQuery.exec() || !checkQuery.next()) return false;

    // Обновляем информацию о группе пользователя
    QSqlQuery updateQuery(m_mainDb);
    updateQuery.prepare("UPDATE users SET group_id = ? WHERE id = ?");
    updateQuery.addBindValue(groupCode);
    updateQuery.addBindValue(m_currentUserId);

    if (!updateQuery.exec()) return false;

    // Переключаемся на групповую базу данных
    if (!switchToGroupDatabase(groupCode)) return false;

    m_currentUserGroup = groupCode;
    emit groupChanged();
    return true;
}

bool DatabaseManager::leaveGroup()
{
    if (!isConnected() || !isLoggedIn() || m_currentUserGroup.isEmpty()) return false;

    // Обновляем информацию о группе пользователя
    QSqlQuery query(m_mainDb);
    query.prepare("UPDATE users SET group_id = NULL WHERE id = ?");
    query.addBindValue(m_currentUserId);

    if (!query.exec()) return false;

    // Переключаемся обратно на персональную базу
    if (!switchToPersonalDatabase()) return false;

    m_currentUserGroup.clear();
    emit groupChanged();
    return true;
}

QString DatabaseManager::getGroupName(const QString& groupId) const
{
    if (!isConnected()) return QString();

    QSqlQuery query(m_mainDb);
    query.prepare("SELECT name FROM groups WHERE id = ?");
    query.addBindValue(groupId);

    return query.exec() && query.next() ? query.value(0).toString() : QString();
}

QStringList DatabaseManager::getGroupMembers(const QString& groupId) const
{
    QStringList members;
    if (!isConnected()) return members;

    QSqlQuery query(m_mainDb);
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

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("INSERT INTO notes (content) VALUES (?)");
    } else {
        query.prepare("INSERT INTO notes (user_id, content) VALUES (?, ?)");
        query.addBindValue(m_currentUserId);
    }

    query.addBindValue(content);

    bool success = query.exec();
    if (success) emit notesUpdated();
    return success;
}

QList<QMap<QString, QVariant>> DatabaseManager::getNotes() const
{
    QList<QMap<QString, QVariant>> notes;
    if (!isConnected() || !isLoggedIn()) return notes;

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("SELECT id, content FROM notes ORDER BY created_at DESC");
    } else {
        query.prepare("SELECT id, content FROM notes WHERE user_id = ? ORDER BY created_at DESC");
        query.addBindValue(m_currentUserId);
    }

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

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("UPDATE notes SET content = ? WHERE id = ?");
    } else {
        query.prepare("UPDATE notes SET content = ? WHERE id = ? AND user_id = ?");
        query.addBindValue(m_currentUserId);
    }

    query.addBindValue(content);
    query.addBindValue(noteId);

    bool success = query.exec();
    if (success) emit notesUpdated();
    return success;
}

bool DatabaseManager::deleteNote(int noteId)
{
    if (!isConnected() || !isLoggedIn()) return false;

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("DELETE FROM notes WHERE id = ?");
    } else {
        query.prepare("DELETE FROM notes WHERE id = ? AND user_id = ?");
        query.addBindValue(m_currentUserId);
    }

    query.addBindValue(noteId);

    bool success = query.exec();
    if (success) emit notesUpdated();
    return success;
}

bool DatabaseManager::userExists(const QString &login) const
{
    if (!isConnected()) return false;

    QSqlQuery query(m_mainDb);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(login.trimmed());

    if (!query.exec()) {
        qDebug() << "Ошибка при проверке существования пользователя:" << query.lastError().text();
        return false;
    }

    return query.next() && query.value(0).toInt() > 0;
}
