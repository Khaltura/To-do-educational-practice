#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class TaskWidget;
class CalendarWidget;
class NotesWidget;
class UserWidget;
class RegistrationWindow;
class LoginWindow;

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

    QWidget *sidePanel;
    QPushButton *taskButton;
    QPushButton *calendarButton;
    QPushButton *notesButton;
    QPushButton *userButton;

    TaskWidget *taskWidget;
    CalendarWidget *calendarWidget;
    NotesWidget *notesWidget;
    UserWidget *userWidget;

    QStackedWidget *stackedWidget;
    RegistrationWindow *regWindow;
    LoginWindow *loginWindow;
};

#endif // MAINWINDOW_H
