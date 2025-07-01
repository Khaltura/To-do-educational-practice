//DatabaseManager.h
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool openDatabase(const QString &path);
    bool createTables();

    // Методы для задач
    bool addTask(const QString &text, const QString &date, const QString &tag);
    QSqlQuery getAllTasks();

    // Методы для заметок
    bool addNote(const QString &text);
    bool updateNote(int id, const QString &text);
    QSqlQuery getNoteById(int id);

    // Получение задачи по id
    QSqlQuery getTaskById(int id)
    {
        QSqlQuery query;
        query.prepare("SELECT id, text, date, tag FROM Tasks WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        return query;
    }

    // Обновление задачи
    bool updateTask(int id, const QString &text, const QString &date, const QString &tag)
    {
        QSqlQuery query;
        query.prepare("UPDATE Tasks SET text = :text, date = :date, tag = :tag WHERE id = :id");
        query.bindValue(":text", text);
        query.bindValue(":date", date);
        query.bindValue(":tag", tag);
        query.bindValue(":id", id);
        if (!query.exec()) {
            qWarning() << "Failed to update task:" << query.lastError().text();
            return false;
        }
        return true;
    }

    // Удаление задачи
    bool deleteTask(int id)
    {
        QSqlQuery query;
        query.prepare("DELETE FROM Tasks WHERE id = :id");
        query.bindValue(":id", id);
        if (!query.exec()) {
            qWarning() << "Failed to delete task:" << query.lastError().text();
            return false;
        }
        return true;
    }

private:
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
