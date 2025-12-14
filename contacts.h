#pragma once
#include <string>
#include <vector>
#include <atomic>

struct PhoneNumber{
    std::string label;
    std::string number;
};

class contacts{
public:
    static std::atomic<size_t> created_count;
    static std::atomic<size_t> copy_count;
    static std::atomic<size_t> move_count;

    std::string firstname;
    std::string lastname;
    std::string middlename;
    std::string address;
    std::string birthday;
    std::string email;

    std::vector<PhoneNumber> phones;

    contacts();
    ~contacts();

    contacts(const contacts & other);
    contacts(contacts && other) noexcept;
    contacts & operator=(const contacts & other);
    contacts & operator=(contacts && other) noexcept;

    std::string serialize() const;
    static contacts deserialize(const std::string& block);
};

