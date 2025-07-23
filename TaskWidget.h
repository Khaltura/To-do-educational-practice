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
#include <QDate>
#include <QTime>
#include <QList>
#include <QMetaObject>
#include <QVariant>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

// Предварительное объявление класса DatabaseManager
class DatabaseManager;

// Тип для обновлений задачи
using TaskUpdates = QMap<QString, QVariant>;

class TaskWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskWidget(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~TaskWidget() override;

    // Запрещаем копирование и присваивание
    TaskWidget(const TaskWidget&) = delete;
    TaskWidget& operator=(const TaskWidget&) = delete;

private:
    // Внутренняя структура для хранения данных задачи и её виджетов
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

    // Указатель на менеджер базы данных
    DatabaseManager* m_dbManager;

    // Основные виджеты
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

    // Данные
    QList<TaskItem*> m_tasks;          // Список задач
    QDate m_selectedDate;              // Выбранная дата
    QTime m_selectedTime;              // Выбранное время
    QString m_selectedTag;             // Выбранный тег

    // Методы инициализации
    void setupUI();
    void setupConnections();

    // Работа с задачами
    void refreshTasks();
    void addTaskItem(const QMap<QString, QVariant>& taskData);
    void setupTaskItemConnections(TaskItem* item);
    void clearTasks();
    void updateTagFilter();
    void filterTasksByTag(const QString& tag);

    // Вспомогательные методы
    QString formatTaskText(const QString& text,
                           const QString& date,
                           const QString& time,
                           const QString& tag) const;

private slots:
    // Слоты для обработки пользовательских действий
    void openDatePopup();
    void openTimePopup();
    void openTagPopup();
    void addTask();
    void onTasksUpdated();

signals:
    // Сигналы для внешнего взаимодействия
    void taskAdded(int taskId);
    void taskUpdated(int taskId);
    void taskRemoved(int taskId);
};

#endif // TASKWIDGET_H
