
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
#include <QTextDocument>
#include <QCoreApplication>
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) :
    QObject(parent), m_loggedIn(false), m_currentUserId(-1), m_currentDbMode(PersonalDb)
{
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

QString DatabaseManager::getProjectRootPath() const
{
    // Получаем путь к исполняемому файлу
    QString exePath = QCoreApplication::applicationDirPath();
    QDir dir(exePath);

    // Поднимаемся вверх по папкам, пока не найдем признак корня проекта
    // (например, папку .git или файл .project)
    while (!dir.isRoot()) {
        if (dir.exists(".git") || dir.exists("CMakeLists.txt") ||
            dir.exists("KursToDo.pro")) {
            return dir.path();
        }
        if (!dir.cdUp()) break;
    }

    // Если не нашли признаков проекта, возвращаем папку с exe
    return exePath;
}

QString DatabaseManager::getDatabaseDirectory() const
{
    QString dbDir = getProjectRootPath() + "/databases/";

    // Создаем папку, если ее нет
    QDir dir(dbDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qCritical() << "Cannot create database directory!";
            // Fallback - используем папку с exe
            return QCoreApplication::applicationDirPath() + "/databases/";
        }
    }

    // Проверяем права на запись
    QFile testFile(dbDir + "test_write.tmp");
    if (!testFile.open(QIODevice::WriteOnly)) {
        qCritical() << "No write permissions in database directory!";
        testFile.remove();
    }

    return dbDir;
}

void DatabaseManager::initializeMainDatabase() {
    QString dbPath = getDatabaseDirectory() + "todo_app_main.db";
    qDebug() << "Main DB path:" << dbPath;

    m_mainDb = QSqlDatabase::addDatabase("QSQLITE", "main_connection");
    m_mainDb.setDatabaseName(dbPath);
    if (!m_mainDb.open()) {
        qCritical() << "Main database connection error:" << m_mainDb.lastError().text();
        return;
    }

    QSqlQuery query(m_mainDb);
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qCritical() << "Failed to enable foreign keys:" << query.lastError();
    }

    // Создаем таблицу users с явным указанием схемы (main)
    if (!query.exec("CREATE TABLE IF NOT EXISTS main.users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "username TEXT UNIQUE NOT NULL,"
                    "password_hash TEXT NOT NULL,"
                    "group_id TEXT)")) {
        qCritical() << "Failed to create users table:" << query.lastError();
    }

    // Создаем таблицу groups с явным указанием схемы
    if (!query.exec("CREATE TABLE IF NOT EXISTS main.groups ("
                    "id TEXT PRIMARY KEY,"
                    "name TEXT NOT NULL)")) {
        qCritical() << "Failed to create groups table:" << query.lastError();
    }
}
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
        qCritical() << "Ошибка подключения к персональной БД";
        return false;
    }

    initializeCurrentDatabase();
    m_currentDbMode = PersonalDb;
    m_currentUserGroup.clear();
    return true;
}
void DatabaseManager::debugCheckDatabase() {
    if (!m_currentDb.isOpen()) {
        qDebug() << "Database is not open!";
        return;
    }

    QSqlQuery q(m_currentDb);
    qDebug() << "=== Database Structure ===";

    // Проверяем таблицу notes
    if (q.exec("PRAGMA table_info(notes)")) {
        qDebug() << "Notes table columns:";
        while (q.next()) {
            qDebug() << "-" << q.value("name").toString()
            << q.value("type").toString()
            << (q.value("notnull").toInt() ? "NOT NULL" : "")
            << (q.value("pk").toInt() ? "PRIMARY KEY" : "");
        }
    }

    // Проверяем подключенные БД
    if (m_currentDbMode == GroupDb && q.exec("PRAGMA database_list")) {
        qDebug() << "Attached databases:";
        while (q.next()) {
            qDebug() << "-" << q.value("name").toString()
            << ":" << q.value("file").toString();
        }
    }

    // Проверяем наличие тестовой записи
    if (q.exec("SELECT COUNT(*) FROM notes")) {
        q.next();
        qDebug() << "Total notes in database:" << q.value(0).toInt();
    }
}

bool DatabaseManager::switchToGroupDatabase(const QString& groupId) {
    // Закрываем и удаляем текущее соединение
    if (m_currentDb.isOpen()) {
        QString connectionName = m_currentDb.connectionName();

        // Завершаем активные запросы до удаления соединения
        {
            QSqlQuery cleanup(m_currentDb);
            cleanup.finish();
        }

        m_currentDb.close();
        QSqlDatabase::removeDatabase(connectionName);
    }

    // Открываем групповую БД
    QString dbPath = getGroupDbPath(groupId);
    m_currentDb = QSqlDatabase::addDatabase("QSQLITE", "group_connection");
    m_currentDb.setDatabaseName(dbPath);

    if (!m_currentDb.open()) {
        qCritical() << "Ошибка подключения к групповой БД";
        return false;
    }

    // Устанавливаем режим
    m_currentDbMode = GroupDb;
    m_currentUserGroup = groupId;

    // Инициализируем структуру
    initializeCurrentDatabase();

    // Отладка: выведем список подключённых БД
    QSqlQuery dbgQuery(m_currentDb);
    if (dbgQuery.exec("PRAGMA database_list")) {
        qDebug() << "=== Подключённые базы данных ===";
        while (dbgQuery.next()) {
            qDebug() << "Alias:" << dbgQuery.value(1).toString()
            << "Файл:" << dbgQuery.value(2).toString();
        }
    } else {
        qDebug() << "Не удалось выполнить PRAGMA database_list:" << dbgQuery.lastError();
    }

    // Перенос задач из личной БД в групповую при первом входе
    if (m_currentUserGroup.isEmpty()) {
        QSqlDatabase personalDb = QSqlDatabase::database("personal_connection");
        if (personalDb.isOpen()) {
            QSqlQuery query(personalDb);
            if (query.exec("SELECT text, date, time, tag, completed FROM tasks")) {
                while (query.next()) {
                    QSqlQuery insert(m_currentDb);
                    insert.prepare("INSERT INTO tasks (text, date, time, tag, completed, user_id) "
                                   "VALUES (?, ?, ?, ?, ?, ?)");
                    insert.addBindValue(query.value(0));
                    insert.addBindValue(query.value(1));
                    insert.addBindValue(query.value(2));
                    insert.addBindValue(query.value(3));
                    insert.addBindValue(query.value(4));
                    insert.addBindValue(m_currentUserId);
                    insert.exec();
                }
            }
        }
    }

    return true;
}


QString DatabaseManager::getPersonalDbPath() const
{
    return getDatabaseDirectory() + QString("user_%1.db").arg(m_currentUserId);
}
QString DatabaseManager::getGroupDbPath(const QString& groupId) const
{
    return getDatabaseDirectory() + QString("group_%1.db").arg(groupId);
}

void DatabaseManager::initializeCurrentDatabase()
{
    if (!m_currentDb.isOpen()) {
        qCritical() << "База данных не открыта!";
        return;
    }

    QSqlQuery query(m_currentDb);

    // Включаем важные настройки SQLite
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qCritical() << "Не удалось включить внешние ключи:" << query.lastError();
    }

    if (!query.exec("PRAGMA journal_mode = WAL")) {
        qCritical() << "Не удалось установить режим журнала:" << query.lastError();
    }

    // Создаем таблицу notes
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS notes ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "content TEXT NOT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "user_id INTEGER)")) {
        qCritical() << "Ошибка создания таблицы notes:" << query.lastError();
    }

    // Создаем таблицу tasks
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS tasks ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "text TEXT NOT NULL,"
            "date DATE,"
            "time TIME,"
            "tag TEXT,"
            "completed BOOLEAN DEFAULT FALSE,"
            "user_id INTEGER,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)")) {
        qCritical() << "Ошибка создания таблицы tasks:" << query.lastError();
    }

    // Для группового режима прикрепляем основную БД
    if (m_currentDbMode == GroupDb) {
        QString mainDbPath = getDatabaseDirectory() + "todo_app_main.db";
        if (!QFile::exists(mainDbPath)) {
            qCritical() << "Файл основной БД не существует по пути:" << mainDbPath;
            return;
        }

        // Используем alias shared вместо main
        if (!query.exec(QString("ATTACH DATABASE '%1' AS shared").arg(mainDbPath.replace("'", "''")))) {
            qCritical() << "Ошибка подключения основной БД:" << query.lastError();
            return;
        }

        if (!query.exec("SELECT 1 FROM shared.sqlite_master WHERE type='table' AND name='users'")) {
            qCritical() << "Ошибка проверки таблицы users:" << query.lastError();
            return;
        }

        if (!query.next() || query.value(0).toInt() != 1) {
            qCritical() << "Таблица users не найдена в основной БД";
            return;
        }

        if (!query.exec("CREATE INDEX IF NOT EXISTS idx_tasks_user_id ON tasks(user_id)")) {
            qWarning() << "Не удалось создать индекс:" << query.lastError();
        }

        qDebug() << "Успешно подключена групповая БД с прикрепленной основной БД";
    }

    qDebug() << "База данных инициализирована в режиме:"
             << (m_currentDbMode == PersonalDb ? "Личная" : "Групповая");
}


bool DatabaseManager::isConnected() const
{
    return m_mainDb.isOpen() && m_currentDb.isOpen();
}

bool DatabaseManager::registerUser(const QString& username, const QString& password)
{
    qDebug() << "=== Регистрация нового пользователя ===";
    qDebug() << "Проверка подключения к основной БД:" << m_mainDb.isOpen();

    if (!m_mainDb.isOpen()) {
        qCritical() << "Основная БД не подключена!";
        return false;
    }

    // Начинаем транзакцию
    m_mainDb.transaction();

    try {
        // 1. Создаем запись о пользователе
        QSqlQuery query(m_mainDb);
        query.prepare("INSERT INTO users (username, password_hash) VALUES (?, ?)");
        query.addBindValue(username);
        query.addBindValue(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

        if (!query.exec()) {
            qCritical() << "Ошибка INSERT в users:" << query.lastError().text();
            m_mainDb.rollback();
            return false;
        }

        // 2. Получаем ID нового пользователя
        m_currentUserId = query.lastInsertId().toInt();
        m_currentUser = username;
        qDebug() << "Создан пользователь с ID:" << m_currentUserId;

        // 3. Создаем персональную БД
        QString personalDbPath = getPersonalDbPath();
        qDebug() << "Путь к персональной БД:" << personalDbPath;

        QFile dbFile(personalDbPath);
        if (dbFile.exists()) {
            qWarning() << "Файл персональной БД уже существует! Удаляем...";
            if (!dbFile.remove()) {
                qCritical() << "Не удалось удалить существующую БД!";
                m_mainDb.rollback();
                return false;
            }
        }

        // 4. Переключаемся на персональную БД
        if (!switchToPersonalDatabase()) {
            qCritical() << "Ошибка создания персональной БД";
            m_mainDb.rollback();
            return false;
        }

        // Если все успешно - коммитим транзакцию
        m_mainDb.commit();
        qDebug() << "Регистрация завершена успешно!";
        return true;
    } catch (...) {
        m_mainDb.rollback();
        qCritical() << "Исключение при регистрации пользователя";
        return false;
    }
}

bool DatabaseManager::loginUser(const QString& username, const QString& password) {
    // Переподключение, если БД закрыта
    if (!m_mainDb.isOpen()) {
        QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/todo_app_main.db";
        m_mainDb = QSqlDatabase::addDatabase("QSQLITE", "main_reconnect");
        m_mainDb.setDatabaseName(dbPath);
        if (!m_mainDb.open()) {
            qCritical() << "DB open error:" << m_mainDb.lastError();
            return false;
        }
    }

    // Проверка пользователя
    QSqlQuery query(m_mainDb);
    query.prepare("SELECT id, password_hash FROM users WHERE username = ?");
    query.addBindValue(username.trimmed());

    if (!query.exec() || !query.next()) {
        qDebug() << "User not found or query error:" << query.lastError();
        return false;
    }

    // Проверка пароля
    QString storedHash = query.value(1).toString();
    QString inputHash = QCryptographicHash::hash(password.trimmed().toUtf8(),
                                                 QCryptographicHash::Sha256).toHex();

    if (inputHash != storedHash) {
        qDebug() << "Password mismatch:\nStored:" << storedHash << "\nInput:" << inputHash;
        return false;
    }

    // Успешный вход
    m_currentUserId = query.value(0).toInt();
    m_currentUser = username.trimmed();
    m_loggedIn = true;

    // Переключение на персональную БД
    if (!switchToPersonalDatabase()) {
        qCritical() << "Failed to switch to personal DB";
        return false;
    }

    emit loggedIn();
    return true;
}

void DatabaseManager::logout()
{
    // Закрываем все активные запросы текущей БД
    if (m_currentDb.isOpen()) {
        // Создаем временный query объект для завершения операций
        QSqlQuery query(m_currentDb);
        query.finish();

        // Даем время на завершение операций
        QCoreApplication::processEvents();

        QString connectionName = m_currentDb.connectionName();
        m_currentDb.close();
        QSqlDatabase::removeDatabase(connectionName);
        qDebug() << "Закрыто соединение с текущей БД:" << connectionName;
    }

    // Сбрасываем состояние
    m_loggedIn = false;
    m_currentUserId = -1;
    m_currentUser.clear();
    m_currentUserGroup.clear();

    emit loggedOut();
    qDebug() << "Успешный выход. Текущий ID пользователя:" << m_currentUserId;
}


bool DatabaseManager::saveTask(const QString& text, const QDate& date, const QTime& time,
                               const QString& tag, bool completed)
{
    if (!isConnected() || !isLoggedIn()) {
        qDebug() << "Database not connected or user not logged in";
        return false;
    }

    QSqlQuery query(m_currentDb);

    if (m_currentDbMode == PersonalDb) {
        query.prepare("INSERT INTO tasks (text, date, time, tag, completed) "
                      "VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(text);
        query.addBindValue(date.isValid() ? date.toString(Qt::ISODate) : QVariant());
        query.addBindValue(time.isValid() ? time.toString("HH:mm") : QVariant());
        query.addBindValue(tag);
        query.addBindValue(completed);
    } else {
        query.prepare("INSERT INTO tasks (text, date, time, tag, completed, user_id) "
                      "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(text);
        query.addBindValue(date.isValid() ? date.toString(Qt::ISODate) : QVariant());
        query.addBindValue(time.isValid() ? time.toString("HH:mm") : QVariant());
        query.addBindValue(tag);
        query.addBindValue(completed);
        query.addBindValue(m_currentUserId);
    }

    if (!query.exec()) {
        qDebug() << "Failed to save task:" << query.lastError();
        return false;
    }

    qDebug() << "Task saved successfully. Mode:"
             << (m_currentDbMode == PersonalDb ? "Personal" : "Group")
             << "User ID:" << m_currentUserId;

    emit tasksUpdated();
    return true;
}

QList<QMap<QString, QVariant>> DatabaseManager::getTasks() const
{
    QList<QMap<QString, QVariant>> tasks;
    if (!isConnected()) {
        qDebug() << "Database not connected";
        return tasks;
    }

    QSqlQuery query(m_currentDb);
    QString queryStr;

    if (m_currentDbMode == PersonalDb) {
        queryStr = "SELECT id, text, date, time, tag, completed FROM tasks "
                   "ORDER BY date, time";
    } else {
        // Проверяем, что основная БД (shared) подключена
        QSqlQuery checkAttach(m_currentDb);
        if (!checkAttach.exec("SELECT 1 FROM shared.sqlite_master LIMIT 1")) {
            qCritical() << "Main (shared) database not attached:" << checkAttach.lastError();
            return tasks;
        }

        queryStr = "SELECT t.id, t.text, t.date, t.time, t.tag, t.completed, u.username "
                   "FROM tasks t "
                   "LEFT JOIN shared.users u ON t.user_id = u.id "
                   "ORDER BY t.date, t.time";
    }

    if (!query.exec(queryStr)) {
        qCritical() << "Failed to get tasks:" << query.lastError()
        << "Query:" << query.lastQuery();
        return tasks;
    }

    while (query.next()) {
        QMap<QString, QVariant> task;
        task["id"] = query.value(0);
        task["text"] = query.value(1);
        task["date"] = query.value(2);
        task["time"] = query.value(3);
        task["tag"] = query.value(4);
        task["completed"] = query.value(5);

        if (m_currentDbMode == GroupDb) {
            task["username"] = query.value(6);
        }

        tasks.append(task);
    }

    return tasks;
}


bool DatabaseManager::updateTask(int taskId, const QMap<QString, QVariant>& updates) {
    if (!isConnected() || !isLoggedIn() || updates.isEmpty()) return false;

    QStringList fields;
    QList<QVariant> values;

    for (auto it = updates.begin(); it != updates.end(); ++it) {
        fields.append(it.key() + " = ?");
        values.append(it.value());
    }

    QSqlQuery query(m_currentDb);
    QString queryStr = "UPDATE tasks SET " + fields.join(", ") + " WHERE id = ?";

    // Для группового режима убираем проверку user_id при обновлении статуса
    if (m_currentDbMode == GroupDb && !updates.contains("completed")) {
        queryStr += " AND user_id = ?";
    }

    query.prepare(queryStr);

    for (const QVariant& value : values) {
        query.addBindValue(value);
    }

    query.addBindValue(taskId);

    if (m_currentDbMode == GroupDb && !updates.contains("completed")) {
        query.addBindValue(m_currentUserId);
    }

    bool success = query.exec();
    if (success) emit tasksUpdated();
    return success;
}

bool DatabaseManager::removeTask(int taskId)
{
    QMutexLocker locker(&m_dbMutex); // Добавляем мьютекс для безопасности

    QSqlQuery query(m_currentDb);
    query.prepare("DELETE FROM tasks WHERE id = :id");
    query.bindValue(":id", taskId);

    if (!query.exec()) {
        qWarning() << "Ошибка удаления задачи:" << query.lastError().text();
        return false;
    }

    // Проверяем, действительно ли задача была удалена
    if (query.numRowsAffected() <= 0) {
        qWarning() << "Задача не найдена или уже удалена";
        return false;
    }

    emit tasksUpdated();
    return true;
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

bool DatabaseManager::createGroup(const QString& groupName, QString& groupCode) {
    if (!m_mainDb.isOpen()) {
        qCritical() << "Main database is not open!";
        return false;
    }

    if (!isLoggedIn()) {
        qCritical() << "User must be logged in to create a group";
        return false;
    }

    // Генерируем уникальный код группы
    groupCode = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);

    // Начинаем транзакцию
    m_mainDb.transaction();

    try {
        // 1. Добавляем запись о группе в основную БД
        QSqlQuery insertGroup(m_mainDb);
        insertGroup.prepare("INSERT INTO groups (id, name) VALUES (?, ?)");
        insertGroup.addBindValue(groupCode);
        insertGroup.addBindValue(groupName);

        if (!insertGroup.exec()) {
            qCritical() << "Failed to insert group record:" << insertGroup.lastError();
            m_mainDb.rollback();
            return false;
        }

        // 2. Обновляем запись пользователя, добавляя его в группу
        QSqlQuery updateUser(m_mainDb);
        updateUser.prepare("UPDATE users SET group_id = ? WHERE id = ?");
        updateUser.addBindValue(groupCode);
        updateUser.addBindValue(m_currentUserId);

        if (!updateUser.exec()) {
            qCritical() << "Failed to update user record:" << updateUser.lastError();
            m_mainDb.rollback();
            return false;
        }

        // 3. Создаем файл базы данных группы
        QString dbPath = getGroupDbPath(groupCode);
        QFile dbFile(dbPath);
        if (dbFile.exists()) {
            if (!dbFile.remove()) {
                qCritical() << "Failed to remove existing group DB file";
                m_mainDb.rollback();
                return false;
            }
        }

        // 4. Переключаемся на групповую БД и инициализируем ее
        if (!switchToGroupDatabase(groupCode)) {
            qCritical() << "Failed to switch to group database";
            m_mainDb.rollback();
            return false;
        }

        // 5. Проверяем, что таблицы создались
        QSqlQuery checkQuery(m_currentDb);
        if (!checkQuery.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='notes'") ||
            !checkQuery.next() ||
            checkQuery.value(0).toInt() == 0) {
            qCritical() << "Notes table was not created properly";
            m_mainDb.rollback();
            return false;
        }

        // Если все успешно - коммитим транзакцию
        m_mainDb.commit();

        // Обновляем состояние
        m_currentUserGroup = groupCode;
        emit groupChanged();

        qDebug() << "Group created successfully. Name:" << groupName << "Code:" << groupCode;
        qDebug() << "Group database path:" << dbPath;

        return true;

    } catch (const std::exception& e) {
        m_mainDb.rollback();
        qCritical() << "Exception during group creation:" << e.what();
        return false;
    }
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

bool DatabaseManager::saveNote(const QString& content) {
    if (!isConnected()) return false;

    QSqlQuery query(m_currentDb);
    if (m_currentDbMode == PersonalDb) {
        query.prepare("INSERT INTO notes (content) VALUES (?)");
        query.addBindValue(content);
    } else {
        query.prepare("INSERT INTO notes (content, user_id) VALUES (?, ?)");
        query.addBindValue(content);
        query.addBindValue(m_currentUserId); // Убедитесь, что m_currentUserId установлен
    }

    if (!query.exec()) {
        qCritical() << "Ошибка сохранения заметки:" << query.lastError();
        return false;
    }

    qDebug() << "Note saved. Mode:" << (m_currentDbMode == PersonalDb ? "Personal" : "Group")
             << "User ID:" << m_currentUserId << "Last insert ID:" << query.lastInsertId();

    emit notesUpdated();
    return true;
}

QList<QMap<QString, QVariant>> DatabaseManager::getNotes() const {
    QList<QMap<QString, QVariant>> notes;
    if (!isConnected()) return notes;

    QSqlQuery query(m_currentDb);
    query.prepare("SELECT id, content, created_at FROM notes ORDER BY created_at DESC");

    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> note;
            note["id"] = query.value("id");
            note["content"] = query.value("content");
            note["created_at"] = query.value("created_at");
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

bool DatabaseManager::deleteNote(int noteId) {
    if (!isConnected() || !isLoggedIn()) {
        qDebug() << "Delete note failed: DB not connected or user not logged in";
        return false;
    }

    qDebug() << "Attempting to delete note. ID:" << noteId
             << "User ID:" << m_currentUserId
             << "DB mode:" << (m_currentDbMode == PersonalDb ? "Personal" : "Group");

    QSqlQuery query(m_currentDb);
    QString sql;

    if (m_currentDbMode == PersonalDb) {
        sql = "DELETE FROM notes WHERE id = ?";
        query.prepare(sql);
        query.addBindValue(noteId);
    } else {
        sql = "DELETE FROM notes WHERE id = ? AND user_id = ?";
        query.prepare(sql);
        query.addBindValue(noteId);
        query.addBindValue(m_currentUserId);
    }

    if (!query.exec()) {
        qCritical() << "Delete note error:" << query.lastError()
        << "Executed SQL:" << sql;
        return false;
    }

    int affected = query.numRowsAffected();
    qDebug() << "Delete note query affected" << affected << "rows";

    if (affected > 0) {
        emit notesUpdated();
        return true;
    }

    // Если строк не затронуто, проверим существование заметки
    query.prepare("SELECT 1 FROM notes WHERE id = ?");
    query.addBindValue(noteId);
    if (query.exec() && query.next()) {
        qDebug() << "Note exists but doesn't belong to current user or other condition failed";
    } else {
        qDebug() << "Note with ID" << noteId << "does not exist";
    }

    return false;
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
QSqlError DatabaseManager::lastError() const
{
    // Возвращаем ошибку текущей БД, если она открыта
    if (m_currentDb.isOpen()) {
        return m_currentDb.lastError();
    }
    // Иначе возвращаем ошибку основной БД
    return m_mainDb.lastError();
}
