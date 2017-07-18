#include <regex>
#include <algorithm>
#include <iterator>

#include "entry.h"

std::ostream& operator<<(std::ostream& os, const Entry& entry)
{
    std::copy(entry.values_.begin(), entry.values_.end(), std::ostream_iterator<std::string>(os, ":"));
    return os;
}

Entry::Entry(const std::string& line)
{
    std::regex _colon(":");

    std::sregex_token_iterator _sBegin(line.begin(), line.end(), _colon, -1);
    std::sregex_token_iterator _sEnd;

    std::copy(_sBegin, _sEnd, std::back_inserter(values_));
}

std::string& Entry::getValue(size_t idx)
{
    // Error checks goes here ????
    return values_[idx];
}

bool Entry::setEntry(const Entry& entry)
{
    values_ = entry.values_;
    return true;
}

bool Entry::setValue(size_t idx, const std::string& value)
{
    if (idx >= values_.size())
        return false;

    values_[idx] = value;
    return true;
}

void Entry::operator>>(std::vector<char>& dest)
{
    for(const auto &value: values_) {
        std::copy(value.begin(), value.end(), std::back_inserter(dest));
        dest.push_back(':');
    }
}

std::string Entry::asString() {
    std::ostringstream _oss;
    std::copy(values_.begin(), values_.end(), std::ostream_iterator<std::string>(_oss, ":"));
    return std::string{_oss.str()};
}
