//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "server.hpp"

// Program Options support
#include <fstream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

// Global configuration
#include "config.h"
DataProviderConf dataProviderConf;

int main(int argc, char** argv)
{
	unsigned short port;
	std::string host;
	std::string docsRoot;
	std::string configFile;

	// Data provider details
	unsigned short dataPort;
	std::string dataHost;
	

	po::options_description server_config("Data Provider options");
	server_config.add_options()
		("help,H", "Usage: reminder-http <address> <port> <doc_root>\n")
		("config,C", po::value<std::string>(&configFile)->default_value("reminder-http.ini"), "Configuration file")
		("port,p", po::value<unsigned short>(&port), "HTTP server port")
		("host,h", po::value<std::string>(&host)->default_value("localhost"), "HTTP sever address")
		("root,R", po::value<std::string>(&docsRoot), "Site root folder")
		("datahost,D", po::value<std::string>(&dataHost), "Data Provider address")
		("dataport,P", po::value<unsigned short>(&dataPort), "Data Provider communication port")
	;
	
	po::variables_map config_vars;
	po::store(po::parse_command_line(argc, argv, server_config), config_vars);
	po::notify(config_vars);

	std::ifstream config(configFile.c_str());
	if (config) {
		po::store(po::parse_config_file(config, server_config), config_vars);
		po::notify(config_vars);
	}
	
	if (config_vars.count("help")) {
		std::cout << server_config << std::endl;
		return 0;
	}

	if (port == 0 || dataPort == 0 || dataHost.empty() || host.empty() || docsRoot.empty()) {
		std::cout << "Not enough parameters to start reminder http server" << std::endl << std::endl;
		std::cout << server_config << std::endl;
		return 0;
	}

	dataProviderConf.port = dataPort;
	dataProviderConf.host = dataHost;

	try
	{
		// Initialise the server.
		http::server::server s(host, std::to_string(port), docsRoot);

		// Run the server until stopped.
		s.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}
