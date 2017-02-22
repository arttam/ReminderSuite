#include "DBSQLite.h"
#include <algorithm>
#include <iterator>

extern "C" DBLibrary *create(const std::string dbPath) {
	return dynamic_cast<DBLibrary*>( new DBSQLite(dbPath) );
}

extern "C" void destroy(DBLibrary* dbl) {
    delete dbl;
}

bool DBSQLite::openDB()
{
	sqlite3 *_db;
	dbRC_ = sqlite3_open(dbPath_.c_str(), &_db);
	if (dbRC_) {
		return false;
	}

	db_.reset(_db);
	return true;
}

std::string DBSQLite::fields() 
{
	std::string _response{""};
	
	for(auto &fieldName: fields_)
		_response.append(fieldName).append(":");

	return _response;
}

std::vector<std::string> DBSQLite::get() 
{
	std::vector<std::string> _response;

	for(auto &row : data_) {
		std::string _row{""};
		for(auto &field: row) {
			_row.append(field).append(":");
		}
		_response.push_back(_row);
	}


	return _response;
}

std::string DBSQLite::get(const std::string name, const std::string field)
{
	return std::string{""};
}

bool DBSQLite::set(const std::string name, const std::string csv_values) 
{
	return false;
}

bool DBSQLite::set(const std::string name, const std::string field, const std::string value) 
{
	return false;
}

bool DBSQLite::erase(const std::string name) 
{
	return false;
}

bool DBSQLite::read() 
{
	if (!openDB()) {
		// TODO: Error message here
		return false;
	}
	// Reading DB into memory
	
	auto _populator = [](void* pThis, int colNum, char **rowData, char **rowColumns) {
		DBSQLite* self = reinterpret_cast<DBSQLite*>(pThis);
		if (self->fields_.empty()) {
			for(int colNr=0; colNr < colNum; ++colNr) {
				self->fields_.push_back(rowColumns[colNr]);
			}
		}

		std::vector<std::string> _row;
		for(int colNr=0; colNr < colNum; ++colNr) {
			_row.push_back(rowData[colNr]);
		}
		self->data_.push_back(std::move(_row));

		return 0;
	};

	char *_exeError = nullptr;
	dbRC_ = sqlite3_exec(
		db_.get(), 
		"select * from reminder", 
		_populator,
		this,
		&_exeError
	);

	if (dbRC_) {
		std::cerr << "Failed to exec SQL: "
			<< _exeError
			<< std::endl;

		return false;
	}

	return true;
}

bool DBSQLite::save() 
{
	return false;
}
