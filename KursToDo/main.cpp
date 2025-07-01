// main.cpp
#include <QApplication>
#include "MainWindow.h"
#include "DatabaseManager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Глобальный стиль приложения (тёмная тема)
    app.setStyleSheet(R"(
        QWidget {
            background-color: #121212;
            color: #ffffff;
            font-family: 'Segoe UI', sans-serif;
        }
        QPushButton {
            background-color: #2d89ef;
            color: white;
            border-radius: 8px;
            padding: 10px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #1e5cb3;
        }
        QWidget#SidePanel {
            background-color: #1e1e1e;
        }
        QLabel {
            font-size: 20px;
            color: #ffffff;
        }
    )");

    DatabaseManager dbManager;
    if (!dbManager.openDatabase("tasks_notes.db")) {
        // Ошибка открытия БД
    }
    if (!dbManager.createTables()) {
        // Ошибка создания таблиц
    }

    MainWindow window;
    window.resize(1000, 700);
    window.show();
    return app.exec();
}
