#ifndef USERWIDGET_H
#define USERWIDGET_H

#include <QWidget>
#include <QListWidget>

class QLabel;
class QPushButton;
class QVBoxLayout;

class UserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserWidget(QWidget *parent = nullptr);
    ~UserWidget();

    void updateUI();

signals:
    void registrationRequested();
    void loginRequested();

private slots:
    void handleCreateGroup();
    void handleJoinGroup();
    void handleLeaveGroup();

private:
    QVBoxLayout *m_layout;
    QLabel *m_userLabel;
    QLabel *m_groupLabel;
    QListWidget *m_membersList; // Новый элемент для списка участников
    QPushButton *m_registerButton;
    QPushButton *m_loginButton;
    QPushButton *m_logoutButton;
    QPushButton *m_createGroupButton;
    QPushButton *m_joinGroupButton;
    QPushButton *m_leaveGroupButton;
};

#endif // USERWIDGET_H
