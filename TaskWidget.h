
#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QWidget>
#include <QDate>
#include <QTime>
#include <QMap>
#include <QVariant>

class DatabaseManager;
class QVBoxLayout;
class QHBoxLayout;
class QLineEdit;
class QLabel;
class QPushButton;
class QScrollArea;
class QCheckBox;
class QComboBox;
class QFrame;

class TaskWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TaskWidget(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~TaskWidget();

    void setSelectedDate(const QDate& date);
    void refreshTasks();

    // Новый публичный метод для обработки выхода пользователя
    void handleUserLoggedOut() { clearTasks(); }

signals:
    void tasksUpdatedForDate(const QDate& date, const QList<QString>& tasks);

private slots:
    void onTasksUpdated();
    void filterTasksByTag(const QString& tag);

private:
    struct TaskItem {
        int id;
        QFrame* frame;
        QCheckBox* checkBox;
        QLabel* label;
        QLineEdit* edit;
        QPushButton* editBtn;
        QPushButton* saveBtn;
        QPushButton* removeBtn;
        QString tag;
    };

    void setupUI();
    void addTaskItem(const QMap<QString, QVariant>& taskData);
    void clearTasks();
    void updateTagFilter();
    void openDatePopup();
    void openTimePopup();
    void openTagPopup();
    void addTask();
    QString formatTaskText(const QString& text, const QString& date,
                           const QString& time, const QString& tag) const;

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
    QDate m_selectedDate;
    QTime m_selectedTime;
    QString m_selectedTag;
    QList<TaskItem*> m_tasks;
};

#endif // TASKWIDGET_H
