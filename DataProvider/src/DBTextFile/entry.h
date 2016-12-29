#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <vector>

class Entry
{
    std::vector<std::string> values_;

public:
    Entry() = default;
    Entry(const std::string& line);

    size_t getCount() const { return values_.size(); }
    std::vector<std::string>& getValues() { return values_; }
    std::string asString();

    std::string& getValue(size_t idx);
    bool setEntry(const Entry& entry);
    bool setValue(size_t idx, const std::string& value);

    friend std::ostream& operator<<(std::ostream& os, const Entry& entry);
    void operator>>(std::vector<char>&);
};

std::ostream& operator<<(std::ostream& os, const Entry& entry);



#endif // ENTRY_H
