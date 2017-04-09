#include "DBSQLite.h"
#include <algorithm>
#include <iterator>
#include <sstream>

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

	auto _populator = [](void* pDest, int colNum, char **rowData, char **) {
		std::vector<std::string> *pResp = reinterpret_cast< std::vector<std::string>* >(pDest);

		std::string _row;
		for(int colNr=0; colNr < colNum; ++colNr) {
			_row.append(rowData[colNr]).append(":");
		}
		pResp->push_back(std::move(_row));

		return 0;
	};

	char *_exeError = nullptr;
	dbRC_ = sqlite3_exec(
			worker_.get(), 
			"select * from reminders", 
			_populator,
			&_response,
			&_exeError
	);

	if (dbRC_) {
		std::cerr << "Failed to exec SQL: "
			<< _exeError
			<< std::endl;

	}
	return _response;
}

std::string DBSQLite::get(const std::string name, const std::string field)
{
	std::string _response;

	std::string stmt{"select "};
	stmt.append(field.empty() ? "*" : field)
		.append(" from reminders where name='")
		.append(name)
		.append("';");

	char *_execError = nullptr;
	dbRC_ = sqlite3_exec(
			worker_.get(), 
			stmt.c_str(), 
			[](void *resPtr, int colNum, char **rowData, char **rowColumns) {
				std::string *result = reinterpret_cast<std::string*>(resPtr);
				for(int colNr=0; colNr < colNum; ++colNr) 
					result->append(rowData[colNr]).append(":");

				return 0;
			}, 
			&_response, 
			&_execError
	);

	sqlite3_free(reinterpret_cast<void*>(_execError));

	return _response;
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
		std::cerr << "Cannot open source database" << std::endl;
		return false;
	}

	// Create 'in-memory' worker
	sqlite3* _inMemory;
	dbRC_ = sqlite3_open(":memory:", &_inMemory);
	if (dbRC_) {
		// TODO: Error message handling here
		std::cerr << "Cannot open in-memory database" << std::endl;
		return false;
	}
	worker_.reset(_inMemory);

	// Backing up file-system db into memory one
	sqlite3_backup *pBackup = sqlite3_backup_init(worker_.get(), "main", db_.get(), "main");
	if (pBackup) {
		sqlite3_backup_step(pBackup, -1);
		sqlite3_backup_finish(pBackup);
	}
	dbRC_ = sqlite3_errcode(worker_.get());
	if (dbRC_) {
		// TODO: Error message handling here
		std::cerr << "Failed to create in-memory backup: "  << dbRC_ << std::endl;
		return false;
	}
	// Closing file-system database
	sqlite3_close(db_.get());

	// Reading DB into memory
	/*
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
			worker_.get(), 
			"select * from reminders", 
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
	*/

	return true;
}

bool DBSQLite::save() 
{
	if (!openDB()) {
		// TODO: Error message here
		std::cerr << "Cannot open source database" << std::endl;
		return false;
	}

	sqlite3_backup *pBackup = sqlite3_backup_init(db_.get(), "main", worker_.get(), "main");
	if (pBackup) {
		sqlite3_backup_step(pBackup, -1);
		sqlite3_backup_finish(pBackup);
	}
	dbRC_ = sqlite3_errcode(db_.get());
	if (dbRC_) {
		// TODO: Error message handling here
		std::cerr << "Failed to create file-system backup: "  << dbRC_ << std::endl;
		return false;
	}

	return true;
}
