#include "TaskWidget.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QCalendarWidget>
#include <QTimeEdit>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QTimer>
#include <QApplication>
#include "DatabaseManager.h"


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
    setupUI();
    setupConnections();
    refreshTasks();
}

TaskWidget::~TaskWidget()
{
    clearTasks();
}

void TaskWidget::setupUI()
{
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(15);

    // Заголовок
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff;");
    m_mainLayout->addWidget(m_titleLabel);

    // Фильтр по тегам
    m_tagFilterCombo->addItem("Все теги");
    m_tagFilterCombo->setStyleSheet("QComboBox { font-size: 14px; padding: 4px 8px; background-color: #2d2d2d; color: white; border: 1px solid #444; border-radius: 6px; }");
    m_mainLayout->addWidget(m_tagFilterCombo);

    // Поле ввода
    QHBoxLayout* inputLayout = new QHBoxLayout;
    m_taskInput->setPlaceholderText("Введите новую задачу...");
    m_taskInput->setStyleSheet("QLineEdit { background-color: #2d2d2d; color: #ffffff; font-size: 16px; padding: 8px 12px; border: 2px solid #444; border-radius: 8px; }");
    inputLayout->addWidget(m_taskInput, 1);

    // Кнопки
    for (QPushButton* btn : {m_dateBtn, m_timeBtn, m_tagBtn, m_addBtn}) {
        btn->setFixedSize(40, 40);
        btn->setStyleSheet("QPushButton { background-color: #2d2d2d; color: white; border: 1px solid #444; border-radius: 8px; font-size: 16px; }");
        inputLayout->addWidget(btn);
    }
    m_mainLayout->addLayout(inputLayout);

    // Список задач
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    m_containerWidget->setLayout(m_taskLayout);
    m_taskLayout->setAlignment(Qt::AlignTop);
    m_taskLayout->setSpacing(10);
    m_scrollArea->setWidget(m_containerWidget);
    m_mainLayout->addWidget(m_scrollArea);
}

void TaskWidget::setupConnections()
{
    connect(m_dbManager, &DatabaseManager::loggedIn, this, &TaskWidget::refreshTasks);
    connect(m_dbManager, &DatabaseManager::loggedOut, this, &TaskWidget::refreshTasks);
    connect(m_dbManager, &DatabaseManager::tasksUpdated, this, &TaskWidget::onTasksUpdated);

    // Добавлено: обновление задач при смене группы
    connect(m_dbManager, &DatabaseManager::groupChanged, this, &TaskWidget::refreshTasks);

    connect(m_tagFilterCombo, &QComboBox::currentTextChanged, this, &TaskWidget::filterTasksByTag);
    connect(m_dateBtn, &QPushButton::clicked, this, &TaskWidget::openDatePopup);
    connect(m_timeBtn, &QPushButton::clicked, this, &TaskWidget::openTimePopup);
    connect(m_tagBtn, &QPushButton::clicked, this, &TaskWidget::openTagPopup);
    connect(m_addBtn, &QPushButton::clicked, this, &TaskWidget::addTask);
}


void TaskWidget::refreshTasks()
{
    QString currentFilter = m_tagFilterCombo->currentText();
    clearTasks();

    if (m_dbManager->isLoggedIn()) {
        auto tasks = m_dbManager->getTasks();

        // Сначала невыполненные задачи
        for (const auto& task : tasks) {
            if (!task["completed"].toBool()) {
                addTaskItem(task);
            }
        }

        // Затем выполненные
        for (const auto& task : tasks) {
            if (task["completed"].toBool()) {
                addTaskItem(task);
            }
        }

        updateTagFilter();

        // Восстановление фильтра
        int idx = m_tagFilterCombo->findText(currentFilter);
        if (idx >= 0) m_tagFilterCombo->setCurrentIndex(idx);
    }
}

void TaskWidget::addTaskItem(const QMap<QString, QVariant>& taskData)
{
    if (!taskData.contains("id") || !taskData.contains("text")) {
        qWarning() << "Invalid task data";
        return;
    }

    TaskItem* item = new TaskItem;
    try {
        // Инициализация данных задачи
        item->id = taskData["id"].toInt();
        QString text = taskData["text"].toString();
        QString date = taskData["date"].toString();
        QString time = taskData["time"].toString();
        QString tag = taskData["tag"].toString();
        bool completed = taskData["completed"].toBool();
        QString displayText = formatTaskText(text, date, time, tag);

        if (m_dbManager->currentDbMode() == DatabaseManager::GroupDb) {
            QString author = taskData.value("username", "Неизвестно").toString();
            displayText += "\n👤 " + author;
        }

        // Создание виджетов
        item->frame = new QFrame;
        item->frame->setFrameShape(QFrame::StyledPanel);
        item->frame->setStyleSheet(QString(
                                       "QFrame { background-color: #2d2d2d; border-radius: 8px; padding: 8px; "
                                       "border-left: 4px solid %1; }"
                                       ).arg(completed ? "#555555" : "#4CAF50"));

        QHBoxLayout* taskLayout = new QHBoxLayout(item->frame);
        taskLayout->setContentsMargins(5, 5, 5, 5);

        item->checkBox = new QCheckBox;
        item->checkBox->setChecked(completed);
        taskLayout->addWidget(item->checkBox);

        item->label = new QLabel(displayText);
        item->label->setStyleSheet(completed ? "color: #888; text-decoration: line-through;" : "color: white;");
        item->label->setWordWrap(true);
        taskLayout->addWidget(item->label, 1);

        item->edit = new QLineEdit(text);
        item->edit->setVisible(false);
        taskLayout->addWidget(item->edit, 1);

        item->editBtn = new QPushButton("✏️");
        item->saveBtn = new QPushButton("💾");
        item->removeBtn = new QPushButton("❌");

        for (QPushButton* btn : {item->editBtn, item->saveBtn, item->removeBtn}) {
            btn->setFixedSize(32, 32);
            btn->setStyleSheet("QPushButton { background: transparent; border: none; }");
        }
        item->saveBtn->setVisible(false);

        taskLayout->addWidget(item->editBtn);
        taskLayout->addWidget(item->saveBtn);
        taskLayout->addWidget(item->removeBtn);

        item->tag = tag;

        // Добавление в layout
        m_taskLayout->addWidget(item->frame);
        m_tasks.append(item);

        // Подключение сигналов
        setupTaskItemConnections(item);
    } catch (...) {
        delete item;
        throw;
    }
}

void TaskWidget::setupTaskItemConnections(TaskItem* item)
{
    if (!item || item->isBeingDeleted) return;

    // Проверка всех указателей
    Q_ASSERT(item->frame);
    Q_ASSERT(item->checkBox);
    Q_ASSERT(item->label);
    Q_ASSERT(item->edit);
    Q_ASSERT(item->editBtn);
    Q_ASSERT(item->saveBtn);
    Q_ASSERT(item->removeBtn);

    // 1. Обработка изменения состояния чекбокса
    item->connections << connect(item->checkBox, &QCheckBox::stateChanged,
                                 this, [this, item](int state) {
                                     if (!item || item->isBeingDeleted) return;

                                     TaskUpdates updates;
                                     updates["completed"] = (state == Qt::Checked);

                                     // Блокируем интерфейс на время операции
                                     QApplication::setOverrideCursor(Qt::WaitCursor);
                                     item->checkBox->setEnabled(false);

                                     // Используем QFuture для асинхронной операции
                                     QFuture<bool> future = QtConcurrent::run([this, item, updates]() {
                                         return m_dbManager->updateTask(item->id, updates);
                                     });

                                     QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>();
                                     watcher->setFuture(future);

                                     connect(watcher, &QFutureWatcher<bool>::finished, this,
                                             [this, item, state, watcher]() {
                                                 bool success = watcher->result();
                                                 watcher->deleteLater();

                                                 QApplication::restoreOverrideCursor();
                                                 item->checkBox->setEnabled(true);

                                                 if (success) {
                                                     QString style = QString(
                                                                         "QFrame { background-color: #2d2d2d; border-radius: 8px; padding: 8px; "
                                                                         "border-left: 4px solid %1; }").arg(state == Qt::Checked ? "#555555" : "#4CAF50");

                                                     item->frame->setStyleSheet(style);
                                                     item->label->setStyleSheet(state == Qt::Checked
                                                                                    ? "color: #888; text-decoration: line-through;"
                                                                                    : "color: white;");
                                                 } else {
                                                     QSignalBlocker blocker(item->checkBox);
                                                     item->checkBox->setChecked(!item->checkBox->isChecked());
                                                     QMessageBox::warning(this, "Ошибка",
                                                                          m_dbManager->isGroupMode()
                                                                              ? "Не удалось обновить статус задачи. Возможно, она была изменена другим пользователем."
                                                                              : "Не удалось обновить статус задачи");
                                                 }
                                             });
                                 });

    // 2. Лямбда для обновления видимости кнопок
    auto updateButtonsVisibility = [this, item]() {
        bool isEditable = !m_dbManager->isGroupMode() ||
                          (item->label->text().contains("\n👤 " + m_dbManager->currentUser()));

        item->editBtn->setVisible(isEditable);
        item->removeBtn->setVisible(isEditable);
    };
    updateButtonsVisibility();

    // 3. Кнопка редактирования
    item->connections << connect(item->editBtn, &QPushButton::clicked,
                                 this, [item]() {
                                     if (!item || item->isBeingDeleted) return;

                                     item->label->setVisible(false);
                                     item->edit->setVisible(true);
                                     item->edit->setFocus();

                                     QString fullText = item->label->text();
                                     QString mainText = fullText.split("\n👤")[0].split("  📅")[0].trimmed();
                                     item->edit->setText(mainText);

                                     item->editBtn->setVisible(false);
                                     item->saveBtn->setVisible(true);
                                 });

    // 4. Кнопка сохранения
    item->connections << connect(item->saveBtn, &QPushButton::clicked,
                                 this, [this, item, updateButtonsVisibility]() {
                                     if (!item || item->isBeingDeleted) return;

                                     QString newText = item->edit->text().trimmed();
                                     if (newText.isEmpty()) {
                                         QMessageBox::warning(this, "Ошибка", "Текст задачи не может быть пустым!");
                                         return;
                                     }

                                     // Блокируем интерфейс на время операции
                                     QApplication::setOverrideCursor(Qt::WaitCursor);
                                     item->saveBtn->setEnabled(false);

                                     // Извлекаем существующие метаданные
                                     QString labelText = item->label->text();
                                     QRegularExpression dateRegex("📅 (\\d{4}-\\d{2}-\\d{2})");
                                     QRegularExpression timeRegex("⏱ (\\d{2}:\\d{2})");
                                     QRegularExpression tagRegex("🏷 (.+?)(?:  |\n|$)");

                                     TaskUpdates updates;
                                     updates["text"] = newText;

                                     auto dateMatch = dateRegex.match(labelText);
                                     if (dateMatch.hasMatch()) updates["date"] = dateMatch.captured(1);

                                     auto timeMatch = timeRegex.match(labelText);
                                     if (timeMatch.hasMatch()) updates["time"] = timeMatch.captured(1);

                                     auto tagMatch = tagRegex.match(labelText);
                                     if (tagMatch.hasMatch()) updates["tag"] = tagMatch.captured(1);

                                     // Асинхронное обновление
                                     QFuture<bool> future = QtConcurrent::run([this, item, updates]() {
                                         return m_dbManager->updateTask(item->id, updates);
                                     });

                                     QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>();
                                     watcher->setFuture(future);

                                     connect(watcher, &QFutureWatcher<bool>::finished, this,
                                             [this, item, updates, watcher, updateButtonsVisibility]() {
                                                 bool success = watcher->result();
                                                 watcher->deleteLater();

                                                 QApplication::restoreOverrideCursor();
                                                 item->saveBtn->setEnabled(true);

                                                 if (success) {
                                                     QString newLabel = formatTaskText(
                                                         updates["text"].toString(),
                                                         updates.value("date").toString(),
                                                         updates.value("time").toString(),
                                                         updates.value("tag").toString());

                                                     // Сохраняем информацию об авторе в групповом режиме
                                                     if (m_dbManager->isGroupMode()) {
                                                         QString author = item->label->text().split("\n👤 ").last();
                                                         newLabel += "\n👤 " + author;
                                                     }

                                                     item->label->setText(newLabel);
                                                     item->label->setVisible(true);
                                                     item->edit->setVisible(false);
                                                     item->editBtn->setVisible(true);
                                                     item->saveBtn->setVisible(false);

                                                     updateButtonsVisibility();
                                                 } else {
                                                     QMessageBox::warning(this, "Ошибка",
                                                                          m_dbManager->isGroupMode()
                                                                              ? "Не удалось сохранить изменения. Возможно, задача была изменена другим пользователем."
                                                                              : "Не удалось сохранить изменения");
                                                 }
                                             });
                                 });

    // 5. Кнопка удаления
    item->connections << connect(item->removeBtn, &QPushButton::clicked,
                                 this, [this, item, updateButtonsVisibility]() {
                                     if (!item || item->isBeingDeleted) return;

                                     // Дополнительная проверка для группового режима
                                     if (m_dbManager->isGroupMode()) {
                                         QString currentAuthor = item->label->text().split("\n👤 ").last();
                                         if (currentAuthor != m_dbManager->currentUser()) {
                                             QMessageBox::warning(this, "Ошибка прав",
                                                                  "Вы можете удалять только свои задачи в группе");
                                             return;
                                         }
                                     }

                                     auto reply = QMessageBox::question(this, "Подтверждение удаления",
                                                                        m_dbManager->isGroupMode()
                                                                            ? "Вы уверены, что хотите удалить эту задачу из группы?\n"
                                                                              "Это действие нельзя отменить."
                                                                            : "Вы уверены, что хотите удалить эту задачу?",
                                                                        QMessageBox::Yes | QMessageBox::No);

                                     if (reply != QMessageBox::Yes) return;

                                     // Блокируем интерфейс на время операции
                                     QApplication::setOverrideCursor(Qt::WaitCursor);
                                     item->removeBtn->setEnabled(false);
                                     item->frame->setEnabled(false);

                                     // Асинхронное удаление
                                     QFuture<bool> future = QtConcurrent::run([this, item]() {
                                         return m_dbManager->removeTask(item->id);
                                     });

                                     QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>();
                                     watcher->setFuture(future);

                                     connect(watcher, &QFutureWatcher<bool>::finished, this,
                                             [this, item, watcher, updateButtonsVisibility]() {
                                                 bool success = watcher->result();
                                                 watcher->deleteLater();

                                                 QApplication::restoreOverrideCursor();

                                                 if (success) {
                                                     // Отложенное безопасное удаление
                                                     QTimer::singleShot(0, this, [this, item]() {
                                                         if (m_tasks.contains(item)) {
                                                             m_taskLayout->removeWidget(item->frame);
                                                             m_tasks.removeOne(item);
                                                             item->frame->deleteLater();
                                                             delete item;
                                                         }
                                                     });
                                                 } else {
                                                     item->removeBtn->setEnabled(true);
                                                     item->frame->setEnabled(true);
                                                     QMessageBox::warning(this, "Ошибка",
                                                                          m_dbManager->isGroupMode()
                                                                              ? "Не удалось удалить задачу. Возможно, она уже была удалена другим пользователем."
                                                                              : "Не удалось удалить задачу");
                                                 }
                                             });
                                 });

    // 6. Обработка нажатия Enter в поле редактирования
    item->connections << connect(item->edit, &QLineEdit::returnPressed,
                                 this, [this, item]() {
                                     if (item->saveBtn->isVisible()) {
                                         item->saveBtn->click();
                                     }
                                 });

    // 7. Отмена редактирования при потере фокуса
    item->connections << connect(item->edit, &QLineEdit::editingFinished,
                                 this, [item]() {
                                     if (!item || !item->saveBtn->isVisible()) return;

                                     item->label->setVisible(true);
                                     item->edit->setVisible(false);
                                     item->editBtn->setVisible(true);
                                     item->saveBtn->setVisible(false);
                                 });
}

void TaskWidget::clearTasks()
{
    for (TaskItem* item : m_tasks) {
        if (!item) continue;

        item->isBeingDeleted = true;

        // Отключаем все сигналы
        for (const auto& connection : item->connections) {
            disconnect(connection);
        }
        item->connections.clear();

        // Планируем удаление
        if (item->frame) {
            item->frame->hide();
            item->frame->deleteLater();
        }
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

void TaskWidget::filterTasksByTag(const QString& tag)
{
    for (TaskItem* item : m_tasks) {
        bool show = (tag == "Все теги") || (item->tag == tag);
        item->frame->setVisible(show);
    }
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
        QMessageBox::critical(this, "Ошибка",
                              QString("Не удалось сохранить задачу:\n%1")
                                  .arg(m_dbManager->lastError().text()));
    }
}

void TaskWidget::onTasksUpdated()
{
    refreshTasks();
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
