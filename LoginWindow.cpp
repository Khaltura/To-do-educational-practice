
#include "LoginWindow.h"
#include "DatabaseManager.h"
#include <QFormLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QRegularExpression>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Вход");
    setFixedSize(350, 200);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    loginEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("Войти", this);

    formLayout->addRow("Логин:", loginEdit);
    formLayout->addRow("Пароль:", passwordEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(loginButton);

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::attemptLogin);
}

void LoginWindow::attemptLogin()
{
    DatabaseManager &dbManager = DatabaseManager::instance();

    QString login = loginEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Логин и пароль не могут быть пустыми");
        return;
    }

    if (dbManager.loginUser(login, password)) {
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Неверный логин или пароль\nИли проблемы с подключением к базе данных");
    }
}
