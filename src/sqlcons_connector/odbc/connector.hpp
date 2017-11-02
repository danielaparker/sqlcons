#ifndef SQLCONSCONNECTOR_ODBC_CONNECTOR_HPP
#define SQLCONSCONNECTOR_ODBC_CONNECTOR_HPP

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
#include <sqlcons_connector/odbc/connector_fwd.hpp>
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

// odbc_connection_impl

class odbc_connection_impl : public virtual connection_impl
{
    bool autoCommit_;
public:
    SQLHENV     henv_;
    SQLHDBC     hdbc_; 

    odbc_connection_impl();

    ~odbc_connection_impl();

    void open(const std::string& connString, bool autoCommit, std::error_code& ec) override;

    void auto_commit(bool val, std::error_code& ec) override;

    void connection_timeout(size_t val, std::error_code& ec) override;

    std::unique_ptr<transaction_impl> create_transaction() override;

    std::unique_ptr<prepared_statement_impl> prepare_statement(const std::string& query, std::error_code& ec) override;

    std::unique_ptr<prepared_statement_impl> prepare_statement(const std::string& query, transaction& trans) override;

    void commit(std::error_code& ec) override;
    void rollback(std::error_code& ec) override;
    void execute(const std::string& query, 
                 std::error_code& ec) override;
    void execute(const std::string& query, 
                 const std::function<void(const row& rec)>& callback,
                 std::error_code& ec) override;
};

// odbc_transaction_impl

class odbc_transaction_impl : public virtual transaction_impl
{
    connection_impl* pimpl_;
    std::error_code ec_;
public:
    odbc_transaction_impl(connection_impl* pimpl);

    ~odbc_transaction_impl();

    std::error_code error_code() const override;

    void update_error_code(std::error_code ec) override;

    void end(std::error_code& ec) override;
};


// odbc_prepared_statement_impl

class odbc_prepared_statement_impl : public virtual prepared_statement_impl
{
    SQLHSTMT hstmt_; 
public:
    odbc_prepared_statement_impl();

    odbc_prepared_statement_impl(SQLHSTMT hstmt);

    odbc_prepared_statement_impl(const odbc_prepared_statement_impl&) = delete;

    odbc_prepared_statement_impl(odbc_prepared_statement_impl&&) = default;

    ~odbc_prepared_statement_impl()
    {
        if (hstmt_) 
        { 
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt_); 
        } 
    }

    void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                    const std::function<void(const row& rec)>& callback,
                    std::error_code& ec) override;

    void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                    std::error_code& ec) override;

    void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                  transaction& t) override;
};

}

#endif
