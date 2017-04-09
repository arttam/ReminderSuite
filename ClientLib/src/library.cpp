#include <iostream>
#include <regex>
#include "library.h"

DataClient *createClient(const char* host, const char *port) {
    return new DataClient(std::string(host), std::string(port));
}

void destroyClient(DataClient* client) {
    delete client;
}

DataClient::DataClient(std::string host, std::string port)
    : host_{host}
    , port_{port}
    , io_service_{}
    , socket_(io_service_, boost::asio::ip::tcp::v4())
{
    boost::asio::ip::tcp::resolver _resolver(io_service_);
    endPoint_ = _resolver.resolve({host_, port_});
}

DataClient::~DataClient()
{
    if (!io_service_.stopped())
        io_service_.stop();
}

bool DataClient::executeCmd(std::string cmd, std::vector<std::string> &result)
{
    try {
        boost::system::error_code error_code_;
        boost::asio::connect(socket_, endPoint_, error_code_);
        if (error_code_) {
            std::cerr << "Failed to connect via socket: " << error_code_.message() << std::endl;
            return false;
        }

        socket_.send(boost::asio::buffer(cmd.c_str(), cmd.length()), 0, error_code_);
        if (error_code_) {
            std::cerr << "Failed to send command: " << error_code_.message() << std::endl;
            socket_.close();
            return false;
        }

        char _streamBuf[8192];
        size_t _bRead = socket_.read_some(boost::asio::buffer(_streamBuf), error_code_);
        if (error_code_) {
            std::cerr << "Failed to read get response: " << error_code_.message() << std::endl;
            socket_.close();
            return false;
        }

        std::string _rb(_streamBuf, _bRead);

        std::regex _rEOL("\\n");
        std::sregex_token_iterator _dLine(_rb.begin(), _rb.end(), _rEOL, -1);
        while (_dLine != std::sregex_token_iterator())
            result.emplace_back(*_dLine++);

        socket_.close();

        return true;
    }
    catch (boost::system::system_error error_) {
        std::cerr << "Exception thrown during command execution: " << error_.what() << std::endl;
        socket_.close();
        return false;
    }
    catch (std::exception& ex) {
        std::cerr << "Generic exception thrown during command execution: " << ex.what() << std::endl;
        socket_.close();
        return false;
    }
}
