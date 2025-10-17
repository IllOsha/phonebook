#pragma once
#include <string>
#include <vector>
#include <atomic>

struct PhoneNumber{
    std::string label; //сюда короче адрес, работу и прочие штуки
    std::string number;
};

class contacts{
public:
    static std::atomic<size_t> created_count;// крч, без статика смысла нет, а то без него у каждого был бы свой счетчик, но нам надо, чтобы он был общим, если правиольно понял
    static std::atomic<size_t> copy_count;
    static std::atomic<size_t> move_count;

    std::string firstname;
    std::string lastname;
    std::string middlename;
    std::string address;
    std::string birthday;
    std::string email;

    std::vector<PhoneNumber> phones;

    void* operator new(size_t sc){ // sc - size contacts
        ++created_count;
        return::operator new(sc);
    }

    void operator delete(void* p) noexcept { ::operator delete(p); }//noexcept - типо спецификатор, который указывает компилятору, что функция гарантированно не будет генерировать исключений
    contacts() = default;
    ~contacts() = default;

    contacts(const contacts & other);
    contacts(contacts && other) noexcept; //contacts && - rvalue ссылка на контакты
    contacts & operator = (const contacts & other);
    contacts & operator = (contacts && other) noexcept;

    std::string serialize() const;
    static contacts deserialize(const std::string& block);

};
