//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>

#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include "server.h"

// Program Options support
#include <fstream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

constexpr char serverDaemonName[] = "ndp_daemon";

int main(int argc, char* argv[])
{
	unsigned short port = 9000;
	std::string dpType;
	std::string dataPath;
	std::string configFile{"reminder-dp.ini"};
	bool isDebug = false;

	po::options_description data_provider_config("Data Provider options");
	data_provider_config.add_options()
		("help,H", "You need: port, provider_type and data_path\nEither by command line or config file\n")
		("port,p", po::value<unsigned short>(&port), "Communication port")
		("type,T", po::value<std::string>(&dpType), "Data Provider type")
		("path,P", po::value<std::string>(&dataPath), "Data location path")
		("debug,D", po::value<bool>(&isDebug)->default_value(false), "Is instance invoked in debug mode")
		("config,C", po::value<std::string>(&configFile), "Configuration file")
	;
	
	po::variables_map config_vars;
	po::store(po::parse_command_line(argc, argv, data_provider_config), config_vars);
	po::notify(config_vars);

	std::ifstream configIni(configFile.c_str());
	if (configIni) {
		po::store(po::parse_config_file(configIni, data_provider_config), config_vars);
		po::notify(config_vars);
	}
	
	if (config_vars.count("help")) {
		std::cout << data_provider_config << std::endl;
		return 0;
	}

	if (port <=0 || dpType.empty() || dataPath.empty()) {
		std::cout << "Not enough parameters to start server" << std::endl << std::endl;
		std::cout << data_provider_config << std::endl;
		return 0;
	}

	// Just want to debug, no need for daemon
	if (isDebug) {
		try {
			Server s(port, dpType, dataPath);
			s.run();
		}
		catch (std::exception& e) {
			std::cerr << "Exception thrown during server session" << std::endl;
			std::cerr << e.what() << std::endl;
		}
		std::cerr << "Server job done, leaving" << std::endl;

		return 0;
	}

	// Run as daemon
    pid_t parentPID = fork();
    if (parentPID < 0) {
        std::cerr << "Failed to fork from parent process, leaving" << std::endl;
        exit(EXIT_FAILURE);
    }

    // We got good parent PID, let's close parent process
    if (parentPID > 0) {
        std::cerr << "Parent process done it's job, leaving it" << std::endl;
        exit(EXIT_SUCCESS);
    }

    std::cout << "Process was daemonized, for errors check journalctl -xe" << std::endl;

    // Now we are dealing with child process

    // Change file mask
    umask(0);

    // Opening log
    setlogmask(LOG_UPTO(LOG_NOTICE));
    openlog(serverDaemonName, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID | LOG_INFO, LOG_USER);

    // Get child's (daemon) PID
    pid_t daemonPID = setsid();
    if (daemonPID < 0) {
        syslog(LOG_ERR, "Failed to obtain child PID, leaving");
        closelog();

        exit(EXIT_FAILURE);
    }

    /*
    //Change Directory
    //If we cant find the directory we exit with failure.
    if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }
     */

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Actual daemon process
    syslog(LOG_NOTICE, "Starting server");

    try {
        Server s(port, dpType, dataPath);
        s.run();
    }
    catch (std::exception& e) {
        syslog(LOG_ERR, "Exception thrown during server session: %s", e.what());
    }
    syslog(LOG_NOTICE, "Server job done, leaving");
    closelog();

    return 0;
}

