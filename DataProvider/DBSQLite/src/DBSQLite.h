#ifndef __USES_DBSQLITE_H__
#define __USES_DBSQLITE_H__

#include "../../DPInterface.h"
#include <string>
#include <vector>
#include <memory>
#include <sqlite3.h>

// While debugging
#include <iostream>
#include <fstream>

auto dbDeleter = [](sqlite3 *db) { sqlite3_close(db); };

class DBSQLite: public DBLibrary
{
	int dbRC_;
	std::string dbPath_;
	std::vector<std::string> fields_;
	std::vector<std::vector<std::string> > data_;

	using dbHandler = std::unique_ptr<sqlite3, decltype(dbDeleter)>;
	dbHandler  db_;
	dbHandler  worker_;

	bool openDB();

public:
    DBSQLite(const std::string dbPath)
		: dbPath_{dbPath}
		, db_{nullptr, dbDeleter}
		, worker_{nullptr, dbDeleter}
	{}

    virtual ~DBSQLite() {}

    std::string fields() override;
    std::vector<std::string> get() override;
    std::string get(const std::string name, const std::string field) override;
    bool set(const std::string name, const std::string csv_values) override;
    bool set(const std::string name, const std::string field, const std::string value) override;
    bool erase(const std::string name) override;
    bool read() override;
    bool save() override;

};

#endif
