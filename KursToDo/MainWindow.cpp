#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QWidget *sidePanel = new QWidget;
    sidePanel->setObjectName("SidePanel");
    QVBoxLayout *sideLayout = new QVBoxLayout;
    sideLayout->setContentsMargins(10, 10, 10, 10);
    sideLayout->setSpacing(10);

    taskButton = new QPushButton("📋 Tasks");
    calendarButton = new QPushButton("📅 Calendar");
    notesButton = new QPushButton("📝 Notes");

    sideLayout->addWidget(taskButton);
    sideLayout->addWidget(calendarButton);
    sideLayout->addWidget(notesButton);
    sideLayout->addStretch();
    sidePanel->setLayout(sideLayout);
    sidePanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // Создаем один объект TaskWidget
    taskWidget = new TaskWidget;

    // Передаем указатель taskWidget в CalendarWidget, чтобы он мог показывать задачи по дате
    calendarWidget = new CalendarWidget(taskWidget);

    notesWidget = new NotesWidget;

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(taskWidget);        // Индекс 0
    stackedWidget->addWidget(calendarWidget);    // Индекс 1
    stackedWidget->addWidget(notesWidget);       // Индекс 2

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(sidePanel);
    mainLayout->addWidget(stackedWidget);
    centralWidget->setLayout(mainLayout);

    connect(taskButton, &QPushButton::clicked, [=](){ stackedWidget->setCurrentIndex(0); });
    connect(calendarButton, &QPushButton::clicked, [=](){ stackedWidget->setCurrentIndex(1); });
    connect(notesButton, &QPushButton::clicked, [=](){ stackedWidget->setCurrentIndex(2); });
}
