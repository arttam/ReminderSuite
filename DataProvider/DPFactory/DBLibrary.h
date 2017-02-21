//
// Created by art on 28/12/16.
//

#ifndef FIRST_TEST_DBLIBRARY_H
#define FIRST_TEST_DBLIBRARY_H
#include <string>
#include <vector>

class DBLibrary {
public:
    DBLibrary() = default;
    virtual ~DBLibrary() = default;

    virtual std::string fields() = 0;
    virtual std::vector<std::string> get() = 0;
    virtual std::string get(const std::string name, const std::string field = {}) = 0;
    virtual bool set(const std::string name, const std::string csv_values) = 0;
    virtual bool set(const std::string name, const std::string field, const std::string value) = 0;
    virtual bool erase(const std::string name) = 0;

    virtual bool read() = 0;
    virtual bool save() = 0;
};

using create_t = DBLibrary* (const std::string, const std::string);
using destroy_t = void (DBLibrary*);
#endif //FIRST_TEST_DBLIBRARY_H
