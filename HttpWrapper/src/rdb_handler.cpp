#include "rdb_handler.hpp"
#include "../../ClientLib/src/library.h"
#include <vector>
#include <memory>
// DEBUG only
#include <iostream>

const std::set<std::string> rdb_handler::commands_{"get", "set", "delete", "fields", "commit"};

bool rdb_handler::isRDBcall(const std::string& request)
{
	if (request.compare(0, 5, "/rdb/") == 0) {
		return true;
	}
	return false;
}

bool rdb_handler::isValidCall(const std::string& request)
{
	for (const auto &command: rdb_handler::commands_) {
		if (request.compare(5, command.length(), command) == 0) 
			return true;
	}
	return false;
}

bool rdb_handler::parse(const std::string& request)
{
	std::unique_ptr<DataClient, std::function<void(DataClient*)> > pDC(createClient("localhost", "9000"),[](DataClient* p) { destroyClient(p); } );
	if (!pDC) {
		std::cout << "Failed to obtain DataClient via createClient call" << std::endl;
		return false;
	}

	std::string _param;
	std::vector<std::string> _result;

	if (request.compare(5, 3, "get") == 0) {
		_param.assign(request.substr(4));

		if (!pDC->executeCmd(_param, _result)) {
			return false;	
		}

		for(const auto &resLine: _result)
			reply_.append(resLine).append("\r\n");

		return true;
	}
	else if (request.compare(5, 3, "set") == 0) {
		_param.assign(request.substr(4)); 
		reply_.assign("SET:");
		return true;
	}
	else if (request.compare(5,6, "fields") == 0) {
		_param.assign(request.substr(4));

		if (!pDC->executeCmd(_param, _result)) {
			return false;	
		}

		for(const auto &resLine: _result)
			reply_.append(resLine).append("\r\n");

		return true;
	}
	return false;
}

const std::string& rdb_handler::getResponse() const
{
	return reply_;
}


