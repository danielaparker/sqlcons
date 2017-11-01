#ifndef SQLCONS_CONNECTOR_CONNECTOR_HPP
#define SQLCONS_CONNECTOR_CONNECTOR_HPP

#include <windows.h> 
//#include <sql.h> 
#define UNICODE  
#include <string> 
#include <sqlext.h> 
#include <stdio.h> 
#include <conio.h> 
#include <tchar.h> 
#include <stdlib.h> 
#include <iostream>
#include <sqlcons/unicode_traits.hpp>
#include <vector>
#include <sstream>
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

// connection_impl

class connection_impl
{
    bool autoCommit_;
public:
    SQLHENV     henv_;
    SQLHDBC     hdbc_; 

    connection_impl();

    ~connection_impl();

    void open(const std::string& connString, bool autoCommit, std::error_code& ec);

    void auto_commit(bool val, std::error_code& ec);

    void connection_timeout(size_t val, std::error_code& ec);

    prepared_statement prepare_statement(const std::string& query, std::error_code& ec);

    prepared_statement prepare_statement(const std::string& query, transaction& trans);

    void commit(std::error_code& ec);
    void rollback(std::error_code& ec);
    void execute(const std::string& query, 
                 std::error_code& ec);
    void execute(const std::string& query, 
                 const std::function<void(const row& rec)>& callback,
                 std::error_code& ec);
};

// transaction_impl

class transaction_impl
{
    connection_impl* pimpl_;
    std::error_code ec_;
public:
    transaction_impl(connection_impl* pimpl);

    ~transaction_impl();

    std::error_code error_code() const;

    void update_error_code(std::error_code ec);

    void end(std::error_code& ec);
};


// prepared_statement_impl

class prepared_statement_impl
{
    SQLHSTMT hstmt_; 
public:
    prepared_statement_impl();

    prepared_statement_impl(SQLHSTMT hstmt);

    prepared_statement_impl(const prepared_statement_impl&) = delete;

    prepared_statement_impl(prepared_statement_impl&&) = default;

    ~prepared_statement_impl()
    {
        if (hstmt_) 
        { 
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt_); 
        } 
    }

    void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                    const std::function<void(const row& rec)>& callback,
                    std::error_code& ec);

    void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                    std::error_code& ec);

    void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                  transaction& t);
};

}

#endif
