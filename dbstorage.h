#pragma once
#include <QString>
#include <vector>
#include "contacts.h"
#include <QSqlDatabase>

class DBStorage {
    QSqlDatabase db;

public:
    DBStorage(const QString &dbname,
              const QString &user,
              const QString &password = "EfanoV0706",
              const QString &host = "localhost",
              int port = 5435);

    ~DBStorage();

    bool open();
    void close();

    bool saveAll(const std::vector<contacts> &all);
    std::vector<contacts> loadAll();

    bool isOpen() const { return db.isOpen(); }
};
