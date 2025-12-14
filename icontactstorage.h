#pragma once
#include "contacts.h"
#include <vector>

class IContactStorage
{
public:
    virtual ~IContactStorage() = default;

    virtual void saveAll(const std::vector<contacts>& contacts) = 0;
    virtual std::vector<contacts> loadAll() = 0;
};
