#include <iostream>
#include <phonebook.h>
#include <validators.h>
#include <limits>
#include "dbstorage.h"

int runConsoleMode();

#ifdef USE_QT_GUI
#include <QApplication>
#include <QDebug>
#include "mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>


int main(int argc, char *argv[]) {
    // а э проверка аргументов командной строки для выбора режима
    bool useGui = true;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--console" || arg == "-c") {
            useGui = false;
        }
        else if (arg == "--gui" || arg == "-g") {
            useGui = true;
        }
    }

    if (useGui) {
        // ну тут типо режим графического интерфейса
        QApplication app(argc, argv);

        // Инициализация базы данных для GUI режима
        DBStorage db("phonebook", "postgres", "EfanoV0706", "localhost", 5432);

        // -------------------- ДОБАВЛЕНО --------------------
        // Выводим список доступных SQL-драйверов, чтобы убедиться что QPSQL доступен
        qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

        // -----------------------------------------------------

        if (!db.open()) {
            QMessageBox::critical(nullptr, "Database Error",
                "Failed to connect to database!\nRunning in offline mode.");
            // Можно продолжить работу без БД
        }

        QFile styleFile(":/style.qss");
        if (styleFile.open(QFile::ReadOnly)) {
            QString style = QTextStream(&styleFile).readAll();
            app.setStyleSheet(style);
            styleFile.close();
        } else {
            QMessageBox::warning(nullptr, "Style Error", "Could not load style.qss");
        }

        mainwindow w;
        // Передаем объект базы данных в mainwindow, если нужно
        // w.setDatabase(&db);
        w.show();

        return app.exec();
    }
    else {
        // Инициализация базы данных для консольного режима
        DBStorage db("phonebook", "postgres", "EfanoV0706", "localhost", 5432);

        // -------------------- ДОБАВЛЕНО --------------------
        // Проверка всех доступных драйверов SQL
        qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();
        // -----------------------------------------------------

        if (!db.open()) {
            std::cerr << "Failed to connect to database! Running in offline mode." << std::endl;
        }

        return runConsoleMode();
    }
}

#else
// если Qt не доступен, всегда используем консольный режим
int main(int argc, char *argv[]) {
    // Инициализация базы данных для консольного режима
    DBStorage db("phonebook", "postgres", "EfanoV0706", "localhost", 5432);

    // -------------------- ДОБАВЛЕНО --------------------
    // Выводим список доступных SQL-драйверов, чтобы убедиться что QPSQL доступен
    qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();
    // -----------------------------------------------------

    if (!db.open()) {
        std::cerr << "Failed to connect to database! Running in offline mode." << std::endl;
    }

    return runConsoleMode();
}
#endif

// реализация консольного режима
int runConsoleMode() {
    phonebook pb;

    // Создаем объект базы данных внутри функции
    DBStorage db("phonebook", "postgres", "EfanoV0706", "localhost", 5432);


    // Пробуем загрузить из базы данных
    if (db.open()) {
        std::cout << "Connected to database. Loading contacts..." << std::endl;
        auto dbContacts = db.loadAll();
        for (const auto& contact : dbContacts) {
            pb.addcontact(contact);
        }
        std::cout << "Loaded " << pb.size() << " contacts from database.\n";
    } else {
        // Если БД не доступна, загружаем из файла
        const std::string datafile = "phonebook.db";
        pb.loadfromfile(datafile);
        std::cout << "Loaded " << pb.size() << " contacts from file.\n";
    }

    while(true) {
        std::cout << "commands: list,add,find,delete,save,stats,exit\n> ";
        std::string cmd;
        if(!(std::cin >> cmd)) break;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (cmd == "list") {
            pb.listall();
        }
        else if (cmd == "add") {
            contacts d;
            std::string tmp;

            std::cout << "FIRST NAME: ";
            std::getline(std::cin, tmp);
            d.firstname = Validators::trim(tmp);

            std::cout << "LAST NAME: ";
            std::getline(std::cin, tmp);
            d.lastname = Validators::trim(tmp);

            std::cout << "MIDDLE NAME: ";
            std::getline(std::cin, tmp);
            d.middlename = Validators::trim(tmp);

            std::cout << "EMAIL: ";
            std::getline(std::cin, tmp);
            d.email = Validators::trim(tmp);

            std::cout << "BIRTHDAY DD-MM-YYYY: ";
            std::getline(std::cin, tmp);
            d.birthday = Validators::trim(tmp);

            std::cout << "ADDRESS: ";
            std::getline(std::cin, tmp);
            d.address = Validators::trim(tmp);

            while(true) {
                std::cout << "add number? (y/n): ";
                std::string a;
                std::getline(std::cin, a);
                a = Validators::trim(a);

                if (a == "n" || a == "N") break;
                if (a.empty()) continue;

                PhoneNumber p;
                std::cout << "label: ";
                std::getline(std::cin, p.label);
                p.label = Validators::trim(p.label);

                std::cout << "number: ";
                std::getline(std::cin, p.number);
                p.number = Validators::trim(p.number);

                if (!Validators::validphone(p.number)) {
                    std::cout << "warning: phone format may be invalid.\n";
                }
                d.phones.push_back(p);
            }

            if (!Validators::validname(d.firstname) || !Validators::validname(d.lastname)) {
                std::cout << "invalid name - not added\n";
            }
            else {
                pb.addcontact(std::move(d));
                std::cout << "added\n";
            }
        }
        else if (cmd == "find") {
            std::string q;
            std::cout << "Enter search query: ";
            std::getline(std::cin, q);
            q = Validators::trim(q);
            auto res = pb.findbyname(q);
            for (auto idx : res) {
                const auto& d = pb.at(idx);
                std::cout << "[" << idx << "] " << d.lastname << " " << d.firstname << " | " << d.email << "\n";
            }
            if (res.empty()) {
                std::cout << "No contacts found.\n";
            }
        }
        else if (cmd == "delete") {
            std::cout << "Enter index to delete: ";
            size_t idx;
            std::cin >> idx;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (pb.removebyindex(idx)) std::cout << "removed\n";
            else std::cout << "bad index\n";
        }
        else if (cmd == "save") {
            // Сохраняем и в базу данных и в файл
            if (db.isOpen()) {
                auto allContacts = pb.getAll(); // Предполагается, что такой метод есть в phonebook
                if (db.saveAll(allContacts)) {
                    std::cout << "Saved to database\n";
                } else {
                    std::cout << "Failed to save to database\n";
                }
            }

            const std::string datafile = "phonebook.db";
            pb.savetofile(datafile);
            std::cout << "Saved to file\n";
        }
        else if (cmd == "stats") {
            std::cout << "Total contacts: " << pb.size() << "\n";
            std::cout << "Database: " << (db.isOpen() ? "connected" : "disconnected") << "\n";
        }
        else if (cmd == "exit") {
            // Сохраняем при выходе
            if (db.isOpen()) {
                auto allContacts = pb.getAll();
                db.saveAll(allContacts);
            }

            const std::string datafile = "phonebook.db";
            pb.savetofile(datafile);
            std::cout << "Goodbye!\n";
            break;
        }
        else {
            std::cout << "unknown command. available: list,add,find,delete,save,stats,exit\n";
        }
    }
    return 0;
}
