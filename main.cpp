#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include "MainWindow.h"
#include "DatabaseManager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Установка информации о приложении
    app.setApplicationName("ToDo App");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("YourCompany");

    // Глобальный стиль приложения (тёмная тема)
    app.setStyleSheet(R"(
        QWidget {
            background-color: #121212;
            color: #ffffff;
            font-family: 'Segoe UI', sans-serif;
            border: none;
        }
        QPushButton {
            background-color: #2d89ef;
            color: white;
            border-radius: 8px;
            padding: 10px;
            font-size: 16px;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #1e5cb3;
        }
        QPushButton:pressed {
            background-color: #0c3b7a;
        }
        QWidget#sidePanel {
            background-color: #1e1e1e;
            border-right: 1px solid #333;
        }
        QLabel {
            font-size: 20px;
            color: #ffffff;
        }
        QLineEdit, QTextEdit {
            background-color: #2d2d2d;
            color: #ffffff;
            border: 1px solid #444;
            border-radius: 4px;
            padding: 5px;
        }
    )");

    // Получаем экземпляр DatabaseManager
    DatabaseManager& dbManager = DatabaseManager::instance();

    // Проверяем наличие ошибки (альтернатива isConnected)
    if (dbManager.lastError().isValid()) {
        QMessageBox::critical(nullptr, "Ошибка",
                              "Не удалось подключиться к базе данных.\n"
                              "Ошибка: " + dbManager.lastError().text());
        return -1;
    }

    qDebug() << "Приложение успешно инициализировано";

    MainWindow window;
    window.setMinimumSize(800, 600);
    window.resize(1000, 700);
    window.show();

    return app.exec();
}
