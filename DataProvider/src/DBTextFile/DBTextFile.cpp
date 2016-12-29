//
// Created by art on 28/12/16.
//
#include <algorithm>
#include <regex>
#include "DBTextFile.h"
#include "filehandler.h"

std::string DBTextFile::fields() {
    std::ostringstream _oss;
    std::copy(fields_.begin(), fields_.end(), std::ostream_iterator<std::string>(_oss, ":"));

    std::string _reply{_oss.str()};
    return _reply;
}

std::vector<std::string> DBTextFile::get() {
    std::vector<std::string> _response(entries_.size());

    std::transform(entries_.begin(), entries_.end(), _response.begin(), [](Entry& entry){ return entry.asString(); });

    return _response;
}

std::string DBTextFile::get(const std::string name, const std::string field) {
    std::vector<Entry>::iterator _findNameIt = std::find_if(entries_.begin(), entries_.end(), [&name](Entry& entry){ return (entry.getValue(0).compare(name) == 0); });
    if (_findNameIt == entries_.end()) {
        return std::string{""};
    }
    Entry& _entry = *_findNameIt;

    if (field.empty()) {
        return _entry.asString();
    }
    else {
        std::vector<std::string>::iterator _findFieldIt = std::find(fields_.begin(), fields_.end(), field);
        if (_findFieldIt == fields_.end()) {
            return std::string{""};
        }

        return _entry.getValue(static_cast<size_t>(std::distance(fields_.begin(), _findFieldIt)));
    }
}

bool DBTextFile::set(const std::string name, const std::string csv_values) {

    std::vector<Entry>::iterator _findNameIt = std::find_if(entries_.begin(), entries_.end(),
                [&name](Entry& element){ return (element.getValue(0).compare(name) == 0); });

    if (_findNameIt == entries_.end()) {
        entries_.emplace_back(csv_values);
        return true;
    }
    _findNameIt->setEntry(Entry(csv_values));
    return true;
}

bool DBTextFile::set(const std::string name, const std::string field, const std::string value) {
    std::vector<Entry>::iterator _findNameIt = std::find_if(entries_.begin(), entries_.end(),
                [&name](Entry& element){ return (element.getValue(0).compare(name) == 0); });

    if (_findNameIt == entries_.end()) {
        return false;
    }

    std::vector<std::string>::iterator _findFieldIt = std::find(fields_.begin(), fields_.end(), field);
    if (_findFieldIt == fields_.end()) {
        return false;
    }

    return _findNameIt->setValue(static_cast<size_t>(std::distance(fields_.begin(), _findFieldIt)), value);
}

bool DBTextFile::erase(const std::string name) {
    std::vector<Entry>::iterator _findNameIt = std::find_if(entries_.begin(), entries_.end(),
                [&name](Entry& entry){ return (entry.getValue(0).compare(name) == 0); });

    if (_findNameIt == entries_.end()) {
        return false;
    }

    entries_.erase(_findNameIt);
    return true;
}

bool DBTextFile::read() {
    fileHandler _fileHandler(dbPath_.c_str());
    if (!_fileHandler)
        return false;

    std::string _contents{ _fileHandler.getContents() };

    std::regex _rEOL("\\n");
    std::sregex_token_iterator _dLine(_contents.begin(), _contents.end(), _rEOL, -1);

    // First line - fields names
    Entry _fields(*_dLine++);
    fields_ = _fields.getValues();
    while (_dLine != std::sregex_token_iterator()) {
        entries_.emplace_back(*_dLine++);
    }
    return true;
}

bool DBTextFile::save() {
    fileHandler _fileHandler(dbPath_.c_str(), true);
    if (!_fileHandler)
        return false;

    std::ostream& os = _fileHandler.getStream();

    std::copy(fields_.begin(), fields_.end(), std::ostream_iterator<std::string>(os, ":"));
    os << std::endl;
    std::copy(entries_.begin(), entries_.end(), std::ostream_iterator<Entry>(os, "\n"));

    return false;
}
