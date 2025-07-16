
#include "RegistrationWindow.h"
#include "DatabaseManager.h"
#include <QFormLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QSqlError>
#include <QDebug>
#include <QDir>

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
    loginEdit->setPlaceholderText(tr("Введите логин (латиница/цифры)"));
    passwordEdit->setPlaceholderText(tr("Введите пароль (6+ символов)"));
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
    qDebug() << "=== Начало попытки регистрации ===";

    QString login = loginEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    qDebug() << "Введенные данные:";
    qDebug() << "Логин:" << login;
    qDebug() << "Пароль:" << QString(password.length(), '*');
    qDebug() << "Подтверждение:" << QString(confirmPassword.length(), '*');

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
        showErrorMessage(tr("Логин может содержать только:\n- Латинские буквы (A-Z, a-z)\n- Цифры (0-9)\n- Символ подчеркивания (_)"));
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
    qDebug() << "Проверка существования пользователя...";

    if (dbManager.userExists(login)) {
        showErrorMessage(tr("Пользователь с таким логином уже существует"));
        return;
    }

    qDebug() << "Попытка регистрации нового пользователя...";
    if (!dbManager.registerUser(login, password)) {
        showErrorMessage(tr("Ошибка при создании пользователя.\nПопробуйте другой логин или перезапустите приложение."));

        // Дополнительная диагностика
        QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        qDebug() << "Диагностика:";
        qDebug() << "Путь к базе данных:" << dbPath;
        qDebug() << "Существует ли директория:" << QDir(dbPath).exists();
        qDebug() << "Содержимое директории:" << QDir(dbPath).entryList();

        return;
    }

    qDebug() << "Регистрация прошла успешно!";
    QMessageBox::information(this, tr("Успех"), tr("Регистрация завершена успешно!\nТеперь вы можете войти в систему."));
    accept();
}

void RegistrationWindow::showErrorMessage(const QString &message)
{
    qDebug() << "Ошибка регистрации:" << message;
    QMessageBox::warning(this, tr("Ошибка"), message);
    loginEdit->setFocus();
}
