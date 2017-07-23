#include "rdb_handler.hpp"
#include "../../ClientLib/src/library.h"
#include <vector>
#include <memory>
#include <algorithm>
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
	std::string _param(request.substr(5));
	if ( commands_.end() == 
		 std::find_if( 
			commands_.begin(), 
			commands_.end(), 
			[&_param] (const std::string& cmd) { return std::equal(cmd.begin(), cmd.end(), _param.begin()); }) ) 
	{
		std::cerr << "Command '" << _param << "' not supported" << std::endl;
		return false;
	}
	return true;
}

bool rdb_handler::parse(const std::string& request)
{
	std::unique_ptr<DataClient, std::function<void(DataClient*)> > pDC(createClient("localhost", "9000"),[](DataClient* p) { destroyClient(p); } );
	if (!pDC) {
		std::cout << "Failed to obtain DataClient via createClient call" << std::endl;
		return false;
	}

	std::string _param(request.substr(4));
	std::vector<std::string> _result;
	if (!pDC->executeCmd(_param, _result)) {
		return false;
	}

	for(const auto &resLine: _result)
		reply_.append(resLine).append("\r\n");

	return true;
}

const std::string& rdb_handler::getResponse() const
{
	return reply_;
}


