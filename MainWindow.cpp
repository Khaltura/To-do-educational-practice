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
    m_sidePanel(new QWidget(this)),
    m_taskButton(new QPushButton("📋 Задачи", this)),
    m_calendarButton(new QPushButton("📅 Календарь", this)),
    m_notesButton(new QPushButton("📝 Заметки", this)),
    m_userButton(new QPushButton("👤 Пользователь", this)),
    m_taskWidget(new TaskWidget(&DatabaseManager::instance(), this)),
    m_calendarWidget(new CalendarWidget(m_taskWidget, this)),
    m_notesWidget(new NotesWidget(&DatabaseManager::instance(), this)),
    m_userWidget(new UserWidget(this)),
    m_stackedWidget(new QStackedWidget(this)),
    m_regWindow(nullptr),
    m_loginWindow(nullptr)
{
    setWindowTitle("To-Do Приложение");
    resize(1000, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    setupMainContent();
    setupSidePanel();

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(m_sidePanel);
    mainLayout->addWidget(m_stackedWidget, 1);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    connect(m_userWidget, &UserWidget::registrationRequested, this, &MainWindow::showRegistrationWindow);
    connect(m_userWidget, &UserWidget::loginRequested, this, &MainWindow::showLoginWindow);

    auto& dbManager = DatabaseManager::instance();
    connect(&dbManager, &DatabaseManager::loggedIn, this, &MainWindow::handleUserLoggedIn);
    connect(&dbManager, &DatabaseManager::loggedOut, this, &MainWindow::handleUserLoggedOut);

    if (dbManager.isLoggedIn()) {
        handleUserLoggedIn();
    }
}

MainWindow::~MainWindow()
{
    if (m_regWindow) m_regWindow->deleteLater();
    if (m_loginWindow) m_loginWindow->deleteLater();
}

void MainWindow::setupSidePanel()
{
    m_sidePanel->setObjectName("sidePanel");
    m_sidePanel->setFixedWidth(200);
    m_sidePanel->setStyleSheet(
        "QWidget#sidePanel {"
        "  background-color: #1e1e1e;"
        "  border-right: 1px solid #444;"
        "}"
        );

    QVBoxLayout *sideLayout = new QVBoxLayout(m_sidePanel);
    sideLayout->setContentsMargins(10, 20, 10, 20);
    sideLayout->setSpacing(15);

    QString sideButtonStyle =
        "QPushButton {"
        "  text-align: left;"
        "  padding: 10px;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 14px;"
        "  color: white;"
        "  background-color: #1976D2;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2196F3;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #0D47A1;"
        "}";

    m_taskButton->setStyleSheet(sideButtonStyle);
    m_calendarButton->setStyleSheet(sideButtonStyle);
    m_notesButton->setStyleSheet(sideButtonStyle);
    m_userButton->setStyleSheet(sideButtonStyle);

    sideLayout->addWidget(m_taskButton);
    sideLayout->addWidget(m_calendarButton);
    sideLayout->addWidget(m_notesButton);
    sideLayout->addStretch();
    sideLayout->addWidget(m_userButton);

    connect(m_taskButton, &QPushButton::clicked, this, [this]() {
        m_stackedWidget->setCurrentWidget(m_taskWidget);
    });

    connect(m_calendarButton, &QPushButton::clicked, this, [this]() {
        m_stackedWidget->setCurrentWidget(m_calendarWidget);
    });

    connect(m_notesButton, &QPushButton::clicked, this, [this]() {
        m_stackedWidget->setCurrentWidget(m_notesWidget);
    });

    connect(m_userButton, &QPushButton::clicked, this, [this]() {
        m_stackedWidget->setCurrentWidget(m_userWidget);
    });
}

void MainWindow::setupMainContent()
{
    m_stackedWidget->addWidget(m_taskWidget);
    m_stackedWidget->addWidget(m_calendarWidget);
    m_stackedWidget->addWidget(m_notesWidget);
    m_stackedWidget->addWidget(m_userWidget);

    setStyleSheet(
        "QMainWindow {"
        "  background-color: #121212;"
        "}"
        "QWidget {"
        "  color: white;"
        "}"
        );
}

void MainWindow::showRegistrationWindow()
{
    if (!m_regWindow) {
        m_regWindow = new RegistrationWindow(this);
        connect(m_regWindow, &QDialog::finished, this, [this]() {
            m_regWindow->deleteLater();
            m_regWindow = nullptr;
        });
    }
    m_regWindow->show();
}

void MainWindow::showLoginWindow()
{
    if (!m_loginWindow) {
        m_loginWindow = new LoginWindow(this);
        connect(m_loginWindow, &QDialog::finished, this, [this]() {
            m_loginWindow->deleteLater();
            m_loginWindow = nullptr;
        });
    }
    m_loginWindow->show();
}

void MainWindow::handleUserLoggedIn()
{
    m_userWidget->updateUI();
    m_userButton->setText("👤 " + DatabaseManager::instance().currentUser());
    m_stackedWidget->setCurrentWidget(m_taskWidget);
    QMessageBox::information(this, "Успех", "Вы успешно вошли в систему");
}

void MainWindow::handleUserLoggedOut()
{
    m_userWidget->updateUI();
    m_userButton->setText("👤 Пользователь");
    m_stackedWidget->setCurrentWidget(m_userWidget);
    QMessageBox::information(this, "Выход", "Вы вышли из системы");
}
