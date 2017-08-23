#include "rdb_handler.hpp"
#include "../../ClientLib/src/library.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <sstream>
#include <regex>

// DEBUG only
#include <iostream>

// to fill json quirky way using accumulate algorithm
#include <numeric>

// json support
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace pt = boost::property_tree;

// Configuration 
#include "config.h"

extern DataProviderConf dataProviderConf;


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
	std::unique_ptr<DataClient, std::function<void(DataClient*)> > pDC(
			createClient(dataProviderConf.host.c_str(), 
			std::to_string(dataProviderConf.port).c_str()),
			[](DataClient* p) { destroyClient(p); } 
	);

	if (!pDC) {
		std::cout << "Failed to obtain DataClient via createClient call" << std::endl;
		return false;
	}

	std::regex _colon(":");
	std::sregex_token_iterator _sEnd;

	std::string _param(request.substr(4));

	// Do it properly with some kind of caching
	std::vector<std::string> _fields;

	if (_param == "/fields" || _param.substr(0, 4) == "/get") {
		std::vector<std::string> _fieldsCsv;
		if (!pDC->executeCmd("/fields", _fieldsCsv))
			return false;

		std::sregex_token_iterator _sBegin(_fieldsCsv.front().begin(), _fieldsCsv.front().end(), _colon, -1);
		std::copy(_sBegin, _sEnd, std::back_inserter(_fields));
	}

	if (_param == "/fields") {
		pt::ptree _children;

		std::accumulate(_fields.begin(), _fields.end(), std::string{""},
			[&_children](const std::string& /*ignoredAccumulatedValue*/, const std::string& field) {
				_children.push_back(pt::ptree::value_type("", pt::ptree(field)));
				// Must return something
				return field; 
			}
		);

		pt::ptree _output;
		_output.add_child("fields", _children);
		std::ostringstream _oss;
		pt::write_json(_oss, _output, true);
		reply_.assign(_oss.str());

		return true;
	}

	std::vector<std::string> _result;
	if (!pDC->executeCmd(_param, _result)) {
		return false;
	}

	if (_param.substr(0, 4) == "/get") {
		pt::ptree _children;

		std::accumulate(_result.begin(), _result.end(), std::string{""}, 
			[&_children, &_colon, &_fields, &_sEnd](const std::string&, const std::string& row) {
				// Skip empty row, usually last one, for human readability of server output
				if (!row.empty()) {
					std::sregex_token_iterator _sBegin(row.begin(), row.end(), _colon, -1);
					std::vector<std::string>::const_iterator _fNameIt = _fields.begin();

					pt::ptree _rowNode;
					std::accumulate(_sBegin, _sEnd, std::string{""}, 
						[&_rowNode, &_fNameIt](const std::string&, const std::string value) {
							_rowNode.put(*_fNameIt++, value);
							return value;
						}
					);
					_children.push_back(std::make_pair("", _rowNode));
				}

				return row;
			}
		);

		pt::ptree _output;
		_output.add_child("reminders", _children);
		std::ostringstream _oss;
		pt::write_json(_oss, _output, true);
		reply_.assign(_oss.str());

		return true;
	}

	std::stringstream _sstr;
	std::copy(_result.begin(), _result.end(), std::ostream_iterator<std::string>(_sstr, "\r\n"));
	reply_.assign(_sstr.str());

	return true;
}

const std::string& rdb_handler::getResponse() const
{
	return reply_;
}


