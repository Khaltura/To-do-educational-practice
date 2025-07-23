#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QWidget>
#include <QMap>
#include <QFrame>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QScrollArea>
#include <QMap>
#include <QVariant>

typedef QMap<QString, QVariant> TaskUpdates;
#include "DatabaseManager.h"

class TaskWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskWidget(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~TaskWidget();

private:
    struct TaskItem {
        int id = -1;                      // ID задачи в базе данных
        QString tag;                      // Тег задачи
        bool isBeingDeleted = false;      // Флаг для отслеживания процесса удаления

        // Виджеты
        QFrame* frame = nullptr;          // Основной фрейм задачи
        QCheckBox* checkBox = nullptr;    // Чекбокс выполнения
        QLabel* label = nullptr;          // Текст задачи
        QLineEdit* edit = nullptr;        // Поле редактирования
        QPushButton* editBtn = nullptr;   // Кнопка редактирования
        QPushButton* saveBtn = nullptr;   // Кнопка сохранения
        QPushButton* removeBtn = nullptr; // Кнопка удаления

        // Хранение соединений сигналов/слотов
        QList<QMetaObject::Connection> connections;

        // Деструктор для правильной очистки
        ~TaskItem() {
            // Отключаем все соединения
            for (const auto& connection : connections) {
                QObject::disconnect(connection);
            }
            connections.clear();

            // Удаляем виджеты (они будут удалены автоматически как children фрейма)
            if (frame) {
                frame->deleteLater();
            }
        }
    };

    DatabaseManager* m_dbManager;
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QComboBox* m_tagFilterCombo;
    QLineEdit* m_taskInput;
    QPushButton* m_dateBtn;
    QPushButton* m_timeBtn;
    QPushButton* m_tagBtn;
    QPushButton* m_addBtn;
    QScrollArea* m_scrollArea;
    QWidget* m_containerWidget;
    QVBoxLayout* m_taskLayout;
    QList<TaskItem*> m_tasks;
    QDate m_selectedDate;
    QTime m_selectedTime;
    QString m_selectedTag;

    void setupUI();
    void setupConnections();
    void refreshTasks();
    void addTaskItem(const QMap<QString, QVariant>& taskData);
    void setupTaskItemConnections(TaskItem* item);
    void clearTasks();
    void updateTagFilter();
    void filterTasksByTag(const QString& tag);
    QString formatTaskText(const QString& text, const QString& date,
                           const QString& time, const QString& tag) const;

private slots:
    void openDatePopup();
    void openTimePopup();
    void openTagPopup();
    void addTask();
    void onTasksUpdated();
};

#endif // TASKWIDGET_H
