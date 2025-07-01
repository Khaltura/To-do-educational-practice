#ifndef REGISTRATIONWINDOW_H
#define REGISTRATIONWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

class RegistrationWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RegistrationWindow(QWidget *parent = nullptr);

signals:
    void registrationSuccess(const QString &username);

private slots:
    void handleRegistration();

private:
    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QPushButton *registerButton;
};

#endif // REGISTRATIONWINDOW_H
