#ifndef REGISTRATIONWINDOW_H
#define REGISTRATIONWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class RegistrationWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RegistrationWindow(QWidget *parent = nullptr);
    ~RegistrationWindow() = default;

private slots:
    void attemptRegistration();

private:
    void showErrorMessage(const QString &message);
    bool validateInput(const QString &login, const QString &password, const QString &confirmPassword);

    QLineEdit *loginEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *registerButton;

    // Запрещаем копирование
    RegistrationWindow(const RegistrationWindow&) = delete;
    RegistrationWindow& operator=(const RegistrationWindow&) = delete;
};

#endif // REGISTRATIONWINDOW_H
