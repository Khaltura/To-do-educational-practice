#include "RegistrationWindow.h"
#include "DatabaseManager.h"
#include <QFormLayout>
#include <QMessageBox>
#include <QVBoxLayout>

RegistrationWindow::RegistrationWindow(QWidget *parent) :
    QDialog(parent),
    loginEdit(new QLineEdit(this)),
    passwordEdit(new QLineEdit(this)),
    confirmPasswordEdit(new QLineEdit(this)),
    registerButton(new QPushButton("Зарегистрироваться", this))
{
    setWindowTitle("Регистрация");
    setFixedSize(350, 200);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    passwordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow("Логин:", loginEdit);
    formLayout->addRow("Пароль:", passwordEdit);
    formLayout->addRow("Подтвердите пароль:", confirmPasswordEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(registerButton);

    connect(registerButton, &QPushButton::clicked, this, &RegistrationWindow::attemptRegistration);
}

void RegistrationWindow::attemptRegistration()
{
    DatabaseManager &dbManager = DatabaseManager::instance();

    QString login = loginEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    // Проверка пустых полей
    if (login.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Все поля должны быть заполнены");
        return;
    }

    // Проверка длины логина
    if (login.length() < 3) {
        QMessageBox::warning(this, "Ошибка", "Логин должен содержать не менее 3 символов");
        return;
    }

    // Проверка совпадения паролей
    if (password != confirmPassword) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают");
        return;
    }

    // Проверка длины пароля
    if (password.length() < 6) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен содержать не менее 6 символов");
        return;
    }

    // Попытка регистрации
    if (dbManager.registerUser(login, password)) {
        QMessageBox::information(this, "Успех", "Регистрация прошла успешно!");
        accept();
    } else {
        QString errorMessage = "Не удалось зарегистрироваться.\n";

        if (dbManager.userExists(login)) {
            errorMessage += "Пользователь с таким логином уже существует.";
        } else {
            errorMessage += "Ошибка базы данных: " + dbManager.lastError().text();
        }

        QMessageBox::critical(this, "Ошибка", errorMessage);
    }
}
