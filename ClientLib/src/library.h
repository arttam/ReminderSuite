#ifndef CLIENTLIB_LIBRARY_H
#define CLIENTLIB_LIBRARY_H

#include <vector>
#include <string>
#include <boost/asio.hpp>


class DataClient
{
public:
    explicit DataClient(std::string host, std::string port);
    ~DataClient();

    bool executeCmd(std::string cmd, std::vector<std::string>& result);

private:
    std::string host_;
    std::string port_;

    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver::iterator endPoint_;
};


extern "C" {
    DataClient *createClient(const char* host, const char *port);
    void destroyClient(DataClient* client);
}
#endif