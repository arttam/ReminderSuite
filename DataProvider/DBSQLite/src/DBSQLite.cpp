#include "DBSQLite.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <regex>

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

bool DBSQLite::getFields()
{
	char *_exeError = nullptr;

	dbRC_ = sqlite3_exec(
			worker_.get(), 
			"pragma table_info('reminders');", 
			[](void *pDest, int, char **rowData, char **) { 
				reinterpret_cast<std::vector<std::string>* >(pDest)->push_back(rowData[1]);
				return 0; 
			},
			&fields_, 
			&_exeError
	);

	if (dbRC_) {
		std::cerr << "Failed to get fields: " << _exeError << std::endl;
		sqlite3_free(_exeError);
		return false;
	}

	return true;
}

std::string DBSQLite::fields() 
{
	if (fieldsReady_.get()) {
		std::ostringstream _ostr;
		std::copy(fields_.begin(), fields_.end(), std::ostream_iterator<std::string>(_ostr, ":"));

		return _ostr.str();
	}

	return std::string();
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

		sqlite3_free(_exeError);
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
	std::vector<std::string> _fieldValues;
	std::regex _colon(":");

	std::sregex_token_iterator _sBegin(csv_values.begin(), csv_values.end(), _colon, -1);
	std::sregex_token_iterator _sEnd;

	std::copy(_sBegin, _sEnd, std::back_inserter(_fieldValues));
	// Sanity check, now only on count of field values ...
	if (!fields_.empty()) {
		if (fields_.size() != _fieldValues.size()) {
			std::cerr << "Fields count does not match tables one, input: '" << csv_values << "'" << std::endl;
			return false;
		}
	}


	std::stringstream _ssSQL;
	if (alreadyExists(name)) {
		_ssSQL << "update reminders set ";
	}
	else {
		_ssSQL << "insert into reminders values ('";
		std::copy(_fieldValues.begin(), _fieldValues.end(), std::ostream_iterator<std::string>(_ssSQL, "','"));
	}

	std::string _setSQL(_ssSQL.str());
	_setSQL.replace(_setSQL.length() - 2, 2, ");");

	sqlite3_stmt *_stmt;
	const char *_stmtTail;

	dbRC_ = sqlite3_prepare(
		worker_.get(),
		_setSQL.c_str(),
		_setSQL.size(),
		&_stmt,
		&_stmtTail
	);
	if (dbRC_ != SQLITE_OK) {
		std::cerr << "Failed to prepare insert statement" << std::endl
			<< _setSQL << std::endl;
		return false;
	}

	dbRC_ = sqlite3_step(_stmt);
	if (dbRC_ == SQLITE_OK || dbRC_ == SQLITE_DONE) {
		sqlite3_finalize(_stmt);
		std::cerr << "Insert successful " << std::endl;
		return true;
	}

	switch(dbRC_) {
		case SQLITE_BUSY:
			std::cerr << "Database is busy, leaving" << std::endl;
			break;
		case SQLITE_ERROR:
			std::cerr << "Error inserting new record" << std::endl;
			break;
		case SQLITE_MISUSE:
			std::cerr << "SQL statement missuse" << std::endl;
			break;
		default:
			std::cerr << "SQL error: " << dbRC_ << std::endl;
			break;
	}
	std::cerr << "Failed while trying: " << std::endl
		<< _setSQL << std::endl;

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

	// initiating async field read
	fieldsReady_ = std::future<bool>( std::async(std::launch::async, [this](){ return getFields(); }) );

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

bool DBSQLite::alreadyExists(const std::string& name) const
{
	std::string stmt{"select count(name) from reminders where name='"};
	stmt.append(name).append("';'");

	std::string _result;
	char *_exeError = nullptr;
	dbRC_ = sqlite3_exec(
			worker_.get(),
			stmt.c_str(),
			[](void *resPtr, int colNum, char **rowData, char **rowColumns) {
				std::string *sPtr = reinterpret_cast<std::string*>(resPtr);
				sPtr->append(rowData[0]);

				return 0;
			},
			&_result,
			&_exeError
	);

	sqlite3_free(reinterpret_cast<void*>(_exeError));

	return (_result.compare("0") != 0);
}
