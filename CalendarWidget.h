
#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QWidget>
#include <QCalendarWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QDate>
#include "TaskWidget.h"
#include "DatabaseManager.h"

class CalendarWidget : public QWidget {
    Q_OBJECT
public:
    CalendarWidget(TaskWidget *taskWidget, QWidget *parent = nullptr) : QWidget(parent), m_taskWidget(taskWidget) {
        QVBoxLayout *layout = new QVBoxLayout(this);

        calendar = new QCalendarWidget;
        layout->addWidget(calendar);

        connect(calendar, &QCalendarWidget::activated, this, &CalendarWidget::showTasksOnDate);
    }

private slots:
    void showTasksOnDate(const QDate &date) {
        // Получаем задачи через DatabaseManager вместо TaskWidget
        QList<QString> tasks = DatabaseManager::instance().getTasksForDate(date);

        QDialog dialog(this);
        dialog.setWindowTitle(QString("Задачи на %1").arg(date.toString("dd.MM.yyyy")));
        dialog.resize(400, 300);

        QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);

        if (tasks.isEmpty()) {
            QLabel *label = new QLabel("Задачи отсутствуют.");
            label->setAlignment(Qt::AlignCenter);
            dialogLayout->addWidget(label);
        } else {
            QListWidget *listWidget = new QListWidget;
            for (const QString &taskText : tasks) {
                listWidget->addItem(taskText);
            }
            dialogLayout->addWidget(listWidget);
        }

        QPushButton *closeBtn = new QPushButton("Закрыть");
        connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
        dialogLayout->addWidget(closeBtn);

        dialog.exec();
    }

private:
    QCalendarWidget *calendar;
    TaskWidget *m_taskWidget;
};

#endif // CALENDARWIDGET_H
