#include "RegistrationWindow.h"
#include "DatabaseManager.h"

RegistrationWindow::RegistrationWindow(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Регистрация");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 20px; font-weight: bold;");
    layout->addWidget(title);

    usernameInput = new QLineEdit;
    usernameInput->setPlaceholderText("Имя пользователя");
    layout->addWidget(usernameInput);

    passwordInput = new QLineEdit;
    passwordInput->setPlaceholderText("Пароль");
    passwordInput->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordInput);

    registerButton = new QPushButton("Зарегистрироваться");
    layout->addWidget(registerButton);

    connect(registerButton, &QPushButton::clicked, this, &RegistrationWindow::handleRegistration);
}

void RegistrationWindow::handleRegistration()
{
    QString username = usernameInput->text().trimmed();
    QString password = passwordInput->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, введите имя пользователя и пароль.");
        return;
    }

    DatabaseManager db;
    db.openDatabase("app.db"); // Путь к вашей БД
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM Users WHERE username = :username");
    checkQuery.bindValue(":username", username);
    if (!checkQuery.exec()) {
        QMessageBox::critical(this, "Ошибка", "Ошибка базы данных.");
        return;
    }

    if (checkQuery.next()) {
        QMessageBox::warning(this, "Ошибка", "Пользователь уже существует.");
        return;
    }

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO Users (username, password) VALUES (:username, :password)");
    insertQuery.bindValue(":username", username);
    insertQuery.bindValue(":password", password);
    if (!insertQuery.exec()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось зарегистрировать пользователя.");
        return;
    }

    emit registrationSuccess(username);
    QMessageBox::information(this, "Успех", "Регистрация прошла успешно.");
    close();
}
