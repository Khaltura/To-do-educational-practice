#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QCalendarWidget>
#include <QTimeEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include "DatabaseManager.h"

class TaskWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskWidget(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~TaskWidget();

    void setSelectedDate(const QDate& date);

signals:
    void tasksUpdatedForDate(const QDate& date, const QList<QString>& tasks);

private slots:
    void refreshTasks();
    void addTask();
    void filterTasksByTag(const QString& tag);
    void openDatePopup();
    void openTimePopup();
    void openTagPopup();
    void onTasksUpdated();

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
    void clearTasks();
    void addTaskItem(const QMap<QString, QVariant>& taskData);
    void updateTagFilter();
    QString formatTaskText(const QString& text, const QString& date, const QString& time, const QString& tag) const;

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
};

#endif // TASKWIDGET_H
