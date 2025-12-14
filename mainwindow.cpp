#include "mainwindow.h"
#include "validators.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


// ================== PostgreSQL ==================
static QSqlDatabase pgdb;

static bool initPostgres(QWidget *parent) {
    if (QSqlDatabase::contains("pg"))
        pgdb = QSqlDatabase::database("pg");
    else
        pgdb = QSqlDatabase::addDatabase("QPSQL", "pg");

    pgdb.setHostName("localhost");
    pgdb.setPort(5432);
    pgdb.setDatabaseName("phonebook");
    pgdb.setUserName("postgres");
    pgdb.setPassword("EfanoV0706");

    if (!pgdb.open()) {
        QMessageBox::warning(parent, "PostgreSQL",
                             "PostgreSQL connection failed:\n" + pgdb.lastError().text());
        return false;
    }

    QSqlQuery q(pgdb);

    // contacts
    q.exec(
        "CREATE TABLE IF NOT EXISTS contacts ("
        "id SERIAL PRIMARY KEY,"
        "lastname TEXT,"
        "firstname TEXT,"
        "middlename TEXT,"
        "birthday TEXT,"
        "email TEXT,"
        "address TEXT)"
    );

    // phones
    q.exec(
        "CREATE TABLE IF NOT EXISTS phones ("
        "id SERIAL PRIMARY KEY,"
        "contact_id INTEGER REFERENCES contacts(id) ON DELETE CASCADE,"
        "label TEXT,"
        "number TEXT)"
    );

    return true;
}

// Конструктор и UI
mainwindow::mainwindow(QWidget * parent) : QMainWindow(parent){
    setWindowTitle("phone alo");
    resize(1000, 600);

    QWidget * central = new QWidget(this);
    QBoxLayout * mainlayout = new QVBoxLayout(central);

    QHBoxLayout * toplayout = new QHBoxLayout;
    searchfield = new QLineEdit;
    searchfield->setPlaceholderText("enter text ");
    findBtn = new QPushButton("search");
    resetBtn = new QPushButton("reset");
    addBtn = new QPushButton("add");
    editBtn = new QPushButton("edit");
    delBtn = new QPushButton("delete");
    saveBtn = new QPushButton("save");

    toplayout->addWidget(searchfield);
    toplayout->addWidget(findBtn);
    toplayout->addWidget(resetBtn);
    toplayout->addWidget(addBtn);
    toplayout->addWidget(editBtn);
    toplayout->addWidget(delBtn);
    toplayout->addWidget(saveBtn);

    table = new QTableWidget;
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({"LASTNAME", "FIRSTNAME", "MIDDLENAME", "BIRTHDAY", "EMAIL", "ADDRESS", "PHONES"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    mainlayout->addLayout(toplayout);
    mainlayout->addWidget(table);
    setCentralWidget(central);

    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly))
        setStyleSheet(styleFile.readAll());

    connect(addBtn, &QPushButton::clicked, this, &mainwindow::onadd);
    connect(editBtn, &QPushButton::clicked, this, &mainwindow::onedit);
    connect(delBtn, &QPushButton::clicked, this, &mainwindow::ondelete);
    connect(findBtn, &QPushButton::clicked, this, &mainwindow::onfind);
    connect(resetBtn, &QPushButton::clicked, this, &mainwindow::onreset);
    connect(saveBtn, &QPushButton::clicked, this, &mainwindow::onsave);

    pb.loadfromfile("phonebook.db");
    refreshtable();

    // ===== POSTGRES =====
    if (initPostgres(this)) {
        QSqlQuery q(pgdb);
        q.exec("SELECT * FROM contacts ORDER BY id");

        while (q.next()) {
            contacts c;
            int cid = q.value("id").toInt();

            c.lastname   = q.value("lastname").toString().toStdString();
            c.firstname  = q.value("firstname").toString().toStdString();
            c.middlename = q.value("middlename").toString().toStdString();
            c.birthday   = q.value("birthday").toString().toStdString();
            c.email      = q.value("email").toString().toStdString();
            c.address    = q.value("address").toString().toStdString();

            QSqlQuery qp(pgdb);
            qp.prepare("SELECT label, number FROM phones WHERE contact_id = :id");
            qp.bindValue(":id", cid);
            qp.exec();

            while (qp.next()) {
                PhoneNumber p;
                p.label  = qp.value(0).toString().toStdString();
                p.number = qp.value(1).toString().toStdString();
                c.phones.push_back(p);
            }
            pb.addcontact(std::move(c));
        }
    }

    refreshtable();
}


void mainwindow::refreshtable(){
    table->setRowCount(static_cast<int>(pb.size()));
    for (int i = 0; i < static_cast<int>(pb.size()); ++i){
        const auto &c = pb.at(i);

        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(c.lastname)));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(c.firstname)));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(c.middlename)));
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(c.birthday)));
        table->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(c.email)));
        table->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(c.address)));

        QString phonesStr;
        for (const auto& p : c.phones)
            phonesStr += QString::fromStdString(p.label + ": " + p.number + " ; ");
        table->setItem(i, 6, new QTableWidgetItem(phonesStr.trimmed()));
    }
}

// показываем ошибку понятной строкой (текст чёрный)
static void showErrorMsg(QWidget *parent, const QString &message) {
    QMessageBox msg(parent);
    msg.setIcon(QMessageBox::Warning);
    msg.setWindowTitle("Validation errors");
    msg.setText("<span style='color:black;'>" + message.toHtmlEscaped() + "</span>");
    msg.exec();
}

// ===== Вспомогательные функции для корректного ввода =====
QString mainwindow::getValidName(const QString &title, bool allowEmpty, const QString &defaultText){
    while (true){
        bool ok;
        QString prompt = "Enter " + title.toLower() + ":";
        QString s = QInputDialog::getText(this, title, prompt, QLineEdit::Normal, defaultText, &ok);
        if (!ok) return QString(); // отмена
        s = s.trimmed();

        // нормализация: склеивание одиночных букв, удаляем лишние пробелы вокруг дефиса
        std::string norm = Validators::normalize_name(s.toStdString());
        QString qs = QString::fromStdString(norm);

        if (qs.isEmpty() && allowEmpty) return QString();
        if (!qs.isEmpty() && Validators::validname(qs.toStdString())) return qs;

        showErrorMsg(this, QString("Invalid %1! Only letters, digits, hyphen and spaces allowed. Must start with a letter and not start/end with hyphen.").arg(title.toLower()));
    }
}

QString mainwindow::getValidEmail(const QString &title, const QString &defaultText){
    while (true){
        bool ok;
        QString s = QInputDialog::getText(this, title, "Enter " + title.toLower() + ":", QLineEdit::Normal, defaultText, &ok);
        if (!ok) return QString();
        s = s.trimmed();
        // убрать пробелы вокруг @
        s.replace(QRegExp("\\s*@\\s*"), "@");

        if (!s.isEmpty() && Validators::valiemail(s.toStdString())) return s;

        showErrorMsg(this, "Invalid email! Must be ASCII letters/digits in user and domain (e.g. example@mail.com).");
    }
}

QString mainwindow::getValidBirthday(const QString &title, const QString &defaultText){
    while (true){
        bool ok;
        QString s = QInputDialog::getText(this, title, "Enter birthday (DD.MM.YYYY):", QLineEdit::Normal, defaultText, &ok);
        if (!ok) return QString();
        s = s.trimmed();
        if (!s.isEmpty() && Validators::validbirthday(s.toStdString())) return s;
        showErrorMsg(this, "Invalid birthday! Format DD.MM.YYYY and must be before today.");
    }
}

QString mainwindow::getValidAddress(const QString &title, const QString &defaultText){
    while (true){
        bool ok;
        QString s = QInputDialog::getText(this, title, "Enter address:", QLineEdit::Normal, defaultText, &ok);
        if (!ok) return QString();
        s = s.trimmed();
        if (!s.isEmpty() && Validators::validaddress(s.toStdString())) return s;
        showErrorMsg(this, "Address cannot be empty!");
    }
}

// ==== ADD ====
void mainwindow::onadd() {
    contacts c;

    QString lname = getValidName("LASTNAME", false);
    if (lname.isEmpty()) return;
    c.lastname = lname.toStdString();

    QString fname = getValidName("FIRSTNAME", false);
    if (fname.isEmpty()) return;
    c.firstname = fname.toStdString();

    QString mname = getValidName("MIDDLENAME", true);
    if (mname.isEmpty()) c.middlename = "";
    else c.middlename = mname.toStdString();

    QString email = getValidEmail("EMAIL");
    if (email.isEmpty()) return;
    c.email = email.toStdString();

    QString bday = getValidBirthday("BIRTHDAY");
    if (bday.isEmpty()) return;
    c.birthday = bday.toStdString();

    QString addr = getValidAddress("ADDRESS");
    if (addr.isEmpty()) return;
    c.address = addr.toStdString();

    // PHONE(S)
    // требуется минимум один номер по ТЗ
    QVector<PhoneNumber> phonesLocal;
    while (true) {
        bool ok;
        QString label = QInputDialog::getText(this, "PHONE LABEL", "Enter phone label (home/work/etc):", QLineEdit::Normal, "", &ok);
        if (!ok) break; // отмена добавления телефонов -> если нет ни одного номера в итоге — ошибка
        label = label.trimmed();
        if (label.isEmpty()) continue;

        QString number = QInputDialog::getText(this, "PHONE NUMBER", "Enter phone number (+7... or 8...):", QLineEdit::Normal, "", &ok);
        if (!ok) break; // отмена
        number = number.trimmed();
        number = QString::fromStdString(Validators::normalize_phone(number.toStdString()));

        if (!Validators::validphone(number.toStdString())) {
            showErrorMsg(this, "Invalid phone number! Must start with +7 or 8 followed by 10 digits.");
            continue;
        }

        PhoneNumber p;
        p.label = label.toStdString();
        p.number = number.toStdString();
        phonesLocal.push_back(p);

        // спросим — добавить ещё?
        QMessageBox addMore;
        addMore.setWindowTitle("More phones?");
        addMore.setText("Add another phone?");
        addMore.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        if (addMore.exec() == QMessageBox::No) break;
    }

    if (phonesLocal.empty()) {
        showErrorMsg(this, "At least one phone number is required.");
        return;
    }

    for (auto &ph : phonesLocal) c.phones.push_back(ph);

    pb.addcontact(std::move(c));
    refreshtable();
    QMessageBox::information(this, "Success", "Contact added successfully!");
}

void mainwindow::onedit() {
    int row = table->currentRow();
    if (row < 0) { showErrorMsg(this, "Please select a contact to edit."); return; }

    contacts &orig = pb.at(static_cast<size_t>(row));
    contacts tmp = orig;

    // ==== LASTNAME ====
    QString lname = getValidName("LASTNAME", false, QString::fromStdString(tmp.lastname));
    if (lname.isEmpty()) return;
    lname = QString::fromStdString(Validators::normalize_name(lname.toStdString()));
    tmp.lastname = lname.toStdString();

    // ==== FIRSTNAME ====
    QString fname = getValidName("FIRSTNAME", false, QString::fromStdString(tmp.firstname));
    if (fname.isEmpty()) return;
    fname = QString::fromStdString(Validators::normalize_name(fname.toStdString()));
    tmp.firstname = fname.toStdString();

    // ==== MIDDLENAME ====
    QString mname = getValidName("MIDDLENAME", true, QString::fromStdString(tmp.middlename));
    if (mname.isNull()) return;
    if (!mname.isEmpty())
        mname = QString::fromStdString(Validators::normalize_name(mname.toStdString()));
    tmp.middlename = mname.toStdString();

    // EMAIL
    QString email = getValidEmail("EMAIL", QString::fromStdString(tmp.email));
    if (email.isEmpty()) return;
    tmp.email = email.toStdString();

    // BIRTHDAY
    QString bday = getValidBirthday("BIRTHDAY", QString::fromStdString(tmp.birthday));
    if (bday.isEmpty()) return;
    tmp.birthday = bday.toStdString();

    // ADDRESS
    QString addr = getValidAddress("ADDRESS", QString::fromStdString(tmp.address));
    if (addr.isEmpty()) return;
    tmp.address = addr.toStdString();

    // PHONES: редактируем существующие, даём возможность удалить/изменить/добавить новые
    std::vector<PhoneNumber> newPhones;
    // редактирование текущих
    for (size_t i = 0; i < tmp.phones.size(); ++i) {
        const PhoneNumber &ph = tmp.phones[i];
        // спользуем диалог, чтобы предложить изменить лейбл (по умолчанию текущее)
        bool ok;
        QString label = QInputDialog::getText(this, "PHONE LABEL", QString("Edit label for phone #%1:").arg(i+1),
                                              QLineEdit::Normal, QString::fromStdString(ph.label), &ok);
        if (!ok) {
            // если отмена — даём возможность полностью отменить редактирование
            QMessageBox::StandardButton res = QMessageBox::question(this, "Cancel editing",
                                                                    "Cancel editing phones? This will abort whole edit.", QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes) return; // abort entire edit
            // else продолжим (переоткрываем диалог)
            --i; // повтор
            continue;
        }
        label = label.trimmed();
        if (label.isEmpty()) {
            // если пользователь стер label — предложим удалить этот номер
            QMessageBox::StandardButton rb = QMessageBox::question(this, "Remove phone?",
                                                                  "Label is empty — remove this phone?", QMessageBox::Yes | QMessageBox::No);
            if (rb == QMessageBox::Yes) continue; // телефон удалён, не добавляем в newPhones
            // иначе попросим повторить
            --i; continue;
        }

        QString number = QInputDialog::getText(this, "PHONE NUMBER", QString("Edit number for phone #%1:").arg(i+1),
                                               QLineEdit::Normal, QString::fromStdString(ph.number), &ok);
        if (!ok) {
            QMessageBox::StandardButton res = QMessageBox::question(this, "Cancel editing",
                                                                    "Cancel editing phones? This will abort whole edit.", QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes) return;
            --i; continue;
        }
        number = QString::fromStdString(Validators::normalize_phone(number.toStdString()));
        if (!Validators::validphone(number.toStdString())) {
            showErrorMsg(this, "Invalid phone number! Must start with +7 or 8 followed by 10 digits.");
            --i; continue;
        }
        PhoneNumber np;
        np.label = label.toStdString();
        np.number = number.toStdString();
        newPhones.push_back(np);
    }

    // возможность добавить новые номера
    QMessageBox::StandardButton addMore = QMessageBox::question(this, "Add more phones", "Add more phone numbers?", QMessageBox::Yes | QMessageBox::No);
    if (addMore == QMessageBox::Yes) {
        while (true) {
            bool ok;
            QString label = QInputDialog::getText(this, "PHONE LABEL", "Enter phone label (home/work/etc):", QLineEdit::Normal, "", &ok);
            if (!ok) break;
            label = label.trimmed();
            if (label.isEmpty()) continue;

            QString number = QInputDialog::getText(this, "PHONE NUMBER", "Enter phone number (+7... or 8...):", QLineEdit::Normal, "", &ok);
            if (!ok) break;
            number = QString::fromStdString(Validators::normalize_phone(number.toStdString()));
            if (!Validators::validphone(number.toStdString())) {
                showErrorMsg(this, "Invalid phone number! Must start with +7 or 8 followed by 10 digits.");
                continue;
            }
            PhoneNumber np;
            np.label = label.toStdString();
            np.number = number.toStdString();
            newPhones.push_back(np);

            QMessageBox::StandardButton more = QMessageBox::question(this, "More?", "Add another phone?", QMessageBox::Yes | QMessageBox::No);
            if (more == QMessageBox::No) break;
        }
    }

    // Валидация: по ТЗ требуется минимум 1 номер
    if (newPhones.empty()) {
        showErrorMsg(this, "At least one phone number must remain. Edit cancelled.");
        return;
    }
    tmp.phones = newPhones;

    // Если дошли сюда — все валидно. Копируем tmp в orig
    orig = tmp;
    refreshtable();
    QMessageBox::information(this, "Success", "Contact edited successfully!");
}

// ==== DELETE ====
void mainwindow::ondelete(){
    int row = table->currentRow();
    if (row < 0){
        showErrorMsg(this, "Please select a contact to delete.");
        return;
    }
    pb.removebyindex(row);
    refreshtable();
}

// ==== FIND ====
void mainwindow::onfind(){
    QString q = searchfield->text().trimmed();
    if (q.isEmpty()) {
        showErrorMsg(this, "Please enter search text.");
        return;
    }
    auto res = pb.findbyname(q.toStdString());

    for (int i = 0; i < table->rowCount(); ++i)
        table->setRowHidden(i, true);
    for (auto idx : res)
        table->setRowHidden(static_cast<int>(idx), false);
}

// ==== RESET (показывает всех) ====
void mainwindow::onreset(){
    searchfield->clear();
    for (int i = 0; i < table->rowCount(); ++i)
        table->setRowHidden(i, false);
}

// ==== SAVE ====
void mainwindow::onsave(){
    pb.savetofile("phonebook.db");

    if (pgdb.isOpen()) {
        QSqlQuery q(pgdb);
        q.exec("DELETE FROM phones");
        q.exec("DELETE FROM contacts");

        for (const auto &c : pb.getAll()) {
            q.prepare(
                "INSERT INTO contacts(lastname,firstname,middlename,birthday,email,address) "
                "VALUES(?,?,?,?,?,?) RETURNING id");
            q.addBindValue(QString::fromStdString(c.lastname));
            q.addBindValue(QString::fromStdString(c.firstname));
            q.addBindValue(QString::fromStdString(c.middlename));
            q.addBindValue(QString::fromStdString(c.birthday));
            q.addBindValue(QString::fromStdString(c.email));
            q.addBindValue(QString::fromStdString(c.address));
            q.exec();
            q.next();
            int cid = q.value(0).toInt();

            for (const auto &p : c.phones) {
                QSqlQuery qp(pgdb);
                qp.prepare(
                    "INSERT INTO phones(contact_id,label,number) VALUES(?,?,?)");
                qp.addBindValue(cid);
                qp.addBindValue(QString::fromStdString(p.label));
                qp.addBindValue(QString::fromStdString(p.number));
                qp.exec();
            }
        }
    }

    QMessageBox::information(this,"Save",
        "Saved to phonebook.db and PostgreSQL successfully!");
}
