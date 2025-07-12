#include "RegistrationWindow.h"
#include "DatabaseManager.h"
#include <QFormLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QRegularExpression>

RegistrationWindow::RegistrationWindow(QWidget *parent) :
    QDialog(parent),
    loginEdit(new QLineEdit(this)),
    passwordEdit(new QLineEdit(this)),
    confirmPasswordEdit(new QLineEdit(this)),
    registerButton(new QPushButton(tr("Зарегистрироваться"), this))
{
    setWindowTitle(tr("Регистрация"));
    setFixedSize(350, 200);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Настройка полей ввода
    loginEdit->setPlaceholderText(tr("Введите логин"));
    passwordEdit->setPlaceholderText(tr("Введите пароль"));
    confirmPasswordEdit->setPlaceholderText(tr("Подтвердите пароль"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    // Настройка кнопки
    registerButton->setCursor(Qt::PointingHandCursor);

    // Создание layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    formLayout->addRow(tr("Логин:"), loginEdit);
    formLayout->addRow(tr("Пароль:"), passwordEdit);
    formLayout->addRow(tr("Подтвердите пароль:"), confirmPasswordEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(registerButton);

    // Подключение сигналов
    connect(registerButton, &QPushButton::clicked, this, &RegistrationWindow::attemptRegistration);
    connect(loginEdit, &QLineEdit::returnPressed, this, &RegistrationWindow::attemptRegistration);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &RegistrationWindow::attemptRegistration);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, this, &RegistrationWindow::attemptRegistration);
}

void RegistrationWindow::attemptRegistration()
{
    QString login = loginEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    // Валидация данных
    if (login.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        showErrorMessage(tr("Все поля должны быть заполнены"));
        return;
    }

    if (login.length() < 3 || login.length() > 20) {
        showErrorMessage(tr("Логин должен содержать от 3 до 20 символов"));
        return;
    }

    if (!QRegularExpression("^[a-zA-Z0-9_]+$").match(login).hasMatch()) {
        showErrorMessage(tr("Логин может содержать только латинские буквы, цифры и подчеркивание"));
        return;
    }

    if (password.length() < 6) {
        showErrorMessage(tr("Пароль должен содержать не менее 6 символов"));
        return;
    }

    if (password != confirmPassword) {
        showErrorMessage(tr("Пароли не совпадают"));
        return;
    }

    // Попытка регистрации
    DatabaseManager &dbManager = DatabaseManager::instance();

    if (dbManager.userExists(login)) {
        showErrorMessage(tr("Пользователь с таким логином уже существует"));
        return;
    }

    if (!dbManager.registerUser(login, password)) {
        showErrorMessage(tr("Ошибка регистрации: ") + dbManager.lastError().text());
        return;
    }

    QMessageBox::information(this, tr("Успех"), tr("Регистрация прошла успешно!"));
    accept();
}

void RegistrationWindow::showErrorMessage(const QString &message)
{
    QMessageBox::warning(this, tr("Ошибка"), message);
    loginEdit->setFocus();
}
