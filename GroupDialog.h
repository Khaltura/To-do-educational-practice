#ifndef GROUPDIALOG_H
#define GROUPDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class GroupDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode { CreateGroup, JoinGroup };

    explicit GroupDialog(Mode mode, QWidget *parent = nullptr);
    QString groupCode() const;

private:
    QLineEdit *m_codeInput;
    QPushButton *m_confirmButton;
    Mode m_mode;
};

#endif // GROUPDIALOG_H
