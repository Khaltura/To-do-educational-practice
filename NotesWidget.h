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

        // Загружаем заметки если пользователь уже вошел
        if (m_dbManager->isLoggedIn()) {
            loadNotes();
        }
    }

    void loadNotes() {
        clearNotes();
        if (m_dbManager->isLoggedIn()) {
            auto notes = m_dbManager->getNotes();
            for (const auto& note : notes) {
                createNoteCard(note["id"].toInt(), note["content"].toString());
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

        // Сохраняем заметку в БД
        int noteId = m_dbManager->saveNote(html);
        if (noteId != -1) {
            createNoteCard(noteId, html);
            m_noteInput->clear();
            m_attachedImages.clear();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить заметку");
        }
    }

private:
    DatabaseManager* m_dbManager;
    QTextEdit* m_noteInput;
    QScrollArea* m_scrollArea;
    QVBoxLayout* m_noteLayout;
    QStringList m_attachedImages;

    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QLabel *title = new QLabel("📝 Заметки");
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 24px; font-weight: bold;");
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
        m_noteInput->setFixedHeight(80);
        mainLayout->addWidget(m_noteInput);

        QHBoxLayout *buttonLayout = new QHBoxLayout;
        QPushButton *addImageBtn = new QPushButton("📌 Изображение");
        QPushButton *addBulletBtn = new QPushButton("• Список");
        QPushButton *addNoteBtn = new QPushButton("➕ Добавить заметку");
        buttonLayout->addWidget(addImageBtn);
        buttonLayout->addWidget(addBulletBtn);
        buttonLayout->addWidget(addNoteBtn);
        mainLayout->addLayout(buttonLayout);

        m_scrollArea = new QScrollArea;
        m_scrollArea->setWidgetResizable(true);
        QWidget *container = new QWidget;
        m_noteLayout = new QVBoxLayout(container);
        m_noteLayout->setAlignment(Qt::AlignTop);
        container->setLayout(m_noteLayout);
        m_scrollArea->setWidget(container);
        mainLayout->addWidget(m_scrollArea);

        connect(addImageBtn, &QPushButton::clicked, this, &NotesWidget::attachImage);
        connect(addBulletBtn, &QPushButton::clicked, this, &NotesWidget::addBulletedList);
        connect(addNoteBtn, &QPushButton::clicked, this, &NotesWidget::addNote);
    }

    void createNoteCard(int noteId, const QString& html) {
        QFrame *noteFrame = new QFrame;
        noteFrame->setFrameShape(QFrame::Box);
        noteFrame->setStyleSheet("background-color: #2e2e2e; border-radius: 10px; padding: 8px;");
        QVBoxLayout *frameLayout = new QVBoxLayout(noteFrame);
        frameLayout->setSpacing(4);

        QTextEdit *noteContent = new QTextEdit;
        noteContent->setHtml(html);
        noteContent->setReadOnly(true);
        noteContent->setStyleSheet("background-color: #2e2e2e; color: white; border: none;");
        noteContent->setMinimumHeight(120);
        noteContent->setMaximumHeight(200);
        frameLayout->addWidget(noteContent);

        QHBoxLayout *actionLayout = new QHBoxLayout;
        QPushButton *editBtn = new QPushButton("✏️");
        QPushButton *saveBtn = new QPushButton("💾");
        QPushButton *openBtn = new QPushButton("🔎");
        QPushButton *deleteBtn = new QPushButton("❌");
        saveBtn->setEnabled(false);
        actionLayout->addWidget(editBtn);
        actionLayout->addWidget(saveBtn);
        actionLayout->addWidget(openBtn);
        actionLayout->addStretch();
        actionLayout->addWidget(deleteBtn);
        frameLayout->addLayout(actionLayout);

        m_noteLayout->addWidget(noteFrame);

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
            if (m_dbManager->deleteNote(noteId)) {
                m_noteLayout->removeWidget(noteFrame);
                noteFrame->deleteLater();
            }
        });

        connect(openBtn, &QPushButton::clicked, this, [=]() {
            QDialog *dialog = new QDialog(this);
            dialog->setWindowTitle("Просмотр заметки");
            dialog->resize(800, 600);

            QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
            QTextBrowser *browser = new QTextBrowser;
            browser->setHtml(noteContent->toHtml());
            browser->setStyleSheet("background-color: #1e1e1e; color: white;");
            dialogLayout->addWidget(browser);

            QPushButton *closeBtn = new QPushButton("Закрыть");
            connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
            dialogLayout->addWidget(closeBtn);

            dialog->exec();
        });
    }
};

#endif // NOTESWIDGET_H
