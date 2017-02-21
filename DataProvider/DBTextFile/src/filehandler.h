#ifndef FILEHANDLER_H
#define FILEHANDLER_H
#include <fstream>
#include <string>

class fileHandler
{
    std::fstream file_;
public:
    fileHandler(const char *fname, bool overwrite = false);
    ~fileHandler();

    operator bool() const { return file_.is_open(); }
    std::fstream& getStream() { return file_; }

    std::string getContents();
};

#endif // FILEHANDLER_H
