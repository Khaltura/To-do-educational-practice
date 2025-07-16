
#include "UserWidget.h"
#include "DatabaseManager.h"
#include "GroupDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QListWidget>
#include <QSqlError>

UserWidget::UserWidget(QWidget *parent) : QWidget(parent)
{
    // Основной layout
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(10);
    m_layout->setContentsMargins(15, 15, 15, 15);

    // Создаем виджеты
    m_userLabel = new QLabel(this);
    m_groupLabel = new QLabel(this);
    m_membersList = new QListWidget(this);

    // Кнопки
    m_registerButton = new QPushButton("Регистрация", this);
    m_loginButton = new QPushButton("Вход", this);
    m_logoutButton = new QPushButton("Выйти из аккаунта", this);
    m_createGroupButton = new QPushButton("Создать группу", this);
    m_joinGroupButton = new QPushButton("Войти в группу", this);
    m_leaveGroupButton = new QPushButton("Выйти из группы", this);

    // Настройка стилей
    QString buttonStyle = R"(
        QPushButton {
            padding: 8px;
            border-radius: 4px;
            background-color: #1976D2;
            color: white;
            min-width: 160px;
        }
        QPushButton:hover {
            background-color: #2196F3;
        }
    )";

    QString labelStyle = "QLabel {"
                         "font-size: 14px;"
                         "color: #E0E0E0;"
                         "margin-bottom: 5px;"
                         "}";

    // Стиль для списка участников
    QString listStyle = R"(
        QListWidget {
            border: 1px solid #444;
            border-radius: 4px;
            padding: 5px;
            background-color: #121212;
            color: #E0E0E0;
        }
        QListWidget::item {
            padding: 5px;
            border-bottom: 1px solid #333;
        }
        QListWidget::item:hover {
            background-color: #333;
        }
    )";

    m_userLabel->setStyleSheet(labelStyle);
    m_groupLabel->setStyleSheet(labelStyle);
    m_membersList->setStyleSheet(listStyle);
    m_membersList->setMaximumHeight(120);

    m_registerButton->setStyleSheet(buttonStyle);
    m_loginButton->setStyleSheet(buttonStyle);
    m_logoutButton->setStyleSheet(buttonStyle);
    m_createGroupButton->setStyleSheet(buttonStyle);
    m_joinGroupButton->setStyleSheet(buttonStyle);
    m_leaveGroupButton->setStyleSheet(buttonStyle);

    // Расположение элементов
    m_layout->addWidget(m_userLabel);
    m_layout->addWidget(m_groupLabel);
    m_layout->addWidget(m_membersList);
    m_layout->addWidget(m_createGroupButton);
    m_layout->addWidget(m_joinGroupButton);
    m_layout->addWidget(m_leaveGroupButton);
    m_layout->addWidget(m_logoutButton);
    m_layout->addWidget(m_registerButton);
    m_layout->addWidget(m_loginButton);
    m_layout->addStretch();

    // Подключение сигналов
    connect(m_registerButton, &QPushButton::clicked, this, &UserWidget::registrationRequested);
    connect(m_loginButton, &QPushButton::clicked, this, &UserWidget::loginRequested);
    connect(m_logoutButton, &QPushButton::clicked, &DatabaseManager::instance(), &DatabaseManager::logout);
    connect(m_createGroupButton, &QPushButton::clicked, this, &UserWidget::handleCreateGroup);
    connect(m_joinGroupButton, &QPushButton::clicked, this, &UserWidget::handleJoinGroup);
    connect(m_leaveGroupButton, &QPushButton::clicked, this, &UserWidget::handleLeaveGroup);

    // Исправленные подключения сигналов
    auto& dbManager = DatabaseManager::instance();
    connect(&dbManager, &DatabaseManager::loggedIn, this, &UserWidget::updateUI);
    connect(&dbManager, &DatabaseManager::loggedOut, this, &UserWidget::updateUI);

    updateUI();
}

void UserWidget::updateUI()
{
    auto& dbManager = DatabaseManager::instance();
    bool isLoggedIn = dbManager.isLoggedIn();
    QString username = dbManager.currentUser();
    QString groupId = dbManager.currentUserGroup();

    m_userLabel->setText(isLoggedIn ? "Пользователь: " + username : "Гость");

    // Управление видимостью элементов
    m_registerButton->setVisible(!isLoggedIn);
    m_loginButton->setVisible(!isLoggedIn);
    m_logoutButton->setVisible(isLoggedIn);
    m_createGroupButton->setVisible(isLoggedIn && groupId.isEmpty());
    m_joinGroupButton->setVisible(isLoggedIn && groupId.isEmpty());
    m_leaveGroupButton->setVisible(isLoggedIn && !groupId.isEmpty());
    m_groupLabel->setVisible(isLoggedIn && !groupId.isEmpty());
    m_membersList->setVisible(isLoggedIn && !groupId.isEmpty());

    if (isLoggedIn && !groupId.isEmpty()) {
        QString groupName = dbManager.getGroupName(groupId);
        m_groupLabel->setText(QString("Группа: %1 (ID: %2)").arg(groupName).arg(groupId));

        // Обновляем список участников
        m_membersList->clear();
        foreach (const QString &member, dbManager.getGroupMembers(groupId)) {
            QListWidgetItem *item = new QListWidgetItem(member, m_membersList);
            item->setForeground(Qt::white);
        }
    }
}

void UserWidget::handleCreateGroup()
{
    GroupDialog dialog(GroupDialog::CreateGroup, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString groupName = dialog.groupCode();
        QString groupCode;
        if (DatabaseManager::instance().createGroup(groupName, groupCode)) {
            QMessageBox::information(this, "Успех",
                                     QString("Группа создана!\nКод: %1").arg(groupCode));
            updateUI();
        } else {
            QMessageBox::warning(this, "Ошибка",
                                 "Ошибка создания группы: " + DatabaseManager::instance().lastError().text());
        }
    }
}

void UserWidget::handleJoinGroup()
{
    GroupDialog dialog(GroupDialog::JoinGroup, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString groupCode = dialog.groupCode();
        if (DatabaseManager::instance().joinGroup(groupCode)) {
            QMessageBox::information(this, "Успех", "Вы в группе!");
            updateUI();
        } else {
            QMessageBox::warning(this, "Ошибка",
                                 "Неверный код группы: " + DatabaseManager::instance().lastError().text());
        }
    }
}

void UserWidget::handleLeaveGroup()
{
    if (DatabaseManager::instance().leaveGroup()) {
        QMessageBox::information(this, "Успех", "Вы вышли из группы");
        updateUI();
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Ошибка выхода из группы: " + DatabaseManager::instance().lastError().text());
    }
}

UserWidget::~UserWidget()
{
    // Автоматическое удаление виджетов через родительскую систему Qt
}
