#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QCalendarWidget>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDate>
#include <QTime>
#include <QTimeEdit>
#include <QFrame>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QSet>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>

class TaskWidget : public QWidget {
    Q_OBJECT
public:
    TaskWidget(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QLabel *title = new QLabel("📋 Задачи");
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 24px; font-weight: bold;");
        mainLayout->addWidget(title);

        tagFilterCombo = new QComboBox;
        tagFilterCombo->addItem("Все теги");
        tagFilterCombo->setStyleSheet(
            "QComboBox { font-size: 14px; padding: 4px 8px; background-color: #1e1e1e; color: white; border: 1px solid #555555; border-radius: 6px; }"
            "QComboBox:hover { border-color: #0078d7; }"
            );
        mainLayout->addWidget(tagFilterCombo);
        connect(tagFilterCombo, &QComboBox::currentTextChanged, this, &TaskWidget::filterTasksByTag);

        QHBoxLayout *inputLayout = new QHBoxLayout;
        taskInput = new QLineEdit;
        taskInput->setPlaceholderText("Введите новую задачу...");
        taskInput->setStyleSheet(
            "QLineEdit { background-color: #1e1e1e; color: #ffffff; font-size: 16px; padding: 8px 12px; border: 2px solid #3a3a3a; border-radius: 8px; }"
            "QLineEdit:focus { border-color: #0078d7; background-color: #252526; }"
            "QLineEdit:hover { border-color: #555555; }"
            );
        inputLayout->addWidget(taskInput);

        QPushButton *dateBtn = new QPushButton("📅");
        QPushButton *timeBtn = new QPushButton("⏱");
        QPushButton *tagBtn = new QPushButton("🏷");
        QPushButton *addBtn = new QPushButton("➕");

        inputLayout->addWidget(dateBtn);
        inputLayout->addWidget(timeBtn);
        inputLayout->addWidget(tagBtn);
        inputLayout->addWidget(addBtn);

        mainLayout->addLayout(inputLayout);

        scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);

        containerWidget = new QWidget;
        taskLayout = new QVBoxLayout(containerWidget);
        taskLayout->setAlignment(Qt::AlignTop);
        containerWidget->setLayout(taskLayout);

        scrollArea->setWidget(containerWidget);
        mainLayout->addWidget(scrollArea);

        connect(dateBtn, &QPushButton::clicked, this, &TaskWidget::openDatePopup);
        connect(timeBtn, &QPushButton::clicked, this, &TaskWidget::openTimePopup);
        connect(tagBtn, &QPushButton::clicked, this, &TaskWidget::openTagPopup);
        connect(addBtn, &QPushButton::clicked, this, &TaskWidget::addTask);

        loadTasksFromFile();
    }

signals:
    void tasksForDateRequested(const QString &date, const QList<QString> &tasksTexts);

private:
    struct TaskItem {
        QString text;
        QString date;
        QString time;
        QString tag;
        bool completed = false;

        QFrame *frame = nullptr;
        QLabel *label = nullptr;
        QLineEdit *edit = nullptr;
        QPushButton *editBtn = nullptr;
        QPushButton *saveBtn = nullptr;
        QPushButton *removeBtn = nullptr;
        QCheckBox *checkBox = nullptr;
    };

    QLineEdit *taskInput;
    QScrollArea *scrollArea;
    QWidget *containerWidget;
    QVBoxLayout *taskLayout;
    QString selectedDate;
    QString selectedTime;
    QString selectedTag;
    QComboBox *tagFilterCombo;

    QList<TaskItem*> tasks;

    const QString tasksFile = "tasks.json";

    void openDatePopup() {
        QDialog dialog(this);
        dialog.setWindowTitle("Выберите дату");

        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        QCalendarWidget *calendar = new QCalendarWidget;
        layout->addWidget(calendar);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, [&]() {
            selectedDate = calendar->selectedDate().toString("dd.MM.yyyy");
            dialog.accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        dialog.exec();
    }

    void openTimePopup() {
        QDialog dialog(this);
        dialog.setWindowTitle("Выберите время");

        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        QTimeEdit *timeEdit = new QTimeEdit(QTime::currentTime());
        timeEdit->setDisplayFormat("HH:mm");
        timeEdit->setStyleSheet("color: white; background-color: #1e1e1e;");
        layout->addWidget(timeEdit);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, [&]() {
            selectedTime = timeEdit->time().toString("HH:mm");
            dialog.accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        dialog.exec();
    }

    void openTagPopup() {
        bool ok;
        QString tag = QInputDialog::getText(this, "Введите тег", "Тег:", QLineEdit::Normal, "", &ok);
        if (ok && !tag.isEmpty()) {
            selectedTag = tag;
        }
    }

    void addTask() {
        QString text = taskInput->text().trimmed();
        if (text.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Задача не может быть пустой!");
            return;
        }

        addTaskItem(text, selectedDate, selectedTime, selectedTag, false);

        updateTagFilter();

        taskInput->clear();
        selectedDate.clear();
        selectedTime.clear();
        selectedTag.clear();

        filterTasksByTag(tagFilterCombo->currentText());

        saveTasksToFile();
    }

    void addTaskItem(const QString &text, const QString &date, const QString &time, const QString &tag, bool completed) {
        QString fullText = text;
        if (!date.isEmpty()) fullText += "  📅 " + date;
        if (!time.isEmpty()) fullText += "  ⏱ " + time;
        if (!tag.isEmpty()) fullText += "  🏷 " + tag;

        QFrame *taskFrame = new QFrame;
        taskFrame->setFrameShape(QFrame::Box);
        taskFrame->setStyleSheet("background-color: #2e2e2e; border-radius: 10px; padding: 10px;");
        QHBoxLayout *taskRow = new QHBoxLayout(taskFrame);

        QCheckBox *checkBox = new QCheckBox;
        checkBox->setChecked(completed);
        taskRow->addWidget(checkBox);

        QLabel *taskLabel = new QLabel(fullText);
        taskLabel->setStyleSheet(completed ? "color: gray; font-size: 16px; text-decoration: line-through;" : "color: white; font-size: 16px;");
        taskRow->addWidget(taskLabel);

        QLineEdit *taskEdit = new QLineEdit(fullText);
        taskEdit->setVisible(false);
        taskRow->addWidget(taskEdit);

        QPushButton *editBtn = new QPushButton("✏️");
        QPushButton *saveBtn = new QPushButton("💾");
        QPushButton *removeBtn = new QPushButton("❌");

        saveBtn->setVisible(false);
        saveBtn->setEnabled(false);

        taskRow->addWidget(editBtn);
        taskRow->addWidget(saveBtn);
        taskRow->addWidget(removeBtn);

        taskLayout->addWidget(taskFrame);

        TaskItem *item = new TaskItem{ text, date, time, tag, completed, taskFrame, taskLabel, taskEdit, editBtn, saveBtn, removeBtn, checkBox };
        tasks.append(item);

        connect(checkBox, &QCheckBox::stateChanged, this, [this, item](int state){
            item->completed = (state == Qt::Checked);
            item->label->setStyleSheet(item->completed ? "color: gray; font-size: 16px; text-decoration: line-through;" : "color: white; font-size: 16px;");
            saveTasksToFile();
        });

        connect(editBtn, &QPushButton::clicked, this, [item](){
            item->label->setVisible(false);
            item->edit->setText(item->label->text());
            item->edit->setVisible(true);
            item->editBtn->setVisible(false);
            item->saveBtn->setVisible(true);
            item->saveBtn->setEnabled(true);
        });

        connect(saveBtn, &QPushButton::clicked, this, [this, item](){
            QString newText = item->edit->text();
            if (newText.isEmpty()) {
                QMessageBox::warning(this, "Ошибка", "Задача не может быть пустой!");
                return;
            }
            item->text = newText;
            item->label->setText(newText);
            item->label->setVisible(true);
            item->edit->setVisible(false);
            item->editBtn->setVisible(true);
            item->saveBtn->setVisible(false);
            saveTasksToFile();
        });

        connect(removeBtn, &QPushButton::clicked, this, [this, item](){
            taskLayout->removeWidget(item->frame);
            item->frame->deleteLater();
            tasks.removeOne(item);
            delete item;
            saveTasksToFile();
        });
    }

    void filterTasksByTag(const QString &tag) {
        for (TaskItem *item : tasks) {
            bool show = (tag == "Все теги") || (item->tag == tag);
            item->frame->setVisible(show);
        }
    }

    void updateTagFilter() {
        QSet<QString> tagsSet;
        for (TaskItem *item : tasks) {
            if (!item->tag.isEmpty()) tagsSet.insert(item->tag);
        }
        QString current = tagFilterCombo->currentText();
        tagFilterCombo->clear();
        tagFilterCombo->addItem("Все теги");
        for (const QString &tag : tagsSet) {
            tagFilterCombo->addItem(tag);
        }
        int idx = tagFilterCombo->findText(current);
        if (idx != -1) tagFilterCombo->setCurrentIndex(idx);
    }

    void loadTasksFromFile() {
        QFile file(tasksFile);
        if (!file.open(QIODevice::ReadOnly)) return;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (!doc.isArray()) return;

        for (const QJsonValue &val : doc.array()) {
            QJsonObject obj = val.toObject();
            addTaskItem(
                obj["text"].toString(),
                obj["date"].toString(),
                obj["time"].toString(),
                obj["tag"].toString(),
                obj["completed"].toBool()
                );
        }
        updateTagFilter();
    }

    void saveTasksToFile() {
        QJsonArray array;
        for (TaskItem *item : tasks) {
            QJsonObject obj;
            obj["text"] = item->text;
            obj["date"] = item->date;
            obj["time"] = item->time;
            obj["tag"] = item->tag;
            obj["completed"] = item->completed;
            array.append(obj);
        }
        QFile file(tasksFile);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить задачи.");
            return;
        }
        file.write(QJsonDocument(array).toJson());
        file.close();
    }

public:
    QList<QString> getTasksForDate(const QString &date) {
        QList<QString> result;
        for (TaskItem *item : tasks) {
            if (item->date == date) result.append(item->text);
        }
        return result;
    }
};

#endif // TASKWIDGET_H
