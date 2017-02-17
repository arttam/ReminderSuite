#ifndef SERVER_H
#define SERVER_H

#include <dlfcn.h>

#include <memory>
#include <boost/asio.hpp>
#include <signal.h>

#include "session.h"
#include "../../DataProvider/src/DBLibrary.h"

const char libName[] = "libDataProvider.so";

using boost::asio::ip::tcp;

// dlfcn helpers
void dlDeleter(void *dl);

class Server
{
    std::string rdbPath_;
    boost::asio::io_service io_service_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    boost::asio::signal_set signals_;

	std::string libraryPath_;
    DBLibrary *dlLibrary;
    std::unique_ptr<void, void (*)(void *)> upDL;
    std::function<destroy_t> f_destroy;

	bool setLibraryPath(const char *rdbType);

    void do_accept();
    void do_await_stop();

public:
    Server(unsigned short port, const char *rdbType, const char *rdbPath);
    ~Server();

    void run();
};

#endif // SERVER_H
