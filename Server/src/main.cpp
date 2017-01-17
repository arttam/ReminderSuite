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

constexpr char serverDaemonName[] = "ndp_daemon";

int main(int argc, char* argv[])
{
    if (argc < 4) {
        std::cerr << "Usage: ndp <port> <dataProviderType> <dataPath>\n";
        exit(EXIT_FAILURE);
    }

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
        Server s(std::atoi(argv[1]), argv[2], argv[3]);
        s.run();
    }
    catch (std::exception& e) {
        syslog(LOG_ERR, "Exception thrown during server session");
        syslog(LOG_ERR, e.what());
    }
    syslog(LOG_NOTICE, "Server job done, leaving");
    closelog();

    return 0;
}

