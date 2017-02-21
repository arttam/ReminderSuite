#include "filehandler.h"

fileHandler::fileHandler(const char *fname, bool overwrite)
{
    if (overwrite)
        file_.open(fname, std::ios_base::out|std::ios_base::trunc);
    else
        file_.open(fname);
}

fileHandler::~fileHandler()
{
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}

std::string fileHandler::getContents()
{
    return std::string((std::istreambuf_iterator<char>(file_)), std::istreambuf_iterator<char>());
}
