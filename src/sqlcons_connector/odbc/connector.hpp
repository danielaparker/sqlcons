#ifndef SQLCONSCONNECTOR_ODBC_CONNECTOR_HPP
#define SQLCONSCONNECTOR_ODBC_CONNECTOR_HPP

#include <sqlcons/sqlcons.hpp>

namespace sqlcons { 

// conv_errc

enum class sql_errc 
{
    db_err = 1,
    E_01000,
    E_01S02,
    E_01001,
    E_01003,
    E_07002,
    E_07006,
    E_07007,
    E_07S01,
    E_08S01,
    E_21S02,
    E_22001,
    E_22002,
    E_22003,
    E_22007,
    E_22008,
    E_22012,
    E_22015,
    E_22018,
    E_22019,
    E_22025,
    E_23000,
    E_24000,
    E_40001,
    E_40003,
    E_42000,
    E_44000,
    E_HY000,
    E_HY001,
    E_HY008,
    E_HY009,
    E_HY010,
    E_HY013,
    E_HY024,
    E_HY090,
    E_HY092,
    E_HY104,
    E_HY117,
    E_HYT01,
    E_HYC00,
    E_IM001,
    E_IM017,
    E_IM018,
    E_42S22
};

class sqlcons_error_category_impl
   : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "sqlcons.error";
    }
    std::string message(int ev) const override;
};

const std::error_category& sqlcons_error_category();

std::error_code make_error_code(sql_errc result);

namespace odbc {
class connector
{
public:
    static std::unique_ptr<connection_impl> create_connection();
};

}

}

#endif
