#ifndef NOTESWIDGET_H
#define NOTESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
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
#include <QTimer>
#include <QSqlError>  // Добавлен заголовочный файл для QSqlError

class DatabaseManager;

class NotesWidget : public QWidget {
    Q_OBJECT

public:
    explicit NotesWidget(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~NotesWidget();

    void loadNotes();
    void clearNotes();

private slots:
    void attachImage();
    void addBulletedList();
    void addNote();

private:
    DatabaseManager* m_dbManager;
    QTextEdit* m_noteInput;
    QScrollArea* m_scrollArea;
    QVBoxLayout* m_noteLayout;
    QStringList m_attachedImages;

    void setupUI();
    void connectSignals();
    QString getNoteAuthor(const QMap<QString, QVariant>& note) const;
    void createNoteCard(int noteId, const QString& content, const QString& author = "");
    void showNoteDialog(const QString& content);
};

#endif // NOTESWIDGET_H
