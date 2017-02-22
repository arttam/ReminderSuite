#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

// Get running path
#include <limits.h>
#include <unistd.h>
// File exists?
#include <sys/stat.h>

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

	// Try to find library either in current folder or ../lib/ folder, faile otherwise
	if (!setLibraryPath(rdbType)) {
		std::string _err{"Failed to find DataProvider library for "};
		_err.append(rdbType);
		throw std::runtime_error(_err);
	}
	
    upDL = std::unique_ptr<void, void (*)(void*)>(dlopen(libraryPath_.c_str(), RTLD_LAZY), dlDeleter);
    if (!upDL) {
        std::string _err{"Failed to open DataProvider library: "};
        _err.append(libraryPath_);
		_err.append("; Reason: ");
		_err.append(dlerror());
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

    dlLibrary = f_create(rdbPath_);
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

bool Server::setLibraryPath(const char *rdbType)
{
	std::string _type{rdbType};

	char result[ PATH_MAX ];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	std::string _exePath(result, (count > 0) ? count : 0);
	if (_exePath.empty())
		return false;

	_exePath.erase(_exePath.find_last_of('/'));

	std::string _libName;

	if (_type == "file") {
		_libName.assign("libTextFileProvider.so");
	}
	else if (_type == "sqlite") {
		_libName.assign("libDBSQLiteProvider.so");
	}
	else {
		return false;
	}

	struct stat buffer;
	std::string _currTest = _exePath + "/" + _libName;
	if (stat(_currTest.c_str(), &buffer) == 0) {
		// Exists
		libraryPath_.assign(_currTest);
		return true;
	}
	
	_currTest = _exePath + "/../lib/" + _libName;
	if (stat(_currTest.c_str(), &buffer) == 0) {
		// Exists
		libraryPath_.assign(_currTest);
		return true;
	}

	return false;
}
