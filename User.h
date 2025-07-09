#ifndef USER_H
#define USER_H

#include <QString>

class User
{
public:
    User();
    User(const QString &login, const QString &password, const QString &email, int groupId = -1);

    QString getLogin() const;
    QString getPassword() const;
    QString getEmail() const;
    int getGroupId() const;

    void setLogin(const QString &login);
    void setPassword(const QString &password);
    void setEmail(const QString &email);
    void setGroupId(int groupId);

    bool isValid() const;

private:
    QString m_login;
    QString m_password;
    QString m_email;
    int m_groupId;
};

#endif // USER_H
