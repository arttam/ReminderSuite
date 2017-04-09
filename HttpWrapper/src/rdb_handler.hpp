#include <string>
#include <set>
#include "reply.hpp"

class rdb_handler
{
	static const std::set<std::string> commands_;
	std::string reply_;

public:
	static bool isRDBcall(const std::string& request);
	static bool isValidCall(const std::string& request);

	rdb_handler() = default;
	bool parse(const std::string& request);
	const std::string& getResponse() const;
};
