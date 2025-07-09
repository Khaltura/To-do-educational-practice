//NotesWidget.h
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

class NotesWidget : public QWidget {
    Q_OBJECT
public:
    NotesWidget(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Заголовок
        QLabel *title = new QLabel("📝 Заметки");
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 24px; font-weight: bold;");
        mainLayout->addWidget(title);

        // Поле ввода
        noteInput = new QTextEdit;
        noteInput->setPlaceholderText("Введите текст заметки...");
        noteInput->setStyleSheet(
            "background-color: #1e1e1e;"
            "color: white;"
            "font-size: 16px;"
            "border: 1px solid #555;"
            "border-radius: 8px;"
            "padding: 8px;"
            );
        noteInput->setFixedHeight(80);
        mainLayout->addWidget(noteInput);

        QHBoxLayout *buttonLayout = new QHBoxLayout;
        QPushButton *addImageBtn = new QPushButton("📌 Изображение");
        QPushButton *addBulletBtn = new QPushButton("• Список");
        QPushButton *addNoteBtn = new QPushButton("➕ Добавить заметку");
        buttonLayout->addWidget(addImageBtn);
        buttonLayout->addWidget(addBulletBtn);
        buttonLayout->addWidget(addNoteBtn);
        mainLayout->addLayout(buttonLayout);

        // Область карточек заметок
        scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);
        QWidget *container = new QWidget;
        noteLayout = new QVBoxLayout(container);
        noteLayout->setAlignment(Qt::AlignTop);
        container->setLayout(noteLayout);
        scrollArea->setWidget(container);
        mainLayout->addWidget(scrollArea);

        // Подключения
        connect(addImageBtn, &QPushButton::clicked, this, &NotesWidget::attachImage);
        connect(addBulletBtn, &QPushButton::clicked, this, &NotesWidget::addBulletedList);
        connect(addNoteBtn, &QPushButton::clicked, this, &NotesWidget::addNote);
    }

private:
    QTextEdit *noteInput;
    QScrollArea *scrollArea;
    QVBoxLayout *noteLayout;
    QStringList attachedImages;

    void attachImage() {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Images (*.png *.jpg *.jpeg)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.size() > 5 * 1024 * 1024) {
                QMessageBox::warning(this, "Ошибка", "Файл превышает 5 МБ");
                return;
            }
            attachedImages.append(fileName);
            noteInput->append("<img src='" + fileName + "' width='200' />");
        }
    }

    void addBulletedList() {
        QTextCursor cursor = noteInput->textCursor();
        QTextListFormat listFormat;
        listFormat.setStyle(QTextListFormat::ListDisc);
        cursor.insertList(listFormat);
    }

    void addNote() {
        QString html = noteInput->toHtml().trimmed();

        // Проверяем, что заметка не пустая (без текста)
        QTextDocument doc;
        doc.setHtml(html);
        QString plainText = doc.toPlainText().trimmed();

        if (plainText.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Нельзя добавить пустую заметку.");
            return;
        }

        // Создание карточки
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

        noteLayout->addWidget(noteFrame);

        connect(editBtn, &QPushButton::clicked, this, [=]() {
            noteContent->setReadOnly(false);
            saveBtn->setEnabled(true);
            noteContent->setFocus();
        });

        connect(saveBtn, &QPushButton::clicked, this, [=]() {
            noteContent->setReadOnly(true);
            saveBtn->setEnabled(false);
        });

        connect(deleteBtn, &QPushButton::clicked, this, [=]() {
            noteLayout->removeWidget(noteFrame);
            noteFrame->deleteLater();
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

        noteInput->clear();
        attachedImages.clear();
    }
};

#endif // NOTESWIDGET_H
