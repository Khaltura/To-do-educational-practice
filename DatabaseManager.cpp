#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) :
    QObject(parent),
    m_loggedIn(false)
{
    // Проверяем наличие QCoreApplication
    if (!QCoreApplication::instance()) {
        qCritical() << "Error: QCoreApplication must be created first!";
        return;
    }

    // Настраиваем путь к базе данных
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    dbPath += "/todo_app.db";

    // Для разработки - удаляем старую БД (в продакшене убрать!)
    if (QFile::exists(dbPath)) {
        qDebug() << "Removing old database file...";
        QFile::remove(dbPath);
    }

    // Настраиваем подключение
    static int connectionCounter = 0;
    QString connectionName = QString("TODO_APP_CONN_%1").arg(++connectionCounter);

    m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Database connection error:" << m_db.lastError().text();
        return;
    }

    // Инициализируем структуру БД
    initializeDatabase();
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        QString connectionName = m_db.connectionName();
        m_db.close();
        QSqlDatabase::removeDatabase(connectionName);
    }
}

void DatabaseManager::initializeDatabase()
{
    QSqlQuery query(m_db);

    // Включаем поддержку внешних ключей
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:" << query.lastError();
    }

    // Создаем таблицы
    QStringList tables = {
        "CREATE TABLE IF NOT EXISTS groups ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL)",

        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "group_id TEXT,"
        "FOREIGN KEY(group_id) REFERENCES groups(id) ON DELETE SET NULL)"
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
    if (!isConnected()) {
        qWarning() << "Database not connected!";
        return false;
    }

    QString trimmedUser = username.trimmed();

    // Валидация ввода
    if (trimmedUser.length() < 3 || trimmedUser.length() > 20) {
        qDebug() << "Invalid username length (3-20 characters required)";
        return false;
    }

    if (password.length() < 6) {
        qDebug() << "Password too short (min 6 characters)";
        return false;
    }

    // Проверка существования пользователя
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT 1 FROM users WHERE username = ?");
    checkQuery.addBindValue(trimmedUser);

    if (!checkQuery.exec()) {
        qWarning() << "User check failed:" << checkQuery.lastError().text();
        return false;
    }

    if (checkQuery.next()) {
        qDebug() << "User already exists";
        return false;
    }

    // Хеширование пароля
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);

    // Вставка нового пользователя
    QSqlQuery insertQuery(m_db);
    insertQuery.prepare("INSERT INTO users (username, password_hash) VALUES (?, ?)");
    insertQuery.addBindValue(trimmedUser);
    insertQuery.addBindValue(hash);

    if (!insertQuery.exec()) {
        qCritical() << "Registration failed:"
                    << "\nError:" << insertQuery.lastError().text()
                    << "\nQuery:" << insertQuery.lastQuery()
                    << "\nBound values:" << insertQuery.boundValues();
        return false;
    }

    qDebug() << "User registered successfully:" << trimmedUser;
    return true;
}

bool DatabaseManager::loginUser(const QString& username, const QString& password)
{
    if (!isConnected()) {
        qWarning() << "Database not connected!";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT password_hash, group_id FROM users WHERE username = ?");
    query.addBindValue(username.trimmed());

    if (!query.exec()) {
        qWarning() << "Login query failed:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        qDebug() << "User not found";
        return false;
    }

    QByteArray storedHash = query.value(0).toByteArray();
    QByteArray inputHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);

    if (inputHash == storedHash) {
        m_loggedIn = true;
        m_currentUser = username.trimmed();
        m_currentUserGroup = query.value(1).toString();
        emit authStateChanged();
        qDebug() << "User logged in:" << m_currentUser;
        return true;
    }

    qDebug() << "Invalid password";
    return false;
}

void DatabaseManager::logout()
{
    m_loggedIn = false;
    m_currentUser.clear();
    m_currentUserGroup.clear();
    emit authStateChanged();
    qDebug() << "User logged out";
}

bool DatabaseManager::isLoggedIn() const
{
    return m_loggedIn;
}

QString DatabaseManager::currentUser() const
{
    return m_currentUser;
}

QString DatabaseManager::currentUserGroup() const
{
    return m_currentUserGroup;
}

bool DatabaseManager::userExists(const QString& username) const
{
    if (!isConnected()) return false;

    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username.trimmed());

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

bool DatabaseManager::createGroup(const QString& groupName, QString &groupCode)
{
    if (!isConnected()) return false;

    // Генерация 6-значного кода группы
    groupCode = QString::number(QRandomGenerator::global()->bounded(100000, 999999));

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO groups (id, name) VALUES (?, ?)");
    query.addBindValue(groupCode);
    query.addBindValue(groupName);

    if (!query.exec()) {
        qWarning() << "Failed to create group:" << query.lastError().text();
        return false;
    }

    // Обновляем группу текущего пользователя
    query.prepare("UPDATE users SET group_id = ? WHERE username = ?");
    query.addBindValue(groupCode);
    query.addBindValue(m_currentUser);

    if (!query.exec()) {
        qWarning() << "Failed to update user group:" << query.lastError().text();
        return false;
    }

    m_currentUserGroup = groupCode;
    emit authStateChanged();
    qDebug() << "Group created:" << groupName << "with code:" << groupCode;
    return true;
}

bool DatabaseManager::joinGroup(const QString& groupCode)
{
    if (!isConnected()) return false;

    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT 1 FROM groups WHERE id = ?");
    checkQuery.addBindValue(groupCode);

    if (!checkQuery.exec() || !checkQuery.next()) {
        qDebug() << "Group not found";
        return false;
    }

    QSqlQuery updateQuery(m_db);
    updateQuery.prepare("UPDATE users SET group_id = ? WHERE username = ?");
    updateQuery.addBindValue(groupCode);
    updateQuery.addBindValue(m_currentUser);

    if (!updateQuery.exec()) {
        qWarning() << "Failed to join group:" << updateQuery.lastError().text();
        return false;
    }

    m_currentUserGroup = groupCode;
    emit authStateChanged();
    qDebug() << "User joined group:" << groupCode;
    return true;
}

bool DatabaseManager::leaveGroup()
{
    if (!isConnected() || m_currentUserGroup.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET group_id = NULL WHERE username = ?");
    query.addBindValue(m_currentUser);

    if (!query.exec()) {
        qWarning() << "Failed to leave group:" << query.lastError().text();
        return false;
    }

    m_currentUserGroup.clear();
    emit authStateChanged();
    qDebug() << "User left group";
    return true;
}

QString DatabaseManager::getGroupName(const QString& groupId) const
{
    if (!isConnected()) return QString();

    QSqlQuery query(m_db);
    query.prepare("SELECT name FROM groups WHERE id = ?");
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        return QString();
    }
    return query.value(0).toString();
}

QStringList DatabaseManager::getGroupMembers(const QString& groupId) const
{
    QStringList members;

    if (!isConnected()) return members;

    QSqlQuery query(m_db);
    query.prepare("SELECT username FROM users WHERE group_id = ?");
    query.addBindValue(groupId);

    if (query.exec()) {
        while (query.next()) {
            members << query.value(0).toString();
        }
    }
    return members;
}

QSqlError DatabaseManager::lastError() const
{
    return m_db.lastError();
}
