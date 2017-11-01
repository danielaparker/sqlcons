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
