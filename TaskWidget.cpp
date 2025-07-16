
#include "TaskWidget.h"
#include "DatabaseManager.h"
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
#include <QSqlError>

TaskWidget::TaskWidget(DatabaseManager* dbManager, QWidget* parent)
    : QWidget(parent), m_dbManager(dbManager),
    m_mainLayout(new QVBoxLayout(this)),
    m_titleLabel(new QLabel("📋 Задачи")),
    m_tagFilterCombo(new QComboBox),
    m_taskInput(new QLineEdit),
    m_dateBtn(new QPushButton("📅")),
    m_timeBtn(new QPushButton("⏱")),
    m_tagBtn(new QPushButton("🏷")),
    m_addBtn(new QPushButton("➕")),
    m_scrollArea(new QScrollArea),
    m_containerWidget(new QWidget),
    m_taskLayout(new QVBoxLayout)
{
    // Настройка интерфейса
    setupUI();

    // Подключение сигналов
    connect(m_dbManager, &DatabaseManager::loggedIn, this, &TaskWidget::refreshTasks);
    connect(m_dbManager, &DatabaseManager::loggedOut, this, &TaskWidget::refreshTasks);
    connect(m_dbManager, &DatabaseManager::tasksUpdated, this, &TaskWidget::onTasksUpdated);

    connect(m_tagFilterCombo, &QComboBox::currentTextChanged, this, &TaskWidget::filterTasksByTag);
    connect(m_dateBtn, &QPushButton::clicked, this, &TaskWidget::openDatePopup);
    connect(m_timeBtn, &QPushButton::clicked, this, &TaskWidget::openTimePopup);
    connect(m_tagBtn, &QPushButton::clicked, this, &TaskWidget::openTagPopup);
    connect(m_addBtn, &QPushButton::clicked, this, &TaskWidget::addTask);

    // Первоначальная загрузка задач
    refreshTasks();
}

QSqlError DatabaseManager::lastError() const
{
    if (m_currentDb.isOpen()) {
        return m_currentDb.lastError();
    }
    return m_mainDb.lastError();
}

void TaskWidget::setupUI()
{
    // Основные настройки layout
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(15);

    // Настройка заголовка
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff;");
    m_mainLayout->addWidget(m_titleLabel);

    // Настройка комбобокса фильтра по тегам
    m_tagFilterCombo->addItem("Все теги");
    m_tagFilterCombo->setStyleSheet(
        "QComboBox { font-size: 14px; padding: 4px 8px; background-color: #2d2d2d; color: white; "
        "border: 1px solid #444; border-radius: 6px; }"
        "QComboBox:hover { border-color: #0078d7; }"
        "QComboBox::drop-down { border: none; }");
    m_mainLayout->addWidget(m_tagFilterCombo);

    // Layout для ввода задачи
    QHBoxLayout* inputLayout = new QHBoxLayout;
    inputLayout->setSpacing(8);

    // Поле ввода задачи
    m_taskInput->setPlaceholderText("Введите новую задачу...");
    m_taskInput->setStyleSheet(
        "QLineEdit { background-color: #2d2d2d; color: #ffffff; font-size: 16px; "
        "padding: 8px 12px; border: 2px solid #444; border-radius: 8px; }"
        "QLineEdit:focus { border-color: #0078d7; background-color: #333; }"
        "QLineEdit:hover { border-color: #555; }");
    inputLayout->addWidget(m_taskInput, 1);

    // Кнопки управления
    for (QPushButton* btn : {m_dateBtn, m_timeBtn, m_tagBtn, m_addBtn}) {
        btn->setFixedSize(40, 40);
        btn->setStyleSheet(
            "QPushButton { background-color: #2d2d2d; color: white; border: 1px solid #444; "
            "border-radius: 8px; font-size: 16px; }"
            "QPushButton:hover { background-color: #3d3d3d; border-color: #0078d7; }"
            "QPushButton:pressed { background-color: #1d1d1d; }");
        inputLayout->addWidget(btn);
    }

    m_mainLayout->addLayout(inputLayout);

    // Область с задачами
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_containerWidget->setLayout(m_taskLayout);
    m_taskLayout->setAlignment(Qt::AlignTop);
    m_taskLayout->setSpacing(10);
    m_taskLayout->setContentsMargins(2, 2, 10, 2);

    m_scrollArea->setWidget(m_containerWidget);
    m_mainLayout->addWidget(m_scrollArea);
}

void TaskWidget::refreshTasks()
{
    clearTasks();

    if (m_dbManager->isLoggedIn()) {
        auto tasks = m_dbManager->getTasks();
        for (const auto& task : tasks) {
            addTaskItem(task);
        }
        updateTagFilter();
    }
}

void TaskWidget::addTaskItem(const QMap<QString, QVariant>& taskData)
{
    int taskId = taskData["id"].toInt();
    QString text = taskData["text"].toString();
    QString date = taskData["date"].toString();
    QString time = taskData["time"].toString();
    QString tag = taskData["tag"].toString();
    bool completed = taskData["completed"].toBool();

    QString displayText = formatTaskText(text, date, time, tag);

    QFrame* taskFrame = new QFrame;
    taskFrame->setFrameShape(QFrame::StyledPanel);
    taskFrame->setStyleSheet("QFrame { background-color: #2d2d2d; border-radius: 8px; padding: 8px; }");

    QHBoxLayout* taskLayout = new QHBoxLayout(taskFrame);
    taskLayout->setContentsMargins(5, 5, 5, 5);
    taskLayout->setSpacing(10);

    QCheckBox* checkBox = new QCheckBox;
    checkBox->setChecked(completed);
    checkBox->setStyleSheet("QCheckBox { spacing: 8px; }");
    taskLayout->addWidget(checkBox);

    QLabel* taskLabel = new QLabel(displayText);
    taskLabel->setStyleSheet(
        completed ? "color: #888; font-size: 16px; text-decoration: line-through;"
                  : "color: white; font-size: 16px;");
    taskLabel->setWordWrap(true);
    taskLayout->addWidget(taskLabel, 1);

    QLineEdit* taskEdit = new QLineEdit(displayText);
    taskEdit->setVisible(false);
    taskEdit->setStyleSheet(
        "QLineEdit { background: #333; color: white; border: 1px solid #555; "
        "border-radius: 4px; padding: 4px; }");
    taskLayout->addWidget(taskEdit, 1);

    QPushButton* editBtn = new QPushButton("✏️");
    QPushButton* saveBtn = new QPushButton("💾");
    QPushButton* removeBtn = new QPushButton("❌");

    for (QPushButton* btn : {editBtn, saveBtn, removeBtn}) {
        btn->setFixedSize(32, 32);
        btn->setStyleSheet(
            "QPushButton { background: transparent; border: none; font-size: 16px; }"
            "QPushButton:hover { color: #0078d7; }");
    }

    saveBtn->setVisible(false);
    saveBtn->setEnabled(false);

    taskLayout->addWidget(editBtn);
    taskLayout->addWidget(saveBtn);
    taskLayout->addWidget(removeBtn);

    m_taskLayout->addWidget(taskFrame);

    TaskItem* item = new TaskItem{
        taskId,
        taskFrame,
        checkBox,
        taskLabel,
        taskEdit,
        editBtn,
        saveBtn,
        removeBtn,
        tag
    };
    m_tasks.append(item);

    connect(checkBox, &QCheckBox::stateChanged, this, [this, item](int state) {
        QMap<QString, QVariant> updates;
        updates["completed"] = (state == Qt::Checked);
        m_dbManager->updateTask(item->id, updates);
    });

    connect(editBtn, &QPushButton::clicked, this, [item]() {
        item->label->setVisible(false);
        item->edit->setText(item->label->text());
        item->edit->setVisible(true);
        item->editBtn->setVisible(false);
        item->saveBtn->setVisible(true);
        item->saveBtn->setEnabled(true);
    });

    connect(saveBtn, &QPushButton::clicked, this, [this, item]() {
        QString newText = item->edit->text();
        if (newText.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Задача не может быть пустой!");
            return;
        }

        QMap<QString, QVariant> updates;
        updates["text"] = newText;
        m_dbManager->updateTask(item->id, updates);

        item->label->setText(newText);
        item->label->setVisible(true);
        item->edit->setVisible(false);
        item->editBtn->setVisible(true);
        item->saveBtn->setVisible(false);
    });

    connect(removeBtn, &QPushButton::clicked, this, [this, item]() {
        if (m_dbManager->removeTask(item->id)) {
            m_taskLayout->removeWidget(item->frame);
            item->frame->deleteLater();
            m_tasks.removeOne(item);
            delete item;
        }
    });
}

void TaskWidget::clearTasks()
{
    for (TaskItem* item : m_tasks) {
        m_taskLayout->removeWidget(item->frame);
        item->frame->deleteLater();
        delete item;
    }
    m_tasks.clear();
}

void TaskWidget::updateTagFilter()
{
    QString currentTag = m_tagFilterCombo->currentText();
    m_tagFilterCombo->clear();
    m_tagFilterCombo->addItem("Все теги");

    if (!m_dbManager->isLoggedIn()) return;

    auto tags = m_dbManager->getAvailableTags();
    for (const QString& tag : tags) {
        m_tagFilterCombo->addItem(tag);
    }

    int idx = m_tagFilterCombo->findText(currentTag);
    m_tagFilterCombo->setCurrentIndex(idx != -1 ? idx : 0);
}

void TaskWidget::openDatePopup()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Выберите дату");
    dialog.setStyleSheet("background-color: #252526; color: white;");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QCalendarWidget* calendar = new QCalendarWidget;
    calendar->setStyleSheet(
        "QCalendarWidget { background: #252526; color: white; }"
        "QToolButton { background: #333; color: white; }");
    layout->addWidget(calendar);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->setStyleSheet("QPushButton { color: white; background: #333; padding: 5px 10px; }");
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, [&]() {
        m_selectedDate = calendar->selectedDate();
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}

void TaskWidget::openTimePopup()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Выберите время");
    dialog.setStyleSheet("background-color: #252526; color: white;");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTimeEdit* timeEdit = new QTimeEdit(QTime::currentTime());
    timeEdit->setDisplayFormat("HH:mm");
    timeEdit->setStyleSheet(
        "QTimeEdit { color: white; background: #333; padding: 5px; }");
    layout->addWidget(timeEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->setStyleSheet("QPushButton { color: white; background: #333; padding: 5px 10px; }");
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, [&]() {
        m_selectedTime = timeEdit->time();
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}

void TaskWidget::openTagPopup()
{
    bool ok;
    QString tag = QInputDialog::getText(this, "Добавить тег", "Тег:",
                                        QLineEdit::Normal, "", &ok,
                                        Qt::MSWindowsFixedSizeDialogHint);
    if (ok && !tag.isEmpty()) {
        m_selectedTag = tag;
    }
}

void TaskWidget::addTask()
{
    QString text = m_taskInput->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Задача не может быть пустой!");
        return;
    }

    if (!m_dbManager->isLoggedIn()) {
        QMessageBox::warning(this, "Ошибка", "Необходимо войти в систему!");
        return;
    }

    if (m_dbManager->saveTask(text, m_selectedDate, m_selectedTime, m_selectedTag, false)) {
        m_taskInput->clear();
        m_selectedDate = QDate();
        m_selectedTime = QTime();
        m_selectedTag.clear();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить задачу: " + m_dbManager->lastError().text());
    }
}

void TaskWidget::filterTasksByTag(const QString& tag)
{
    for (TaskItem* item : m_tasks) {
        bool show = (tag == "Все теги") || (item->tag == tag);
        item->frame->setVisible(show);
    }
}

void TaskWidget::setSelectedDate(const QDate& date)
{
    m_selectedDate = date;

    if (date.isValid()) {
        auto tasks = m_dbManager->getTasksForDate(date);
        emit tasksUpdatedForDate(date, tasks);
    }
}

QString TaskWidget::formatTaskText(const QString& text, const QString& date,
                                   const QString& time, const QString& tag) const
{
    QString result = text;
    if (!date.isEmpty()) result += "  📅 " + date;
    if (!time.isEmpty()) result += "  ⏱ " + time;
    if (!tag.isEmpty()) result += "  🏷 " + tag;
    return result;
}

void TaskWidget::onTasksUpdated()
{
    refreshTasks();

    if (m_selectedDate.isValid()) {
        auto tasks = m_dbManager->getTasksForDate(m_selectedDate);
        emit tasksUpdatedForDate(m_selectedDate, tasks);
    }
}

TaskWidget::~TaskWidget()
{
    clearTasks();
}
