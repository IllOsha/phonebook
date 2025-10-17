// тут проверка данных, типо чтобы имена были норм, телефон также, а не набор букв
#pragma once
#include <string>
#include <regex> // умом поиск делает, чтобы не расписывать валидаторы, чтобы он находил сразу номера с цифр, эмэил @ буквы, домены и БЛА БЛА БЛА

namespace Validators{
// тут уборка лишних символов в конце и в начале строки и т.д
inline std::string trim(const std::string& s){
    size_t b = 0;
    while (b < s.size() && isspace ((unsigned char)s[b])) ++b;// ispace - проверка является символ пробельным символом или нет.
    size_t c = size_t();
    while (c > b && isspace ((unsigned char)s[c-1])) --c;

    return s.substr(b,c-b);// вырезает часть строки ( начало, длина строки)
}

inline bool validname (const std::string& s){
    auto t = trim(s);
    if (t.empty()) return false;
    static const std::regex re("^[A-Za-zА-Яа-яЁё][A-Za-zА-Яа-яЁё0-9\\- ]*[A-Za-zА-Яа-яЁё0-9]$");
    return std::regex_match(t, re) && t.front() != '-' && t.back() != '-';
}
inline bool validphone(const std::string& s){
    std::string t;// здесь будут очищенные строки
    for (char d : s){
        if (d!= ' ' && d != '(' && d!= ')' && d != '-') t.push_back(d);
    }
    static const std::regex re(R"(^(?:\+7|8)\d{10}$)");
    return std::regex_match(t,re);
}
inline bool valiemail (const std::string& s){
    auto t = trim(s);
    static const std::regex re(R"(^[A-Za-z0-9]+(?:[._%+-][A-Za-z0-9]+)*@[A-Za-z0-9]+(?:\.[A-Za-z0-9]+)+$)");
    return std:: regex_match(t,re);
}
}
