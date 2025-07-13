#ifndef NOTESWIDGET_H
#define NOTESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QFrame>
#include <QDialog>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextListFormat>
#include <QTextDocument>
#include "DatabaseManager.h"

class NotesWidget : public QWidget {
    Q_OBJECT
public:
    explicit NotesWidget(DatabaseManager* dbManager, QWidget *parent = nullptr)
        : QWidget(parent), m_dbManager(dbManager) {
        setupUI();
        connect(m_dbManager, &DatabaseManager::loggedIn, this, &NotesWidget::loadNotes);
        connect(m_dbManager, &DatabaseManager::loggedOut, this, &NotesWidget::clearNotes);
        connect(m_dbManager, &DatabaseManager::groupChanged, this, &NotesWidget::loadNotes);
        connect(m_dbManager, &DatabaseManager::notesUpdated, this, &NotesWidget::loadNotes);

        if (m_dbManager->isLoggedIn()) {
            loadNotes();
        }
    }

    void loadNotes() {
        clearNotes();
        if (m_dbManager->isLoggedIn()) {
            auto notes = m_dbManager->getNotes();
            for (const auto& note : notes) {
                QString author = "";
                if (m_dbManager->currentDbMode() == DatabaseManager::GroupDb) {
                    author = note.contains("username") ? note["username"].toString() :
                                 note.contains("user_id") ? QString::number(note["user_id"].toInt()) : "";
                }
                createNoteCard(note["id"].toInt(), note["content"].toString(), author);
            }
        }
    }

    void clearNotes() {
        QLayoutItem *item;
        while ((item = m_noteLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }

private slots:
    void attachImage() {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Images (*.png *.jpg *.jpeg)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.size() > 5 * 1024 * 1024) {
                QMessageBox::warning(this, "Ошибка", "Файл превышает 5 МБ");
                return;
            }
            m_attachedImages.append(fileName);
            m_noteInput->append("<img src='" + fileName + "' width='200' />");
        }
    }

    void addBulletedList() {
        QTextCursor cursor = m_noteInput->textCursor();
        QTextListFormat listFormat;
        listFormat.setStyle(QTextListFormat::ListDisc);
        cursor.insertList(listFormat);
    }

    void addNote() {
        if (!m_dbManager->isLoggedIn()) {
            QMessageBox::warning(this, "Ошибка", "Необходимо войти в систему");
            return;
        }

        QString html = m_noteInput->toHtml().trimmed();
        QTextDocument doc;
        doc.setHtml(html);
        QString plainText = doc.toPlainText().trimmed();

        if (plainText.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Нельзя добавить пустую заметку.");
            return;
        }

        if (!m_dbManager->saveNote(html)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить заметку");
            return;
        }

        m_noteInput->clear();
        m_attachedImages.clear();
    }

private:
    DatabaseManager* m_dbManager;
    QTextEdit* m_noteInput;
    QScrollArea* m_scrollArea;
    QVBoxLayout* m_noteLayout;
    QStringList m_attachedImages;

    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(5, 5, 5, 5);
        mainLayout->setSpacing(10);

        QLabel *title = new QLabel("📝 Заметки");
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 10px;");
        mainLayout->addWidget(title);

        m_noteInput = new QTextEdit;
        m_noteInput->setPlaceholderText("Введите текст заметки...");
        m_noteInput->setStyleSheet(
            "background-color: #1e1e1e;"
            "color: white;"
            "font-size: 16px;"
            "border: 1px solid #555;"
            "border-radius: 8px;"
            "padding: 8px;"
            );
        m_noteInput->setFixedHeight(120);
        mainLayout->addWidget(m_noteInput);

        QHBoxLayout *buttonLayout = new QHBoxLayout;
        buttonLayout->setSpacing(10);

        QPushButton *addImageBtn = new QPushButton("📌 Изображение");
        QPushButton *addBulletBtn = new QPushButton("• Список");
        QPushButton *addNoteBtn = new QPushButton("➕ Добавить заметку");

        addImageBtn->setStyleSheet("padding: 5px;");
        addBulletBtn->setStyleSheet("padding: 5px;");
        addNoteBtn->setStyleSheet("padding: 5px; background-color: #4CAF50; color: white;");

        buttonLayout->addWidget(addImageBtn);
        buttonLayout->addWidget(addBulletBtn);
        buttonLayout->addWidget(addNoteBtn);
        mainLayout->addLayout(buttonLayout);

        m_scrollArea = new QScrollArea;
        m_scrollArea->setWidgetResizable(true);
        m_scrollArea->setStyleSheet("border: none;");

        QWidget *container = new QWidget;
        m_noteLayout = new QVBoxLayout(container);
        m_noteLayout->setAlignment(Qt::AlignTop);
        m_noteLayout->setSpacing(15);
        m_noteLayout->setContentsMargins(5, 5, 5, 5);

        container->setLayout(m_noteLayout);
        m_scrollArea->setWidget(container);
        mainLayout->addWidget(m_scrollArea);

        connect(addImageBtn, &QPushButton::clicked, this, &NotesWidget::attachImage);
        connect(addBulletBtn, &QPushButton::clicked, this, &NotesWidget::addBulletedList);
        connect(addNoteBtn, &QPushButton::clicked, this, &NotesWidget::addNote);
    }

    void createNoteCard(int noteId, const QString& html, const QString& author = "") {
        QFrame *noteFrame = new QFrame;
        noteFrame->setFrameShape(QFrame::StyledPanel);
        noteFrame->setStyleSheet(
            "background-color: #2e2e2e;"
            "border-radius: 10px;"
            "padding: 12px;"
            "border: 1px solid #444;"
            );

        QVBoxLayout *frameLayout = new QVBoxLayout(noteFrame);
        frameLayout->setSpacing(8);
        frameLayout->setContentsMargins(5, 5, 5, 5);

        // Заголовок заметки (с автором для группового режима)
        QHBoxLayout *headerLayout = new QHBoxLayout;

        if (!author.isEmpty()) {
            QLabel *authorLabel = new QLabel("👤 " + author);
            authorLabel->setStyleSheet(
                "color: #aaa;"
                "font-size: 12px;"
                "font-style: italic;"
                );
            headerLayout->addWidget(authorLabel);
        }

        headerLayout->addStretch();
        frameLayout->addLayout(headerLayout);

        // Содержимое заметки
        QTextEdit *noteContent = new QTextEdit;
        noteContent->setHtml(html);
        noteContent->setReadOnly(true);
        noteContent->setStyleSheet(
            "background-color: #2e2e2e;"
            "color: white;"
            "border: none;"
            "font-size: 14px;"
            );
        noteContent->setMinimumHeight(100);
        noteContent->setMaximumHeight(300);
        frameLayout->addWidget(noteContent);

        // Кнопки управления (только для своих заметок)
        bool isMyNote = author.isEmpty() || (author == m_dbManager->currentUser());
        if (isMyNote) {
            QHBoxLayout *actionLayout = new QHBoxLayout;
            actionLayout->setSpacing(5);

            QPushButton *editBtn = new QPushButton("✏️ Редактировать");
            QPushButton *saveBtn = new QPushButton("💾 Сохранить");
            QPushButton *deleteBtn = new QPushButton("❌ Удалить");

            editBtn->setStyleSheet("padding: 3px; font-size: 12px;");
            saveBtn->setStyleSheet("padding: 3px; font-size: 12px; background-color: #4CAF50; color: white;");
            deleteBtn->setStyleSheet("padding: 3px; font-size: 12px; background-color: #f44336; color: white;");

            saveBtn->setEnabled(false);

            actionLayout->addWidget(editBtn);
            actionLayout->addWidget(saveBtn);
            actionLayout->addStretch();
            actionLayout->addWidget(deleteBtn);
            frameLayout->addLayout(actionLayout);

            // Обработчики кнопок
            connect(editBtn, &QPushButton::clicked, this, [=]() {
                noteContent->setReadOnly(false);
                saveBtn->setEnabled(true);
                noteContent->setFocus();
            });

            connect(saveBtn, &QPushButton::clicked, this, [=]() {
                noteContent->setReadOnly(true);
                saveBtn->setEnabled(false);
                m_dbManager->updateNote(noteId, noteContent->toHtml());
            });

            connect(deleteBtn, &QPushButton::clicked, this, [=]() {
                if (QMessageBox::question(this, "Подтверждение",
                                          "Удалить эту заметку?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                    if (m_dbManager->deleteNote(noteId)) {
                        m_noteLayout->removeWidget(noteFrame);
                        noteFrame->deleteLater();
                    }
                }
            });
        }

        // Кнопка просмотра (для всех заметок)
        QPushButton *viewBtn = new QPushButton("🔍 Просмотреть");
        viewBtn->setStyleSheet("padding: 3px; font-size: 12px;");
        frameLayout->addWidget(viewBtn, 0, Qt::AlignRight);

        connect(viewBtn, &QPushButton::clicked, this, [=]() {
            QDialog *viewDialog = new QDialog(this);
            viewDialog->setWindowTitle("Просмотр заметки");
            viewDialog->resize(800, 600);

            QVBoxLayout *dialogLayout = new QVBoxLayout(viewDialog);

            QTextBrowser *browser = new QTextBrowser;
            browser->setHtml(noteContent->toHtml());
            browser->setStyleSheet(
                "background-color: #1e1e1e;"
                "color: white;"
                "border: none;"
                "font-size: 16px;"
                );
            dialogLayout->addWidget(browser);

            QPushButton *closeBtn = new QPushButton("Закрыть");
            closeBtn->setStyleSheet("padding: 5px;");
            dialogLayout->addWidget(closeBtn, 0, Qt::AlignRight);

            connect(closeBtn, &QPushButton::clicked, viewDialog, &QDialog::accept);

            viewDialog->exec();
        });

        m_noteLayout->addWidget(noteFrame);
    }
};

#endif // NOTESWIDGET_H
