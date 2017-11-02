#ifndef SQLCONS_CONNECTOR_CONNECTORFWD_HPP
#define SQLCONS_CONNECTOR_CONNECTORFWD_HPP

#include <memory>
#include <system_error>
#include <functional>
#include <vector>
#include <map>
#include <string>

namespace sqlcons { 

class connection_impl;

namespace odbc {
class connector
{
public:
    static std::unique_ptr<connection_impl> create_connection();
};

}

}

#endif
