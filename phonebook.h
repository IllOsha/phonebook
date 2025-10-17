#pragma once
#include "contacts.h"
#include <vector>
#include <string>

class phonebook {
    std::vector<contacts> contacts_;
public:
    bool loadfromfile(const std::string &filename);
    bool savetofile(const std::string &filename) const;

    void addcontact(contacts c);
    bool removebyindex(size_t idx);
    std::vector<size_t> findbyname(const std::string &query) const;
    void listall() const;

    size_t size() const { return contacts_.size(); }
    const contacts &at(size_t idx) const { return contacts_.at(idx); }
    contacts &at(size_t idx) { return contacts_.at(idx); }
};
