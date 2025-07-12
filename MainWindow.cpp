#include "MainWindow.h"
#include "TaskWidget.h"
#include "CalendarWidget.h"
#include "NotesWidget.h"
#include "UserWidget.h"
#include "RegistrationWindow.h"
#include "LoginWindow.h"
#include "DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    sidePanel(nullptr),
    taskButton(nullptr),
    calendarButton(nullptr),
    notesButton(nullptr),
    userButton(nullptr),
    taskWidget(nullptr),
    calendarWidget(nullptr),
    notesWidget(nullptr),
    userWidget(nullptr),
    stackedWidget(new QStackedWidget(this)),
    regWindow(nullptr),
    loginWindow(nullptr)
{
    setWindowTitle("To-Do Приложение");
    resize(1000, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    setupMainContent();
    setupSidePanel();

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(sidePanel);
    mainLayout->addWidget(stackedWidget, 1);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    connect(userWidget, &UserWidget::registrationRequested, this, &MainWindow::showRegistrationWindow);
    connect(userWidget, &UserWidget::loginRequested, this, &MainWindow::showLoginWindow);

    // Исправлено: сигнал без параметров, в лямбде проверяем состояние через isLoggedIn()
    connect(&DatabaseManager::instance(), &DatabaseManager::authStateChanged, this, [this]() {
        if (DatabaseManager::instance().isLoggedIn()) {
            handleUserLoggedIn();
        } else {
            handleUserLoggedOut();
        }
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupSidePanel()
{
    sidePanel = new QWidget(this);
    sidePanel->setObjectName("sidePanel");
    sidePanel->setFixedWidth(200);
    sidePanel->setStyleSheet(R"(
        QWidget#sidePanel {
            background-color: #1e1e1e;
            border-right: 1px solid #444;
        }
    )");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(10, 20, 10, 20);
    sideLayout->setSpacing(15);

    QString sideButtonStyle = R"(
        QPushButton {
            text-align: left;
            padding: 10px;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            color: white;
            background-color: #1976D2;
        }
        QPushButton:hover {
            background-color: #2196F3;
        }
    )";

    taskButton = new QPushButton("\xF0\x9F\x93\x8B Задачи", sidePanel);
    calendarButton = new QPushButton("\xF0\x9F\x93\x85 Календарь", sidePanel);
    notesButton = new QPushButton("\xF0\x9F\x93\x9D Заметки", sidePanel);
    userButton = new QPushButton("\xF0\x9F\x91\xA4 Пользователь", sidePanel);

    taskButton->setStyleSheet(sideButtonStyle);
    calendarButton->setStyleSheet(sideButtonStyle);
    notesButton->setStyleSheet(sideButtonStyle);
    userButton->setStyleSheet(sideButtonStyle);

    sideLayout->addWidget(taskButton);
    sideLayout->addWidget(calendarButton);
    sideLayout->addWidget(notesButton);
    sideLayout->addStretch();
    sideLayout->addWidget(userButton);

    connect(taskButton, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(0);
    });
    connect(calendarButton, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(1);
    });
    connect(notesButton, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(2);
    });
    connect(userButton, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(3);
    });
}

void MainWindow::setupMainContent()
{
    taskWidget = new TaskWidget(this);
    calendarWidget = new CalendarWidget(taskWidget, this);
    notesWidget = new NotesWidget(this);
    userWidget = new UserWidget(this);

    stackedWidget->addWidget(taskWidget);
    stackedWidget->addWidget(calendarWidget);
    stackedWidget->addWidget(notesWidget);
    stackedWidget->addWidget(userWidget);

    setStyleSheet("background-color: #121212; color: white;");
}

void MainWindow::showRegistrationWindow()
{
    if (!regWindow) {
        regWindow = new RegistrationWindow(this);
        connect(regWindow, &RegistrationWindow::finished, this, [this]() {
            regWindow->deleteLater();
            regWindow = nullptr;
        });
    }
    regWindow->show();
}

void MainWindow::showLoginWindow()
{
    if (!loginWindow) {
        loginWindow = new LoginWindow(this);
        connect(loginWindow, &LoginWindow::finished, this, [this]() {
            loginWindow->deleteLater();
            loginWindow = nullptr;
        });
    }
    loginWindow->show();
}

void MainWindow::handleUserLoggedIn()
{
    if (DatabaseManager::instance().isLoggedIn()) {
        userWidget->updateUI();
        stackedWidget->setCurrentIndex(3);
        QMessageBox::information(this, "Успех", "Вы успешно вошли в систему");
    }
}

void MainWindow::handleUserLoggedOut()
{
    userWidget->updateUI();
    stackedWidget->setCurrentIndex(3);
    QMessageBox::information(this, "Выход", "Вы вышли из системы");
}
