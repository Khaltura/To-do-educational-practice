#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QRandomGenerator>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) :
    QObject(parent),
    m_isLoggedIn(false)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::openDatabase(const QString &path)
{
    if (m_db.isOpen()) {
        return true;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QDir().mkpath(QFileInfo(path).path());
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qCritical() << "Не удалось открыть базу данных:" << m_db.lastError().text();
        return false;
    }

    if (!createTables()) {
        qCritical() << "Не удалось создать таблицы";
        m_db.close();
        return false;
    }

    return true;
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}

QSqlError DatabaseManager::lastError() const
{
    return m_db.lastError();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query;
    bool success = true;

    success &= query.exec("CREATE TABLE IF NOT EXISTS users ("
                          "login TEXT PRIMARY KEY, "
                          "password TEXT NOT NULL, "
                          "group_id TEXT DEFAULT '')");

    success &= query.exec("CREATE TABLE IF NOT EXISTS groups ("
                          "code TEXT PRIMARY KEY, "
                          "name TEXT NOT NULL, "
                          "admin TEXT NOT NULL)");

    if (!success) {
        qCritical() << "Ошибка создания таблиц:" << query.lastError().text();  // Исправленная строка
        return false;
    }

    return success;
}

bool DatabaseManager::userExists(const QString &login)
{
    if (!m_db.isOpen()) return false;

    QSqlQuery query;
    query.prepare("SELECT login FROM users WHERE login = ?");
    query.addBindValue(login);

    if (!query.exec()) {
        qCritical() << "Ошибка проверки пользователя:" << query.lastError().text();
        return false;
    }

    return query.next();
}

bool DatabaseManager::registerUser(const QString &login, const QString &password)
{
    if (!m_db.isOpen()) {
        qWarning() << "База данных не открыта";
        return false;
    }

    if (login.isEmpty() || password.isEmpty()) {
        qWarning() << "Логин и пароль должны быть заполнены";
        return false;
    }

    if (password.length() < 6) {
        qWarning() << "Пароль должен содержать минимум 6 символов";
        return false;
    }

    if (userExists(login)) {
        qWarning() << "Пользователь с таким логином уже существует";
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO users (login, password) VALUES (?, ?)");
    query.addBindValue(login);
    query.addBindValue(password);

    if (!query.exec()) {
        qCritical() << "Ошибка регистрации:" << query.lastError().text();
        return false;
    }

    qDebug() << "Пользователь зарегистрирован:" << login;
    return true;
}

bool DatabaseManager::loginUser(const QString &login, const QString &password)
{
    if (!m_db.isOpen()) return false;

    QSqlQuery query;
    query.prepare("SELECT login, password FROM users WHERE login = ?");
    query.addBindValue(login);

    if (!query.exec()) {
        qCritical() << "Ошибка входа:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        qWarning() << "Пользователь не найден";
        return false;
    }

    if (query.value(1).toString() != password) {
        qWarning() << "Неверный пароль";
        return false;
    }

    m_currentUser = query.value(0).toString();
    m_isLoggedIn = true;
    emit authStateChanged(true);
    return true;
}

void DatabaseManager::logout()
{
    m_currentUser.clear();
    m_isLoggedIn = false;
    emit authStateChanged(false);
}

bool DatabaseManager::isLoggedIn() const
{
    return m_isLoggedIn;
}

QString DatabaseManager::currentUser() const
{
    return m_currentUser;
}

bool DatabaseManager::createGroup(const QString &groupName, QString &groupCode)
{
    if (!m_db.isOpen() || !isLoggedIn()) return false;

    // Генерируем случайный 6-значный код группы
    groupCode = QString::number(100000 + QRandomGenerator::global()->bounded(900000));

    QSqlQuery query;
    query.prepare("INSERT INTO groups (code, name, admin) VALUES (?, ?, ?)");
    query.addBindValue(groupCode);
    query.addBindValue(groupName);
    query.addBindValue(m_currentUser);

    if (!query.exec()) {
        qCritical() << "Ошибка создания группы:" << query.lastError().text();
        return false;
    }

    // Обновляем группу текущего пользователя
    query.prepare("UPDATE users SET group_id = ? WHERE login = ?");
    query.addBindValue(groupCode);
    query.addBindValue(m_currentUser);

    if (!query.exec()) {
        qCritical() << "Ошибка обновления группы пользователя:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::joinGroup(const QString &groupCode)
{
    if (!m_db.isOpen() || !isLoggedIn()) return false;

    // Проверяем существование группы
    QSqlQuery query;
    query.prepare("SELECT code FROM groups WHERE code = ?");
    query.addBindValue(groupCode);

    if (!query.exec() || !query.next()) {
        qWarning() << "Группа не найдена";
        return false;
    }

    // Обновляем группу пользователя
    query.prepare("UPDATE users SET group_id = ? WHERE login = ?");
    query.addBindValue(groupCode);
    query.addBindValue(m_currentUser);

    if (!query.exec()) {
        qCritical() << "Ошибка входа в группу:" << query.lastError().text();
        return false;
    }

    return true;
}

QString DatabaseManager::currentUserGroup() const
{
    if (!m_db.isOpen() || !isLoggedIn()) return "";

    QSqlQuery query;
    query.prepare("SELECT group_id FROM users WHERE login = ?");
    query.addBindValue(m_currentUser);

    if (!query.exec() || !query.next()) {
        return "";
    }

    return query.value(0).toString();
}

QString DatabaseManager::getGroupName(const QString &groupCode) const
{
    if (!m_db.isOpen()) return "";

    QSqlQuery query;
    query.prepare("SELECT name FROM groups WHERE code = ?");
    query.addBindValue(groupCode);

    if (!query.exec() || !query.next()) {
        return "";
    }

    return query.value(0).toString();
}

QStringList DatabaseManager::getGroupMembers(const QString &groupCode) const
{
    QStringList members;
    if (!m_db.isOpen()) return members;

    QSqlQuery query;
    query.prepare("SELECT login FROM users WHERE group_id = ?");
    query.addBindValue(groupCode);

    if (!query.exec()) {
        return members;
    }

    while (query.next()) {
        members.append(query.value(0).toString());
    }

    return members;
}

bool DatabaseManager::leaveGroup()
{
    if (!m_db.isOpen() || !isLoggedIn()) return false;

    QSqlQuery query;
    query.prepare("UPDATE users SET group_id = '' WHERE login = ?");
    query.addBindValue(m_currentUser);

    if (!query.exec()) {
        qCritical() << "Ошибка выхода из группы:" << query.lastError().text();
        return false;
    }

    return true;
}
