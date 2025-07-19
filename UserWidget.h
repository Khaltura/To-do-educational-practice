#ifndef USERWIDGET_H
#define USERWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;
class QVBoxLayout;
class QListWidget;

class UserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserWidget(QWidget *parent = nullptr);
    ~UserWidget();

    void updateUI(); // Переносим в public секцию

signals:
    void registrationRequested();
    void loginRequested();

private slots:
    void handleCreateGroup();
    void handleJoinGroup();
    void handleLeaveGroup();

private:
    void initUI();
    void setupConnections();
    void applyStyles();

    QVBoxLayout *m_layout;
    QLabel *m_userLabel;
    QLabel *m_groupLabel;
    QListWidget *m_membersList;
    QPushButton *m_registerButton;
    QPushButton *m_loginButton;
    QPushButton *m_logoutButton;
    QPushButton *m_createGroupButton;
    QPushButton *m_joinGroupButton;
    QPushButton *m_leaveGroupButton;
};

#endif // USERWIDGET_H
