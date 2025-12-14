#include "phonebook.h"
#include "contacts.h"
#include <fstream>
#include <iostream>


bool phonebook::loadfromfile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;

    contacts_.clear();
    std::string line, block;

    while (std::getline(in, line)) {
        if (line == "----") {
            if (!block.empty()) {
                contacts_.push_back(contacts::deserialize(block));
                block.clear();
            }
        } else {
            block += line + "\n";
        }
    }

    if (!block.empty()) {
        contacts_.push_back(contacts::deserialize(block));
    }

    return true;
}

bool phonebook::savetofile(const std::string& filename) const {  // ← исправлено имя
    std::ofstream out(filename, std::ios::trunc);
    if (!out) return false;

    for (const auto& c : contacts_) {
        out << c.serialize();
    }

    return true;
}

void phonebook::addcontact(contacts c) {
    contacts_.push_back(std::move(c));
}

bool phonebook::removebyindex(size_t idx) {
    if (idx >= contacts_.size()) return false;
    contacts_.erase(contacts_.begin() + idx);
    return true;
}

std::vector<size_t> phonebook::findbyname(const std::string& query) const {
    std::vector<size_t> result;
    for (size_t i = 0; i < contacts_.size(); ++i) {
        const auto& c = contacts_[i];
        if (c.firstname.find(query) != std::string::npos ||
            c.lastname.find(query) != std::string::npos ||
            c.middlename.find(query) != std::string::npos) {
            result.push_back(i);
        }
    }
    return result;
}

void phonebook::listall() const {
    if (contacts_.empty()) {
        std::cout << "(no contacts)\n";
        return;
    }

    for (size_t i = 0; i < contacts_.size(); ++i) {
        const auto& c = contacts_[i];
        std::cout << i << ": " << c.lastname << " " << c.firstname
                  << " " << c.middlename << "\n"
                  << "   Email: " << c.email << "\n"
                  << "   Birthday: " << c.birthday << "\n"
                  << "   Address: " << c.address << "\n";
        for (const auto& p : c.phones)
            std::cout << "   Phone [" << p.label << "]: " << p.number << "\n";
        std::cout << "----\n";
    }
}
