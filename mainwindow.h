#pragma once
#include <QMainWindow> // я плакать буду, короче это для интерфейса
#include <QTableWidget>// таблица для отображения данных
#include <QPushButton>// ну тут кнопочки добавить
#include <QLineEdit>// поле для ввода текста
#include <QVBoxLayout>// крч для расположения элементов этот вертикально
#include <QHBoxLayout>// этот горизонтально
#include <QLabel>// текстовая метка тип <3

#include "phonebook.h"
#include "dbstorage.h"


class mainwindow : public QMainWindow{
    Q_OBJECT;// ало макрос для включения метаобъектов, типо как сигнал для динамических свойств и бла бла чтобы класс наследовался от QOBJECT
public:
    explicit mainwindow(QWidget *parent = nullptr); // explicit для неявных преобразований нужен, но вообще можно mainwindow w = nullptr, но как будто опасно будет, если будет не лень попробую
// * parent - короче родитльский виджет, который будет управлять памятью дочернного, ну короче база, чтобы автоматом управлять памятью
private slots:// ну слоты типо для обработки пользовательских действий
    void onadd();// ну тут понятно, кнопки добавляю
    void ondelete();
    void onfind();
    void onsave();
    void onreset();
    void onedit(); // edit contact

private:
    phonebook pb;
    DBStorage *db = nullptr;


    QTableWidget * table;// таблица для контактов ну и дальше понятно
    QLineEdit * searchfield;
    QPushButton * addBtn;
    QPushButton * delBtn;
    QPushButton * saveBtn;
    QPushButton * findBtn;
    QPushButton * resetBtn;
    QPushButton * editBtn;

    void refreshtable(); // для обновления таблицы

    // ===== Вспомогательные функции для ввода =====
    // defaultText - значение по умолчанию, allowEmpty позволяет пустое значение (для middlename)
    QString getValidName(const QString &title, bool allowEmpty = false, const QString &defaultText = QString());
    QString getValidEmail(const QString &title, const QString &defaultText = QString());
    QString getValidBirthday(const QString &title, const QString &defaultText = QString());
    QString getValidAddress(const QString &title, const QString &defaultText = QString());

};
