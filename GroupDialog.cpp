#include "GroupDialog.h"

GroupDialog::GroupDialog(Mode mode, QWidget *parent)
    : QDialog(parent), m_mode(mode)
{
    setWindowTitle(mode == CreateGroup ? "Создать группу" : "Войти в группу");
    setFixedSize(300, 150);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel(mode == CreateGroup ?
                                   "Введите название группы:" : "Введите код группы:", this);
    m_codeInput = new QLineEdit(this);
    m_confirmButton = new QPushButton(mode == CreateGroup ?
                                          "Создать" : "Присоединиться", this);

    layout->addWidget(label);
    layout->addWidget(m_codeInput);
    layout->addWidget(m_confirmButton);

    connect(m_confirmButton, &QPushButton::clicked, this, &QDialog::accept);
}

QString GroupDialog::groupCode() const
{
    return m_codeInput->text().trimmed();
}
