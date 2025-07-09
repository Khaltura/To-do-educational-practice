#include "User.h"

User::User() : m_groupId(-1) {}

User::User(const QString &login, const QString &password, const QString &email, int groupId) :
    m_login(login),
    m_password(password),
    m_email(email),
    m_groupId(groupId)
{
}

QString User::getLogin() const { return m_login; }
QString User::getPassword() const { return m_password; }
QString User::getEmail() const { return m_email; }
int User::getGroupId() const { return m_groupId; }

void User::setLogin(const QString &login) { m_login = login; }
void User::setPassword(const QString &password) { m_password = password; }
void User::setEmail(const QString &email) { m_email = email; }
void User::setGroupId(int groupId) { m_groupId = groupId; }

bool User::isValid() const {
    return !m_login.isEmpty() && !m_password.isEmpty() && !m_email.isEmpty();
}
