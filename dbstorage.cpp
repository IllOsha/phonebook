#include "dbstorage.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

DBStorage::DBStorage(const QString &dbname,
                     const QString &user,
                     const QString &password,
                     const QString &host,
                     int port)
{
    db = QSqlDatabase::addDatabase("QPSQL",
        QString("phonebook_conn_%1").arg(reinterpret_cast<uintptr_t>(this)));

    db.setDatabaseName(dbname);
    db.setUserName(user);
    db.setPassword(password);
    db.setHostName(host);
    db.setPort(port);
}

DBStorage::~DBStorage() {
    if (db.isOpen()) db.close();
    QSqlDatabase::removeDatabase(db.connectionName());
}

bool DBStorage::open() {
    if (!db.open()) {
        qWarning() << "DB open error:" << db.lastError().text();
        return false;
    }

    QSqlQuery q(db);

    // Таблица контактов
    QString createContacts =
        "CREATE TABLE IF NOT EXISTS contacts ("
        "id SERIAL PRIMARY KEY,"
        "firstname TEXT,"
        "lastname TEXT,"
        "middlename TEXT,"
        "birthday TEXT,"
        "email TEXT,"
        "address TEXT"
        ");";

    if (!q.exec(createContacts)) {
        qWarning() << "create contacts error:" << q.lastError().text();
        return false;
    }

    // Таблица телефонов
    QString createPhones =
        "CREATE TABLE IF NOT EXISTS phones ("
        "id SERIAL PRIMARY KEY,"
        "contact_id INTEGER REFERENCES contacts(id) ON DELETE CASCADE,"
        "label TEXT,"
        "number TEXT"
        ");";

    if (!q.exec(createPhones)) {
        qWarning() << "create phones error:" << q.lastError().text();
        return false;
    }

    return true;
}

void DBStorage::close() {
    if (db.isOpen())
        db.close();
}

bool DBStorage::saveAll(const std::vector<contacts> &all) {
    if (!db.isOpen() && !open())
        return false;

    QSqlQuery q(db);

    if (!q.exec("TRUNCATE phones, contacts RESTART IDENTITY CASCADE;")) {
        qWarning() << "truncate error:" << q.lastError().text();
        return false;
    }

    for (const auto &c : all) {

        QSqlQuery ins(db);
        ins.prepare(
            "INSERT INTO contacts "
            "(firstname, lastname, middlename, birthday, email, address)"
            "VALUES (:fn, :ln, :mn, :bd, :em, :ad) RETURNING id;"
        );

        ins.bindValue(":fn", QString::fromStdString(c.firstname));
        ins.bindValue(":ln", QString::fromStdString(c.lastname));
        ins.bindValue(":mn", QString::fromStdString(c.middlename));
        ins.bindValue(":bd", QString::fromStdString(c.birthday));
        ins.bindValue(":em", QString::fromStdString(c.email));
        ins.bindValue(":ad", QString::fromStdString(c.address));

        if (!ins.exec()) {
            qWarning() << "insert contact error:" << ins.lastError().text();
            return false;
        }

        ins.next();
        int cid = ins.value(0).toInt();

        for (const auto &ph : c.phones) {

            QSqlQuery phq(db);
            phq.prepare(
                "INSERT INTO phones (contact_id, label, number) "
                "VALUES (:cid, :lab, :num);"
            );
            phq.bindValue(":cid", cid);
            phq.bindValue(":lab", QString::fromStdString(ph.label));
            phq.bindValue(":num", QString::fromStdString(ph.number));

            if (!phq.exec()) {
                qWarning() << "insert phone error:" << phq.lastError().text();
            }
        }
    }

    return true;
}

std::vector<contacts> DBStorage::loadAll() {
    std::vector<contacts> result;

    if (!db.isOpen() && !open())
        return result;

    QSqlQuery q(db);

    if (!q.exec("SELECT id, firstname, lastname, middlename, birthday, email, address "
                "FROM contacts ORDER BY id;"))
    {
        qWarning() << "select error:" << q.lastError().text();
        return result;
    }

    while (q.next()) {
        contacts c;
        int id = q.value(0).toInt();

        c.firstname  = q.value(1).toString().toStdString();
        c.lastname   = q.value(2).toString().toStdString();
        c.middlename = q.value(3).toString().toStdString();
        c.birthday   = q.value(4).toString().toStdString();
        c.email      = q.value(5).toString().toStdString();
        c.address    = q.value(6).toString().toStdString();

        QSqlQuery ph(db);
        ph.prepare("SELECT label, number FROM phones WHERE contact_id = :cid;");
        ph.bindValue(":cid", id);
        ph.exec();

        while (ph.next()) {
            PhoneNumber p;
            p.label = ph.value(0).toString().toStdString();
            p.number = ph.value(1).toString().toStdString();
            c.phones.push_back(p);
        }

        result.push_back(std::move(c));
    }

    return result;
}
