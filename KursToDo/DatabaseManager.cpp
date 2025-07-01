#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseManager::openDatabase(const QString &path)
{
    if (QSqlDatabase::contains("qt_sql_default_connection"))
        m_db = QSqlDatabase::database("qt_sql_default_connection");
    else
        m_db = QSqlDatabase::addDatabase("QSQLITE");

    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query;

    bool res1 = query.exec("CREATE TABLE IF NOT EXISTS Tasks ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                           "text TEXT NOT NULL, "
                           "date TEXT, "
                           "tag TEXT)");
    if (!res1) {
        qWarning() << "Failed to create Tasks table:" << query.lastError().text();
        return false;
    }

    bool res2 = query.exec("CREATE TABLE IF NOT EXISTS Notes ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                           "text TEXT NOT NULL)");
    if (!res2) {
        qWarning() << "Failed to create Notes table:" << query.lastError().text();
        return false;
    }

    query.exec("CREATE TABLE IF NOT EXISTS Users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "username TEXT UNIQUE,"
               "password TEXT)");

    return true;
}

bool DatabaseManager::addTask(const QString &text, const QString &date, const QString &tag)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Tasks (text, date, tag) VALUES (:text, :date, :tag)");
    query.bindValue(":text", text);
    query.bindValue(":date", date);
    query.bindValue(":tag", tag);
    if (!query.exec()) {
        qWarning() << "Failed to insert task:" << query.lastError().text();
        return false;
    }
    return true;
}

QSqlQuery DatabaseManager::getAllTasks()
{
    QSqlQuery query("SELECT id, text, date, tag FROM Tasks ORDER BY date");
    return query;
}

bool DatabaseManager::addNote(const QString &text)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Notes (text) VALUES (:text)");
    query.bindValue(":text", text);
    if (!query.exec()) {
        qWarning() << "Failed to insert note:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateNote(int id, const QString &text)
{
    QSqlQuery query;
    query.prepare("UPDATE Notes SET text = :text WHERE id = :id");
    query.bindValue(":text", text);
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "Failed to update note:" << query.lastError().text();
        return false;
    }
    return true;
}

QSqlQuery DatabaseManager::getNoteById(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, text FROM Notes WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();
    return query;
}
