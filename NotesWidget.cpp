#include "NotesWidget.h"
#include "DatabaseManager.h"
#include <QDebug>

NotesWidget::NotesWidget(DatabaseManager* dbManager, QWidget *parent)
    : QWidget(parent),
    m_dbManager(dbManager),
    m_noteInput(new QTextEdit(this)),
    m_scrollArea(new QScrollArea(this)),
    m_noteLayout(new QVBoxLayout())
{
    setupUI();
    connectSignals();

    if (m_dbManager->isLoggedIn()) {
        QTimer::singleShot(0, this, &NotesWidget::loadNotes);
    }
}

NotesWidget::~NotesWidget() {
    clearNotes();
}

void NotesWidget::loadNotes() {
    clearNotes();
    if (!m_dbManager->isLoggedIn()) return;

    auto notes = m_dbManager->getNotes();
    for (const auto& note : notes) {
        QString author = getNoteAuthor(note);
        createNoteCard(note["id"].toInt(), note["content"].toString(), author);
    }
    updateGeometry();
}

void NotesWidget::clearNotes() {
    QLayoutItem* item;
    while ((item = m_noteLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void NotesWidget::attachImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Images (*.png *.jpg *.jpeg)");
    if (!fileName.isEmpty()) {
        m_attachedImages.append(fileName);
        m_noteInput->append(QString("<img src='%1' width='200'/>").arg(fileName));
    }
}

void NotesWidget::addBulletedList() {
    QTextCursor cursor = m_noteInput->textCursor();
    QTextListFormat listFormat;
    listFormat.setStyle(QTextListFormat::ListDisc);
    cursor.insertList(listFormat);
}

void NotesWidget::addNote() {
    if (!m_dbManager->isLoggedIn()) {
        QMessageBox::warning(this, "Ошибка", "Необходимо войти в систему");
        return;
    }

    QString html = m_noteInput->toHtml().trimmed();
    QTextDocument doc;
    doc.setHtml(html);
    if (html.isEmpty() || doc.toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нельзя добавить пустую заметку");
        return;
    }

    // Простой вызов saveNote с одним параметром
    if (!m_dbManager->saveNote(html)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить заметку");
        return;
    }

    m_noteInput->clear();
    m_attachedImages.clear();
    loadNotes();
}

void NotesWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Заголовок
    QLabel* title = new QLabel("📝 Заметки", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
    mainLayout->addWidget(title);

    // Поле ввода
    m_noteInput->setPlaceholderText("Введите текст заметки...");
    m_noteInput->setStyleSheet(
        "QTextEdit {"
        "   background: #f8f8f8;"
        "   border-radius: 5px;"
        "   padding: 8px;"
        "   border: 1px solid #ddd;"
        "   color: #333;"
        "}"
        );
    m_noteInput->setMinimumHeight(100);
    mainLayout->addWidget(m_noteInput);

    // Панель кнопок
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* imgBtn = new QPushButton("📌 Изображение", this);
    QPushButton* listBtn = new QPushButton("• Список", this);
    QPushButton* addBtn = new QPushButton("➕ Добавить", this);
    addBtn->setStyleSheet(
        "QPushButton {"
        "   background: #4CAF50;"
        "   color: white;"
        "   border-radius: 4px;"
        "   padding: 5px 10px;"
        "}"
        "QPushButton:hover { background: #45a049; }"
        );

    connect(imgBtn, &QPushButton::clicked, this, &NotesWidget::attachImage);
    connect(listBtn, &QPushButton::clicked, this, &NotesWidget::addBulletedList);
    connect(addBtn, &QPushButton::clicked, this, &NotesWidget::addNote);

    btnLayout->addWidget(imgBtn);
    btnLayout->addWidget(listBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(addBtn);
    mainLayout->addLayout(btnLayout);

    // Область с заметками
    QWidget* container = new QWidget();
    container->setLayout(m_noteLayout);

    m_scrollArea->setWidget(container);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet(
        "QScrollArea {"
        "   border: none;"
        "   background: #f0f0f0;"
        "}"
        );
    mainLayout->addWidget(m_scrollArea);
}

void NotesWidget::connectSignals() {
    connect(m_dbManager, &DatabaseManager::loggedIn, this, &NotesWidget::loadNotes);
    connect(m_dbManager, &DatabaseManager::loggedOut, this, &NotesWidget::clearNotes);
    connect(m_dbManager, &DatabaseManager::groupChanged, this, &NotesWidget::loadNotes);
    connect(m_dbManager, &DatabaseManager::notesUpdated, this, &NotesWidget::loadNotes);
}

QString NotesWidget::getNoteAuthor(const QMap<QString, QVariant>& note) const {
    if (m_dbManager->currentDbMode() == DatabaseManager::GroupDb) {
        return note.contains("username") ? note["username"].toString() :
                   note.contains("user_id") ? QString::number(note["user_id"].toInt()) : "";
    }
    return "";
}

void NotesWidget::createNoteCard(int noteId, const QString& content, const QString& author) {
    QFrame* frame = new QFrame();
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setStyleSheet(
        "QFrame {"
        "   background: #f5f5f5;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   border: 1px solid #e0e0e0;"
        "}"
        );

    QVBoxLayout* layout = new QVBoxLayout(frame);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 8, 8, 8);

    // Заголовок с автором
    if (!author.isEmpty()) {
        QLabel* authorLabel = new QLabel("👤 " + author, frame);
        authorLabel->setStyleSheet(
            "QLabel {"
            "   color: #666;"
            "   font-size: 12px;"
            "   font-style: italic;"
            "}"
            );
        layout->addWidget(authorLabel);
    }

    // Содержимое заметки
    QTextEdit* contentEdit = new QTextEdit(frame);
    contentEdit->setHtml(content);
    contentEdit->setReadOnly(true);
    contentEdit->setStyleSheet(
        "QTextEdit {"
        "   background: transparent;"
        "   border: none;"
        "   color: #333;"
        "   font-size: 14px;"
        "}"
        );
    contentEdit->setMinimumHeight(100);
    contentEdit->setMaximumHeight(300);
    layout->addWidget(contentEdit);

    // Кнопки действий
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(5);

    bool isEditable = author.isEmpty() || author == m_dbManager->currentUser();
    if (isEditable) {
        QPushButton* editBtn = new QPushButton("✏️", frame);
        QPushButton* deleteBtn = new QPushButton("❌", frame);

        editBtn->setStyleSheet(
            "QPushButton {"
            "   border: none;"
            "   background: transparent;"
            "   font-size: 14px;"
            "}"
            "QPushButton:hover { color: #4CAF50; }"
            );

        deleteBtn->setStyleSheet(
            "QPushButton {"
            "   border: none;"
            "   background: transparent;"
            "   font-size: 14px;"
            "}"
            "QPushButton:hover { color: #f44336; }"
            );

        connect(editBtn, &QPushButton::clicked, [this, noteId]() {
            // Реализация редактирования
        });

        connect(deleteBtn, &QPushButton::clicked, [this, noteId]() {
            if (QMessageBox::question(this, "Удаление", "Удалить заметку?") == QMessageBox::Yes) {
                if (m_dbManager->deleteNote(noteId)) loadNotes();
            }
        });

        actionLayout->addWidget(editBtn);
        actionLayout->addWidget(deleteBtn);
    }

    QPushButton* viewBtn = new QPushButton("🔍 Просмотр", frame);
    viewBtn->setStyleSheet(
        "QPushButton {"
        "   background: #e0e0e0;"
        "   color: #333;"
        "   border-radius: 4px;"
        "   padding: 3px 8px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover { background: #d0d0d0; }"
        );

    connect(viewBtn, &QPushButton::clicked, [this, content]() {
        showNoteDialog(content);
    });

    actionLayout->addStretch();
    actionLayout->addWidget(viewBtn);
    layout->addLayout(actionLayout);

    m_noteLayout->addWidget(frame);
}

void NotesWidget::showNoteDialog(const QString& content) {
    QDialog dialog(this);
    dialog.setWindowTitle("Просмотр заметки");
    dialog.resize(600, 400);

    QTextBrowser* browser = new QTextBrowser(&dialog);
    browser->setHtml(content);
    browser->setStyleSheet(
        "QTextBrowser {"
        "   background: #f5f5f5;"
        "   border: none;"
        "   font-size: 14px;"
        "}"
        );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(browser);

    QPushButton* closeBtn = new QPushButton("Закрыть", &dialog);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    layout->addLayout(btnLayout);
    dialog.exec();
}
