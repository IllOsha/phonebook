#include "contacts.h"
#include <sstream> // первый раз пробую, предоставляет классы для работы со строками как с потоками. Он позволяет вставлять данные в строку и извлекать их из нее, что удобно для форматирования, преобразования и анализа строковых данных.
#include <utility>

std:: atomic<size_t> contacts::created_count{0};
std:: atomic<size_t> contacts::copy_count{0};
std:: atomic<size_t> contacts::move_count{0};

contacts::contacts(const contacts& other){
    ++copy_count;
    firstname =other.firstname;
    lastname = other.lastname;
    middlename = other.middlename;
    address = other.address;
    birthday = other.birthday;
    email = other.email;
    phones = other.phones;
}

contacts::contacts(contacts && other) noexcept{
    ++move_count;
    firstname =std::move(other.firstname);
    lastname =std::move(other.lastname);
    middlename = std::move(other.middlename);
    address = std::move(other.address);
    birthday = std::move(other.birthday);
    email = std::move(other.email);
    phones = std::move(other.phones);
}

contacts & contacts::operator = (const contacts&other){
    if (this != &other){
        ++copy_count;
        firstname =other.firstname;
        lastname = other.lastname;
        middlename = other.middlename;
        address = other.address;
        birthday = other.birthday;
        email = other.email;
        phones = other.phones;
    }
    return *this;
}

contacts& contacts::operator=(contacts&& other) noexcept {
    if (this != &other) {
        ++move_count;
        firstname = std::move(other.firstname);
        lastname = std::move(other.lastname);
        middlename = std::move(other.middlename);
        address = std::move(other.address);
        birthday = std::move(other.birthday);
        email = std::move(other.email);
        phones = std::move(other.phones);
    }
    return *this;
}


std::string contacts::serialize() const {
    std::ostringstream ss;
    ss << "first:" << firstname << "\n";
    ss << "last:" << lastname << "\n";
    ss << "middle:" << middlename << "\n";
    ss << "addr:" << address << "\n";
    ss << "birth:" << birthday << "\n";
    ss << "email:" << email << "\n";
    for (size_t i = 0; i < phones.size(); ++i) {
        ss << "phone" << i << ":" << phones[i].label << "|" << phones[i].number << "\n";
    }
    ss << "----\n";
    return ss.str();
}

static std::string trim_local (const std::string& s){ // все это добро, чтобы не было лишних пробелов и адекватно вписывал значения
    size_t b = 0; while (b < s.size() && isspace((unsigned char)s[b])) ++b;
    size_t e = s.size(); while (e > b && isspace((unsigned char)s[e-1])) --e;
    return s.substr(b, e - b);
}

contacts contacts :: deserialize(const std::string& block){// 1 - ый контакс возвразает тип вызываемого значения, второй юудет указывает, что он принадлежит классу контактс
    contacts c;
    std::istringstream ss(block);
    std::string line;
    while(std::getline(ss, line)){
        if (line.empty()) continue;
        auto pos = line.find(':');// крч, пропуск пустых строк, если пустая, то переходит к итерации игонрируя ее
        if ( pos == std::string::npos) continue;
        std::string key = line.substr(0,pos);
        std:: string val = trim_local(line.substr(pos + 1));
        if (key == "first")c.firstname = val;
        else if (key == "last")c.lastname = val;
        else if (key == "middle") c.middlename = val;
        else if (key == "addr") c.address = val;
        else if (key == "email") c.email = val;
        else if (key == "birth") c.birthday = val;
        else if (key.rfind("phone", 0)==0){
            auto sep = val.find("|");
            PhoneNumber p;
            if (sep != std::string::npos){
                p.label = val.substr(0,sep);
                p.number = val.substr(sep + 1);
            }
            else{
                p.label = "";
                p.number = val;
            }
            c.phones.push_back(std::move(p));
            }
        }
    return c;
}
