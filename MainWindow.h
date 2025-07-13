#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QWidget>

// Forward declarations
class TaskWidget;
class CalendarWidget;
class NotesWidget;
class UserWidget;
class RegistrationWindow;
class LoginWindow;
class DatabaseManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showRegistrationWindow();
    void showLoginWindow();
    void handleUserLoggedIn();
    void handleUserLoggedOut();

private:
    void setupSidePanel();
    void setupMainContent();

    // UI Elements
    QWidget *m_sidePanel;
    QPushButton *m_taskButton;
    QPushButton *m_calendarButton;
    QPushButton *m_notesButton;
    QPushButton *m_userButton;

    // Main Widgets
    TaskWidget *m_taskWidget;
    CalendarWidget *m_calendarWidget;
    NotesWidget *m_notesWidget;
    UserWidget *m_userWidget;
    QStackedWidget *m_stackedWidget;

    // Auth Windows
    RegistrationWindow *m_regWindow;
    LoginWindow *m_loginWindow;
};

#endif // MAINWINDOW_H
