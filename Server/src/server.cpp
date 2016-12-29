#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

#include "server.h"

using boost::asio::ip::tcp;

void dlDeleter(void *dl) {
    if (dl) {
        dlclose(dl);
    }
}

template<typename T>
std::function<T> loadFunction(const char *fName, const char *errPtr, void *pHandle) {
    std::function<T> _f = reinterpret_cast<T*>(dlsym(pHandle, fName));
    errPtr = dlerror();
    if (errPtr || !_f) {
        std::cerr << "Cannot load symbol '" << fName << "': " << errPtr << std::endl;
    }
    return _f;
}

Server::Server(unsigned short port, const char *rdbType, const char *rdbPath)
    : rdbPath_(rdbPath)
    , io_service_()
    , acceptor_(io_service_, tcp::endpoint(tcp::v4(), port))
    , socket_(io_service_)
    , signals_(io_service_)
    , upDL(nullptr, dlDeleter)
{

    //std::unique_ptr<void, decltype(dlDeleter)> upDL;
    upDL = std::unique_ptr<void, void (*)(void*)>(dlopen(libPath, RTLD_LAZY), dlDeleter);
    if (!upDL) {
        std::string _err{"Failed to open DataProvider library: "};
        _err.append(libPath);
        throw std::runtime_error(_err);
    }

    dlerror();
    const char *dlsym_error = nullptr;

    auto f_create = loadFunction<create_t>("create", dlsym_error, upDL.get());
    if (!f_create) {
        throw std::runtime_error(dlsym_error);
    }

    f_destroy = loadFunction<destroy_t>("destroy", dlsym_error, upDL.get());
    if (!f_destroy) {
        throw std::runtime_error(dlsym_error);
    }

    dlLibrary = f_create(rdbType, rdbPath_);
    if (dlLibrary == nullptr) {
        throw std::runtime_error("Cannot create DataProvider library instance");
    }

    if (!dlLibrary->read()) {
        throw std::runtime_error("Failed to read data");
    }

    signals_.add(SIGTERM);
    do_await_stop();

    do_accept();
}

Server::~Server()
{
    f_destroy(dlLibrary);
}

void Server::run()
{
    io_service_.run();
}

void Server::do_accept()
{
    acceptor_.async_accept(socket_,
        [this](boost::system::error_code ec)
        {
            // Closing connections already?
            if (!acceptor_.is_open())
                return;

            if (!ec) {
                std::make_shared<Session>(Session(std::move(socket_), dlLibrary, rdbPath_))->start();
            }

            do_accept();
        }
    );
}

void Server::do_await_stop()
{
    signals_.async_wait(
        [this](boost::system::error_code /*ec*/, int /*signo*/)
        {
            acceptor_.close();
        }
    );
}
