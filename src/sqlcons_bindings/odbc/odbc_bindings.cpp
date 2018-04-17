#include <sqlcons_bindings/odbc/odbc_bindings.hpp>
#include <windows.h> 
#ifndef UNICODE  
#define UNICODE  
#endif
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

namespace sqlcons { 

template<>
int sql_type_traits<odbc::odbc_bindings,bool>::sql_type_identifier() { return SQL_CHAR; }
template<>
int sql_type_traits<odbc::odbc_bindings,bool>::c_type_identifier() { return SQL_C_CHAR; }

template<>
int sql_type_traits<odbc::odbc_bindings,std::string>::sql_type_identifier() { return SQL_WVARCHAR; }
template<>
int sql_type_traits<odbc::odbc_bindings,std::string>::c_type_identifier() { return SQL_C_WCHAR; }

template<>
int sql_type_traits<odbc::odbc_bindings,int16_t>::sql_type_identifier() { return SQL_SMALLINT; }
template<>
int sql_type_traits<odbc::odbc_bindings,int16_t>::c_type_identifier() { return SQL_C_SSHORT; }

template<>
int sql_type_traits<odbc::odbc_bindings,int32_t>::sql_type_identifier() { return SQL_INTEGER; }
template<>
int sql_type_traits<odbc::odbc_bindings,int32_t>::c_type_identifier() { return SQL_C_SLONG; }

template<>
int sql_type_traits<odbc::odbc_bindings,int64_t>::sql_type_identifier() { return SQL_BIGINT; }
template<>
int sql_type_traits<odbc::odbc_bindings,int64_t>::c_type_identifier() { return SQL_C_SBIGINT; }

template<>
int sql_type_traits<odbc::odbc_bindings,uint64_t>::sql_type_identifier() { return SQL_BIGINT; }
template<>
int sql_type_traits<odbc::odbc_bindings,uint64_t>::c_type_identifier() { return SQL_C_UBIGINT; }

template<>
int sql_type_traits<odbc::odbc_bindings,double>::sql_type_identifier() { return SQL_DOUBLE; }
template<>
int sql_type_traits<odbc::odbc_bindings,double>::c_type_identifier() { return SQL_C_DOUBLE; }

namespace odbc {

// odbc_error_category_impl

const std::error_category& odbc_error_category()
{
    static odbc_error_category_impl instance;
    return instance;
}

std::error_code make_error_code(odbc_errc result)
{
    return std::error_code(static_cast<int>(result), odbc_error_category());
}

std::string odbc_error_category_impl::message(int ev) const
{
    switch (static_cast<odbc_errc>(ev))
    {
    case odbc_errc::E_01000:
        return "[01000] General warning";
    case odbc_errc::E_01S02:
        return "[01S02] Option value changed";
    case odbc_errc::E_01001:
        return "[01001] Cursor operation conflict";
    case odbc_errc::E_01003:
        return "[01003] Cursor operation conflict";
    case odbc_errc::E_01004:
        return "[01004] String data, right truncated";
    case odbc_errc::E_01006:
        return "[01006] Privilidge not revoked";
    case odbc_errc::E_01007:
        return "[01007] Privilidge not granted";
    case odbc_errc::E_07002:
        return "[07002] COUNT field incorrect";
    case odbc_errc::E_07006:
        return "[07006] Restricted data type attribute violation";
    case odbc_errc::E_07007:
        return "[07007] Restricted parameter value violation";
    case odbc_errc::E_07S01:
        return "[07S01] Invalid use of default parameter";
    case odbc_errc::E_08S01:
        return "[08S01] Communication link failure";
    case odbc_errc::E_21S02:
        return "[21S02] Degree of derived table does not match column list";
    case odbc_errc::E_22001:
        return "[22001] String data, right truncation";
    case odbc_errc::E_22002:
        return "[22002] Indicator variable required but not supplied";
    case odbc_errc::E_22003:
        return "[22003] Numeric value out of range";
    case odbc_errc::E_22007:
        return "[22007] Invalid datetime format";
    case odbc_errc::E_22008:
        return "Datetime field overflow";
    case odbc_errc::E_22012:
        return "[22012] Division by zero";
    case odbc_errc::E_22015:
        return "[22015] Interval field overflow";
    case odbc_errc::E_22018:
        return "[22018] Interval field overflowInvalid character value for cast specification";
    case odbc_errc::E_22019:
        return "[22019] Invalid escape character";
    case odbc_errc::E_22025:
        return "[22025] Invalid escape sequence";
    case odbc_errc::E_23000:
        return "[23000] Integrity constraint violation";
    case odbc_errc::E_24000:
        return "[24000] Invalid cursor state";
    case odbc_errc::E_40001:
        return "[40001] Serialization failure";
    case odbc_errc::E_40003:
        return "[40003] Statement completion unknown";
    case odbc_errc::E_42000:
        return "[42000] Syntax error or access violation";
    case odbc_errc::E_42S02:
        return "[42S02] Base table or view not found";
    case odbc_errc::E_44000:
        return "[44000] WITH CHECK OPTION violation";
    case odbc_errc::E_HY000:
        return "[HY000] General error";
    case odbc_errc::E_HY001:
        return "[HY001] Memory allocation error";
    case odbc_errc::E_HY008:
        return "[HY008] Operation canceled";
    case odbc_errc::E_HY009:
        return "[HY009] Invalid use of null pointer";
    case odbc_errc::E_HY010:
        return "[HY010] Function sequence error";
    case odbc_errc::E_HY013:
        return "[HY013] Memory management error";
    case odbc_errc::E_HY024:
        return "[HY024] Invalid attribute value";
    case odbc_errc::E_HY090:
        return "[HY090] Invalid string or buffer length";
    case odbc_errc::E_HY092:
        return "[HY092] Invalid attribute/option identifier";
    case odbc_errc::E_HY104:
        return "[HY104] Invalid precision or scale value";
    case odbc_errc::E_HY117:
        return "[HY117] Connection is suspended due to unknown transaction state";
    case odbc_errc::E_HYT01:
        return "[HYT01] Connection timeout expired";
    case odbc_errc::E_HYC00:
        return "[HYC00] Optional feature not implemented";
    case odbc_errc::E_IM001:
        return "[IM001] Driver does not support this function";
    case odbc_errc::E_IM017:
        return "[IM017] Polling is disabled in asynchronous notification mode";
    case odbc_errc::E_IM018:
        return "[IM018] SQLCompleteAsync has not been called to complete the previous asynchronous operation on this handle. If the previous function call on the handle returns SQL_STILL_EXECUTING and if notification mode is enabled, SQLCompleteAsync must be called on the handle to do post-processing and complete the operation";
    case odbc_errc::E_42S22:
        return "[E_42S22] Column not found";
    default:
        return "db error";
    }
}

// odbc_connection_impl

class odbc_connection_impl : public virtual connection_impl
{
    bool autoCommit_;
public:
    SQLHENV     henv_;
    SQLHDBC     hdbc_; 

    odbc_connection_impl();

    ~odbc_connection_impl();

    void open(const std::string& connString, std::error_code& ec) override;

    void auto_commit(bool val, std::error_code& ec) override;

    void connection_timeout(size_t val, std::error_code& ec) override;

    std::unique_ptr<prepared_statement_impl> prepare_statement(const std::string& query, std::error_code& ec) override;

    void commit(std::error_code& ec) override;
    void rollback(std::error_code& ec) override;
    void execute(const std::string& query, 
                 std::error_code& ec) override;
    void execute(const std::string& query, 
                 const std::function<void(const row& rec)>& callback,
                 std::error_code& ec) override;

    bool is_valid() const override;
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

    void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                  const std::function<void(const row& rec)>& callback,
                  std::error_code& ec) override;

    void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                  std::error_code& ec) override;

};

// odbc_bindings

std::unique_ptr<connection_impl> odbc_bindings::create_connection(const std::string& connString, std::error_code& ec)
{
    auto ptr = std::make_unique<odbc_connection_impl>();
    ptr->open(connString, ec);
    return std::move(ptr);
}

void process_results(SQLHSTMT hstmt,
                     const std::function<void(const row& rec)>& callback,
                     std::error_code& ec);

void handle_diagnostic_record(SQLHANDLE hHandle,
                              SQLSMALLINT hType,
                              RETCODE RetCode,
                              std::error_code& ec);


// statement_impl

class statement_impl
{
    SQLHSTMT hstmt_; 
public:
    statement_impl()
        : hstmt_(nullptr)
    {
    }

    ~statement_impl()
    {
        if (hstmt_) 
        { 
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt_); 
        } 
    }

    void execute(SQLHDBC hDbc, 
                 const std::string& query, 
                 std::error_code& ec);

    void execute(SQLHDBC hDbc, 
                 const std::string& query, 
                 const std::function<void(const row& rec)>& callback,
                 std::error_code& ec);
};

// odbc_connection_impl

odbc_connection_impl::odbc_connection_impl()
    : henv_(nullptr), hdbc_(nullptr), autoCommit_(false)
{
}

odbc_connection_impl::~odbc_connection_impl()
{
    if (hdbc_) 
    { 
        SQLDisconnect(hdbc_); 
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc_); 
    } 

    if (henv_) 
    { 
        SQLFreeHandle(SQL_HANDLE_ENV, henv_); 
    } 
}

void odbc_connection_impl::auto_commit(bool val, std::error_code& ec)
{
    autoCommit_ = val;

    RETCODE rc;
    if (val)
    {
        rc = SQLSetConnectAttr(hdbc_, 
                           SQL_ATTR_AUTOCOMMIT, 
                           (SQLPOINTER)TRUE, 
                           0);
    }
    else
    {
        rc = SQLSetConnectAttr(hdbc_, 
                           SQL_ATTR_AUTOCOMMIT, 
                           (SQLPOINTER)FALSE, 
                           0);
    }
    if (rc != SQL_SUCCESS)
    {
        handle_diagnostic_record(henv_, SQL_HANDLE_ENV, rc, ec);
    }
}

void odbc_connection_impl::connection_timeout(size_t val, std::error_code& ec)
{
    RETCODE rc;

    rc = SQLSetConnectAttr(hdbc_, 
                       SQL_ATTR_CONNECTION_TIMEOUT, 
                       (SQLPOINTER)val, 
                       0);
    if (rc != SQL_SUCCESS)
    {
        handle_diagnostic_record(henv_, SQL_HANDLE_ENV, rc, ec);
    }
}

void odbc_connection_impl::execute(const std::string& query, 
                                   const std::function<void(const row& rec)>& callback,
                                   std::error_code& ec)
{
    statement_impl q;
    q.execute(hdbc_,query,callback,ec);
}

bool odbc_connection_impl::is_valid() const
{
    SQLUINTEGER	valid = SQL_CD_FALSE;
    RETCODE rc = SQLGetConnectAttr(hdbc_, 
                                   //SQL_COPT_SS_CONNECTION_DEAD,
                                  SQL_ATTR_CONNECTION_DEAD,
                                  (SQLPOINTER) &valid,
                                  (SQLINTEGER) sizeof(valid),
                                  NULL);
    return rc == SQL_SUCCESS && valid == SQL_CD_TRUE;
}

void odbc_connection_impl::execute(const std::string& query, 
                               std::error_code& ec)
{
    statement_impl q;
    q.execute(hdbc_,query,ec);
}

void odbc_connection_impl::open(const std::string& connString, std::error_code& ec)
{
    autoCommit_ = true;

    RETCODE rc = SQLAllocHandle(SQL_HANDLE_ENV, 
                                SQL_NULL_HANDLE, 
                                &henv_);
    if (rc == SQL_ERROR)
    {
        ec = make_error_code(odbc_errc::db_err);
        return;
    }

    // Register this as an application that expects 3.x behavior, 
    // you must register something if you use AllocHandle 

    rc = SQLSetEnvAttr(henv_, 
                       SQL_ATTR_ODBC_VERSION, 
                       (SQLPOINTER)SQL_OV_ODBC3, 
                       0);
    if (rc != SQL_SUCCESS)
    {
        handle_diagnostic_record(henv_, SQL_HANDLE_ENV, rc, ec);
        return;
    }

    rc = SQLAllocHandle(SQL_HANDLE_DBC, henv_, &hdbc_);
    if (rc != SQL_SUCCESS)
    {
        handle_diagnostic_record (henv_, SQL_HANDLE_ENV, rc, ec);
        return;
    }

    std::wstring cs;
    auto result1 = unicons::convert(connString.begin(),connString.end(),
                                    std::back_inserter(cs), 
                                    unicons::conv_flags::strict);
    //std::cout << connString << std::endl;
    //std::wcout << cs << std::endl;

    rc = SQLSetConnectAttr(hdbc_, 
                       SQL_ATTR_AUTOCOMMIT, 
                       (SQLPOINTER)TRUE, 
                       0);

    if (rc != SQL_SUCCESS)
    {
        handle_diagnostic_record(henv_, SQL_HANDLE_ENV, rc, ec);
        return;
    }

    // Connect to the driver.  Use the connection string if supplied 
    // on the input, otherwise let the driver manager prompt for input. 
    rc = SQLDriverConnect(hdbc_, 
                         NULL, 
                         &cs[0], 
                         (SQLSMALLINT)cs.size(), 
                         NULL, 
                         0, 
                         NULL,
                         SQL_DRIVER_NOPROMPT);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record (hdbc_, SQL_HANDLE_DBC, rc, ec);
        return;
    }
}

std::unique_ptr<prepared_statement_impl> odbc_connection_impl::prepare_statement(const std::string& query, std::error_code& ec)
{
    std::wstring wquery;
    auto result1 = unicons::convert(query.begin(), query.end(),
                                    std::back_inserter(wquery), 
                                    unicons::conv_flags::strict);

    SQLHSTMT    hstmt; 
    RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &hstmt);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hdbc_, SQL_HANDLE_DBC, rc, ec);
        return std::unique_ptr<odbc_prepared_statement_impl>();
    }
    rc = SQLPrepare(hstmt, &wquery[0], (SQLINTEGER)wquery.size()); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
        return std::unique_ptr<odbc_prepared_statement_impl>();
    }

    return std::make_unique<odbc_prepared_statement_impl>(hstmt);
}

void odbc_connection_impl::commit(std::error_code& ec)
{
    if (!autoCommit_)
    {
        RETCODE rc = SQLEndTran (SQL_HANDLE_DBC, hdbc_, SQL_COMMIT);
        if (rc == SQL_ERROR)
        {
            ec = make_error_code(odbc_errc::db_err);
            return;
        }
    }
}

void odbc_connection_impl::rollback(std::error_code& ec)
{
    if (!autoCommit_)
    {
        RETCODE rc = SQLEndTran (SQL_HANDLE_DBC, hdbc_, SQL_ROLLBACK);
        if (rc == SQL_ERROR)
        {
            ec = make_error_code(odbc_errc::db_err);
            return;
        }
    }
}


// value_impl

class value_impl : public value
{
public:
    virtual void bind(SQLHSTMT hstmt, std::error_code& ec) = 0;
    virtual void get_data(SQLHSTMT hstmt, std::error_code& ec) = 0;
};

class string_value : public value_impl
{
public:
    std::wstring name_;
    SQLUSMALLINT column_;
    SQLULEN column_size_;
    SQLSMALLINT nullable_;

    std::vector<CHAR> value_;
    SQLLEN length_or_null_;  // size or null

    string_value(std::wstring&& name,
                 SQLUSMALLINT column,
                 SQLULEN column_size,
                 SQLSMALLINT nullable)
        : name_(name),
          column_(column),
          column_size_(column_size),
          nullable_(nullable),
          length_or_null_(0),
          value_(column_size+1)
    {
    }

    void bind(SQLHSTMT hstmt, std::error_code& ec)
    {
        RETCODE rc;

        size_t buffer_length = value_.size();
        rc = SQLBindCol(hstmt, 
            column_, 
            SQL_C_CHAR, 
            (SQLPOINTER)&value_[0], 
            buffer_length, 
            &length_or_null_); 
        if (rc == SQL_ERROR)
        {
            handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
            return;
        }
    }

    void get_data(SQLHSTMT hstmt, std::error_code& ec)
    {
    }

    bool is_null() const
    {
        return length_or_null_ == SQL_NULL_DATA;
    }

    std::wstring as_wstring() const override
    {
        if (is_null())
        {
            return L"";
        }
        else
        {
            size_t len = length_or_null_;
            std::wstring s;
            auto result1 = unicons::convert(value_.begin(),value_.begin() + len,
                                            std::back_inserter(s), 
                                            unicons::conv_flags::strict);
            return s;
        }
    }

    std::string as_string() const override
    {
        if (is_null())
        {
            return "";
        }
        else
        {
            size_t len = length_or_null_;
            return std::string(value_.data(), value_.data() + len);
        }
    }

    double as_double() const override
    {
        size_t len = length_or_null_;
        std::istringstream is(std::string(value_.data(), value_.data() + len));
        double d;
        is >> d;
        return d;
    }

    long as_long() const override
    {
        return 0;
    }
};

class long_string_value : public value_impl
{
public:
    std::wstring name_;
    SQLUSMALLINT column_;
    SQLSMALLINT nullable_;

    std::vector<CHAR> value_;
    SQLLEN length_or_null_;  // size or null

    long_string_value(std::wstring&& name,
                      SQLUSMALLINT column,
                      SQLSMALLINT nullable)
        : name_(name),
          column_(column),
          nullable_(nullable),
          length_or_null_(0),
          value_(1024)
    {
    }

    void bind(SQLHSTMT hstmt, std::error_code& ec)
    {
    }

    void get_data(SQLHSTMT hstmt, std::error_code& ec)
    {
        RETCODE rc;

        SQLLEN length_or_null = 0;  // size or null
        size_t offset = 0;
        size_t size = value_.size();

        bool first = true;
        bool done = false;
        while (!done)
        {
            rc = SQLGetData(hstmt, 
                            column_, 
                            SQL_C_CHAR, 
                            (SQLPOINTER)&(value_[offset]), 
                            size, 
                            &length_or_null); 
            if (rc == SQL_ERROR)
            {
                handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                done = true;
            }
            else if (rc == SQL_NO_DATA)
            {
                done = true;
            }
            if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
            {
                if (first)
                {
                    if (length_or_null == SQL_NULL_DATA)
                    {
                        length_or_null_ = SQL_NULL_DATA;
                    }
                    else
                    {
                        length_or_null_ = length_or_null;
                        if ((size_t)length_or_null < size)
                        {
                            done = true;
                        }
                        else
                        {
                            value_.resize(length_or_null + 1);
                            offset += size - 1;
                            size = length_or_null - offset + 1;
                        }
                    }
                    first = false;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    bool is_null() const
    {
        return length_or_null_ == SQL_NULL_DATA;
    }

    std::wstring as_wstring() const override
    {
        if (is_null())
        {
            return L"";
        }
        else
        {
            size_t len = length_or_null_;
            std::wstring s;
            auto result1 = unicons::convert(value_.begin(),value_.begin() + len,
                                            std::back_inserter(s), 
                                            unicons::conv_flags::strict);
            return s;
        }
    }

    std::string as_string() const override
    {
        if (is_null())
        {
            return "";
        }
        else
        {
            size_t len = length_or_null_;
            return std::string(value_.data(), value_.data() + len);
        }
    }

    double as_double() const override
    {
        size_t len = length_or_null_;
        std::istringstream is(std::string(value_.data(), value_.data() + len));
        double d;
        is >> d;
        return d;
    }

    long as_long() const override
    {
        return 0;
    }
};

class wstring_value : public value_impl
{
public:
    std::wstring name_;
    SQLUSMALLINT column_;
    SQLULEN column_size_;
    SQLSMALLINT nullable_;

    std::vector<WCHAR> value_;
    SQLLEN length_or_null_;  // size or null

    wstring_value(std::wstring&& name,
                  SQLUSMALLINT column,
                  SQLULEN column_size,
                  SQLSMALLINT nullable)
        : name_(name),
          column_(column),
          column_size_(column_size),
          nullable_(nullable),
          length_or_null_(0),
          value_(column_size+1)
    {
    }

    void bind(SQLHSTMT hstmt, std::error_code& ec)
    {
        RETCODE rc;

        size_t buffer_length = value_.size();
        rc = SQLBindCol(hstmt, 
            column_, 
            SQL_C_WCHAR, 
            (SQLPOINTER)&value_[0], 
            buffer_length * sizeof(WCHAR), 
            &length_or_null_); 
        if (rc == SQL_ERROR)
        {
            handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
            return;
        }
    }

    void get_data(SQLHSTMT hstmt, std::error_code& ec)
    {
    }

    bool is_null() const
    {
        return length_or_null_ == SQL_NULL_DATA;
    }

    std::wstring as_wstring() const override
    {
        if (is_null())
        {
            return L"";
        }
        else
        {
            size_t len = length_or_null_/sizeof(wchar_t);
            return std::wstring(value_.data(), value_.data() + len);
        }
    }

    std::string as_string() const override
    {
        if (is_null())
        {
            return "";
        }
        else
        {
            size_t len = length_or_null_/sizeof(wchar_t);
            std::string s;
            auto result1 = unicons::convert(value_.begin(),value_.begin() + len,
                                            std::back_inserter(s), 
                                            unicons::conv_flags::strict);
            return s;
        }
    }

    double as_double() const override
    {
        size_t len = length_or_null_;
        std::wistringstream is(std::wstring(value_.data(), value_.data() + len));
        double d;
        is >> d;
        return d;
    }

    long as_long() const override
    {
        return 0;
    }
};

class long_wstring_value : public value_impl
{
public:
    std::wstring name_;
    SQLUSMALLINT column_;
    SQLSMALLINT nullable_;

    std::vector<WCHAR> value_;
    SQLLEN length_or_null_;  // size or null

    long_wstring_value(std::wstring&& name,
                       SQLUSMALLINT column,
                       SQLSMALLINT nullable)
        : name_(name),
          column_(column),
          nullable_(nullable),
          length_or_null_(0),
          value_(1024)
    {
    }

    void bind(SQLHSTMT hstmt, std::error_code& ec)
    {
    }

    void get_data(SQLHSTMT hstmt, std::error_code& ec)
    {   
        RETCODE rc;

        SQLLEN length_or_null = 0;  // size or null
        size_t offset = 0;
        size_t size = value_.size();

        bool first = true;
        bool done = false;
        while (!done)
        {
            rc = SQLGetData(hstmt, 
                            column_, 
                            SQL_C_WCHAR, 
                            (SQLPOINTER)&(value_[offset]), 
                            size*sizeof(wchar_t), 
                            &length_or_null); 
            if (rc == SQL_ERROR)
            {
                handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                done = true;
            }
            else if (rc == SQL_NO_DATA)
            {
                done = true;
            }
            else if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
            {
                if (first)
                {
                    if (length_or_null == SQL_NULL_DATA)
                    {
                        length_or_null_ = SQL_NULL_DATA;
                    }
                    else
                    {
                        length_or_null_ = length_or_null;
                        if (length_or_null/sizeof(wchar_t) < size)
                        {
                            done = true;
                        }
                        else
                        {
                            value_.resize(length_or_null/sizeof(wchar_t) + 1);
                            offset += size - 1;
                            size = length_or_null/sizeof(wchar_t) - offset + 1;
                        }
                    }
                    first = false;
                }
                else
                {
                    done = true;
                }
            }
            else
            {
                done = true;
            }
        }
    }

    bool is_null() const
    {
        return length_or_null_ == SQL_NULL_DATA;
    }


    std::wstring as_wstring() const override
    {
        if (is_null())
        {
            return L"";
        }
        else
        {
            size_t len = length_or_null_/sizeof(wchar_t);
            std::wcout << L"len: " << len << ", str: " << std::wstring(value_.data(), value_.data() + len) << std::endl;
            return std::wstring(value_.data(), value_.data() + len);
        }
    }

    std::string as_string() const override
    {
        if (is_null())
        {
            return "";
        }
        else
        {
            size_t len = length_or_null_/sizeof(wchar_t);
            std::string s;
            auto result1 = unicons::convert(value_.begin(),value_.begin() + len,
                                            std::back_inserter(s), 
                                            unicons::conv_flags::strict);
            return s;
        }
    }

    double as_double() const override
    {
        size_t len = length_or_null_;
        std::wistringstream is(std::wstring(value_.data(), value_.data() + len));
        double d;
        is >> d;
        return d;
    }

    long as_long() const override
    {
        return 0;
    }
};

class integer_value : public value_impl
{
public:
    std::wstring name_;
    SQLUSMALLINT column_;
    SQLULEN column_size_;
    SQLSMALLINT nullable_;

    long value_;
    SQLLEN length_or_null_;  // size or null

    integer_value(std::wstring&& name,
                  SQLUSMALLINT column,
                  SQLULEN column_size,
                  SQLSMALLINT nullable)
        : name_(name),
          column_(column),
          column_size_(column_size),
          nullable_(nullable),
          value_(0),
          length_or_null_(0)
    {
    }

    void bind(SQLHSTMT hstmt, std::error_code& ec)
    {
        RETCODE rc;

        rc = SQLBindCol(hstmt,
            column_, 
            SQL_C_ULONG,
            (SQLPOINTER)&value_, 
            0, 
            &length_or_null_); 
        if (rc == SQL_ERROR)
        {
            handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
            return;
        }
    }

    void get_data(SQLHSTMT hstmt, std::error_code& ec)
    {
    }

    bool is_null() const
    {
        return length_or_null_ == SQL_NULL_DATA;
    }

    std::wstring as_wstring() const override
    {
        std::wstringstream ss;
        ss << value_;
        return ss.str();
    }

    std::string as_string() const override
    {
        std::stringstream ss;
        ss << value_;
        return ss.str();
    }

    double as_double() const override
    {
        return (double)value_;
    }

    long as_long() const override
    {
        return value_;
    }
};

class double_value : public value_impl
{
public:
    std::wstring name_;
    SQLUSMALLINT column_;
    SQLULEN column_size_;
    SQLSMALLINT nullable_;

    double value_;
    SQLLEN length_or_null_;  // size or null

    double_value(std::wstring&& name,
                 SQLUSMALLINT column,
                 SQLULEN column_size,
                 SQLSMALLINT nullable)
        : name_(name),
          column_(column),
          column_size_(column_size),
          nullable_(nullable),
          value_(0.0),
          length_or_null_(0)
    {
    }

    void bind(SQLHSTMT hstmt, std::error_code& ec)
    {
        RETCODE rc;

        rc = SQLBindCol(hstmt, 
            column_, 
            SQL_C_DOUBLE,
            (SQLPOINTER)(&value_), 
            0, 
            &length_or_null_); 
        if (rc == SQL_ERROR)
        {
            handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
            return;
        }
    }

    void get_data(SQLHSTMT hstmt, std::error_code& ec)
    {
    }

    bool is_null() const
    {
        return length_or_null_ == SQL_NULL_DATA;
    }

    std::wstring as_wstring() const override
    {
        std::wstringstream ss;
        ss << value_;
        return ss.str();
    }

    std::string as_string() const override
    {
        std::stringstream ss;
        ss << value_;
        return ss.str();
    }

    double as_double() const override
    {
        return value_;
    }

    long as_long() const override
    {
        return (long)value_;
    }
};


// statement_impl

void statement_impl::execute(SQLHDBC hDbc, 
                             const std::string& query, 
                             const std::function<void(const row& rec)>& callback,
                             std::error_code& ec)
{
    std::wstring buf;
    auto result1 = unicons::convert(query.begin(), query.end(),
                                    std::back_inserter(buf), 
                                    unicons::conv_flags::strict);

    RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt_);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hDbc, SQL_HANDLE_DBC, rc, ec);
        return;
    }

    rc = SQLExecDirect(hstmt_, &buf[0], (SQLINTEGER)buf.size()); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hstmt_, SQL_HANDLE_STMT, rc, ec);
        return;
    }

    process_results(hstmt_, callback, ec);
}

void statement_impl::execute(SQLHDBC hDbc, 
                             const std::string& query, 
                             std::error_code& ec)
{
    std::wstring buf;
    auto result1 = unicons::convert(query.begin(), query.end(),
                                    std::back_inserter(buf), 
                                    unicons::conv_flags::strict);

    RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt_);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hDbc, SQL_HANDLE_DBC, rc, ec);
        return;
    }

    rc = SQLExecDirect(hstmt_, &buf[0], (SQLINTEGER)buf.size()); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hstmt_, SQL_HANDLE_STMT, rc, ec);
        return;
    }
}

// odbc_prepared_statement_impl

odbc_prepared_statement_impl::odbc_prepared_statement_impl()
    : hstmt_(nullptr)
{
}

odbc_prepared_statement_impl::odbc_prepared_statement_impl(SQLHSTMT hstmt)
    : hstmt_(hstmt)
{
}

void odbc_prepared_statement_impl::execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                                            const std::function<void(const row& rec)>& callback,
                                            std::error_code& ec)
{
    RETCODE rc;

    std::vector<SQLLEN> lengths(bindings.size());

    for (size_t i = 0; i < bindings.size(); ++i)
    {
        lengths[i] = bindings[i]->buffer_length();
        //std::cout << "column_size: " << bindings[i]->column_size() << std::endl;
        rc = SQLBindParameter(hstmt_, 
                              i+1, 
                              SQL_PARAM_INPUT, 
                              bindings[i]->value_type(), 
                              bindings[i]->parameter_type(), 
                              bindings[i]->column_size(), 
                              0,
                              bindings[i]->pvalue(), 
                              bindings[i]->buffer_capacity(),
                              &lengths[i]);
        if (rc == SQL_ERROR)
        {
            handle_diagnostic_record(hstmt_, SQL_HANDLE_STMT, rc, ec);
            return;
        }
    }

    rc = SQLExecute(hstmt_); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hstmt_, SQL_HANDLE_STMT, rc, ec);
        return;
    }

    if (rc == SQL_PARAM_DATA_AVAILABLE)
    {
        std::cout << "SQL_PARAM_DATA_AVAILABLE" << std::endl;
    }
    if (rc == NO_DATA)
    {
        std::cout << "NO_DATA" << std::endl;
    }
    if (rc == SQL_NEED_DATA)
    {
        std::cout << "SQL_NEED_DATA" << std::endl;
    }

    process_results(hstmt_, callback, ec);
}

void odbc_prepared_statement_impl::execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                                          std::error_code& ec)
{
    RETCODE rc;

    std::vector<SQLLEN> lengths(bindings.size());

    for (size_t i = 0; i < bindings.size(); ++i)
    {
        lengths[i] = bindings[i]->buffer_length();
        //std::cout << "column_size: " << bindings[i]->column_size() << std::endl;
        rc = SQLBindParameter(hstmt_, 
                              i+1, 
                              SQL_PARAM_INPUT, 
                              bindings[i]->value_type(), 
                              bindings[i]->parameter_type(), 
                              bindings[i]->column_size(), 
                              0,
                              bindings[i]->pvalue(), 
                              bindings[i]->buffer_capacity(), 
                              &lengths[i]);
        if (rc == SQL_ERROR)
        {
            handle_diagnostic_record(hstmt_, SQL_HANDLE_STMT, rc, ec);
            return;
        }
    }

    rc = SQLExecute(hstmt_); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hstmt_, SQL_HANDLE_STMT, rc, ec);
        return;
    }

    if (rc == SQL_PARAM_DATA_AVAILABLE)
    {
        //std::cout << "SQL_PARAM_DATA_AVAILABLE" << std::endl;
    }
    if (rc == NO_DATA)
    {
        std::cout << "NO_DATA" << std::endl;
    }
    if (rc == SQL_NEED_DATA)
    {
        std::cout << "SQL_NEED_DATA" << std::endl;
    }
}

// compare_states

struct compare_states
{
    bool operator()(const wchar_t* key1, const wchar_t* key2) const
    {
        return wcsncmp(key1, key2, 5) < 0;
    }
};

struct odbc_error_codes
{
    std::map<const wchar_t*,odbc_errc,compare_states> code_map;

    odbc_error_codes()
    {
        code_map[L"01000"] = odbc_errc::E_01000;
        code_map[L"01001"] = odbc_errc::E_01001;
        code_map[L"01003"] = odbc_errc::E_01003;
        code_map[L"01004"] = odbc_errc::E_01004;
        code_map[L"01006"] = odbc_errc::E_01006;
        code_map[L"01007"] = odbc_errc::E_01007;
        code_map[L"01S02"] = odbc_errc::E_01S02; 
        code_map[L"07002"] = odbc_errc::E_07002;
        code_map[L"07006"] = odbc_errc::E_07006;
        code_map[L"07007"] = odbc_errc::E_07007;
        code_map[L"07S01"] = odbc_errc::E_07S01;
        code_map[L"08S01"] = odbc_errc::E_08S01;
        code_map[L"21S02"] = odbc_errc::E_21S02;
        code_map[L"22001"] = odbc_errc::E_22001;
        code_map[L"22002"] = odbc_errc::E_22002;
        code_map[L"22003"] = odbc_errc::E_22003;
        code_map[L"22007"] = odbc_errc::E_22007;
        code_map[L"22008"] = odbc_errc::E_22008;
        code_map[L"22012"] = odbc_errc::E_22012;
        code_map[L"22015"] = odbc_errc::E_22015;
        code_map[L"22018"] = odbc_errc::E_22018;
        code_map[L"22019"] = odbc_errc::E_22019;
        code_map[L"22025"] = odbc_errc::E_22025;
        code_map[L"23000"] = odbc_errc::E_23000;
        code_map[L"24000"] = odbc_errc::E_24000;
        code_map[L"40001"] = odbc_errc::E_40001;
        code_map[L"40003"] = odbc_errc::E_40003;
        code_map[L"42000"] = odbc_errc::E_42000;
        code_map[L"42S02"] = odbc_errc::E_42S02;
        code_map[L"44000"] = odbc_errc::E_44000;
        code_map[L"HY000"] = odbc_errc::E_HY000;
        code_map[L"HY001"] = odbc_errc::E_HY001;
        code_map[L"HY008"] = odbc_errc::E_HY008;
        code_map[L"HY009"] = odbc_errc::E_HY009; //
        code_map[L"HY010"] = odbc_errc::E_HY010;
        code_map[L"HY013"] = odbc_errc::E_HY013;
        code_map[L"HY024"] = odbc_errc::E_HY024; //
        code_map[L"HY090"] = odbc_errc::E_HY090; //
        code_map[L"HY092"] = odbc_errc::E_HY092; //
        code_map[L"HY104"] = odbc_errc::E_HY104;
        code_map[L"HY117"] = odbc_errc::E_HY117;
        code_map[L"HYT01"] = odbc_errc::E_HYT01;
        code_map[L"HYC00"] = odbc_errc::E_HYC00; //
        code_map[L"IM001"] = odbc_errc::E_IM001;
        code_map[L"IM017"] = odbc_errc::E_IM017;
        code_map[L"IM018"] = odbc_errc::E_IM018;
        code_map[L"42S22"] = odbc_errc::E_42S22;
    }

    std::error_code get_error_code(const wchar_t* state)
    {
        auto it = code_map.find(state);
        odbc_errc ec = (it == code_map.end()) ? odbc_errc::db_err : it->second;
        return make_error_code(ec);
    }
};
 
/************************************************************************ 
/* handle_diagnostic_record : display error/warning information 
/* 
/* Parameters: 
/*      hHandle     ODBC handle 
/*      hType       Type of handle (HANDLE_STMT, HANDLE_ENV, HANDLE_DBC) 
/*      RetCode     Return code of failing command 
/************************************************************************/ 
 
void handle_diagnostic_record(SQLHANDLE      hHandle,     
                              SQLSMALLINT    hType,   
                              RETCODE        RetCode,
                              std::error_code& ec) 
{ 
    static odbc_error_codes error_codes;

    SQLSMALLINT iRec = 0; 
    SQLINTEGER  iError; 
    WCHAR       wszMessage[1000]; 
    WCHAR       wszState[SQL_SQLSTATE_SIZE+1]; 
 
 
    if (RetCode == SQL_INVALID_HANDLE) 
    { 
        ec = make_error_code(odbc_errc::db_err);
        fwprintf(stderr, L"Invalid handle!\n"); 
        return; 
    } 
 
    while (SQLGetDiagRec(hType, 
                         hHandle, 
                         ++iRec, 
                         wszState, 
                         &iError, 
                         wszMessage, 
                         (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), 
                         (SQLSMALLINT *)NULL) == SQL_SUCCESS) 
    { 
        // Hide data truncated.. 
        if (!wcsncmp(wszState, L"01004", 5)) 
        { 
            //fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError); 
        }
        else
        {
            //fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError); 
            ec = error_codes.get_error_code(wszState);
            break;
        }
    } 
} 

void process_results(SQLHSTMT hstmt,
                     const std::function<void(const row& rec)>& callback,
                     std::error_code& ec)
{
    RETCODE rc;

    SQLSMALLINT numColumns; 
    rc = SQLNumResultCols(hstmt,&numColumns);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
        return;
    }
    //std::cout << "numColumns = " << numColumns << std::endl;
    std::vector<std::unique_ptr<value_impl>> values;
    values.reserve(numColumns);
    if (numColumns > 0) 
    { 
        for (SQLUSMALLINT col = 1; col <= numColumns; col++) 
        { 
            WCHAR name[SQL_MAX_COLUMN_NAME_LEN];

            SQLSMALLINT nameLength;
            SQLSMALLINT dataType;
            SQLULEN column_size;
            SQLSMALLINT decimalDigits;
            SQLSMALLINT nullable;
            SQLDescribeCol(hstmt,  
                           col,  
                           name,  
                           SQL_MAX_COLUMN_NAME_LEN,  
                           &nameLength,  
                           &dataType,  
                           &column_size,  
                           &decimalDigits,  
                           &nullable);  

            //std::wcout << std::wstring(&name[0],nameLength) << " column_size: " << column_size << " int32_t size: " << sizeof(int32_t) << std::endl;
            switch (dataType)
            {
            case SQL_DATE:
            case SQL_TYPE_DATE:
            case SQL_TYPE_TIMESTAMP:
                {
                    SQLLEN displaySize = 0; 
                    rc = SQLColAttribute(hstmt, 
                                         col, 
                                         SQL_DESC_DISPLAY_SIZE, 
                                         NULL, 
                                         0, 
                                         NULL, 
                                         &displaySize);
                    if (rc == SQL_ERROR)
                    {
                        handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                        return;
                    }
                    std::wcout << std::wstring(&name[0],nameLength) << L" " << L"date/timestamp" << std::endl;
                    values.push_back(std::make_unique<string_value>(std::wstring(name,nameLength),
                                                                    col,
                                                                    (SQLULEN)(displaySize),
                                                                    nullable));
                    values.back()->bind(hstmt, ec);
                }
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "SQL_TYPE_TIMESTAMP" << std::endl;
                break;
            case SQL_VARCHAR:
            case SQL_CHAR:
                std::wcout << std::wstring(&name[0],nameLength) << L" " << L"char" << std::endl;
                values.push_back(std::make_unique<string_value>(std::wstring(&name[0],nameLength),
                                                                col,
                                                                column_size,
                                                                nullable));
                values.back()->bind(hstmt, ec);
                break;
            case SQL_LONGVARCHAR:
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "wchar" << std::endl;
                values.push_back(std::make_unique<long_string_value>(std::wstring(&name[0],nameLength),
                                                                     col,
                                                                     nullable));
                values.back()->bind(hstmt, ec);
                break;
            case SQL_WVARCHAR:
            case SQL_WCHAR:
                //std::wcout << std::wstring(&name[0],nameLength) << L" " << L"wchar" << std::endl;
                values.push_back(std::make_unique<wstring_value>(std::wstring(&name[0],nameLength),
                                                                 col,
                                                                 column_size,
                                                                 nullable));
                values.back()->bind(hstmt, ec);
                break;
            case SQL_WLONGVARCHAR:
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "wchar" << std::endl;
                values.push_back(std::make_unique<long_wstring_value>(std::wstring(&name[0],nameLength),
                                                                      col,
                                                                      nullable));
                values.back()->bind(hstmt, ec);
                break;
            case SQL_SMALLINT:
            case SQL_TINYINT:
            case SQL_INTEGER:
            case SQL_BIGINT:
                //std::wcout << std::wstring(&name[0], nameLength) << " " << "BIGINT" << std::endl;
                values.push_back(std::make_unique<integer_value>(std::wstring(&name[0],nameLength),
                                                                 col,
                                                                 column_size,
                                                                 nullable));
                values.back()->bind(hstmt, ec);
                break;
            case SQL_DECIMAL:
            case SQL_NUMERIC:
            case SQL_REAL:
            case SQL_FLOAT:
            case SQL_DOUBLE:
                values.push_back(std::make_unique<double_value>(std::wstring(&name[0],nameLength),
                                                                col,
                                                                column_size,
                                                                nullable));
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "Float" << std::endl;
                values.back()->bind(hstmt, ec);
                break;
            default:
                //std::wcout << std::wstring(&name[0],nameLength) << ", dataType= " << dataType << std::endl;
                break;
            }
        }

    }  
    if (numColumns > 0) 
    { 
        bool fNoData = false; 

        std::vector<value*> cols;
        cols.reserve(values.size());
        for (auto& c : values)
        {
            cols.push_back(c.get());
        }

        row rec(std::move(cols));

        do { 
            // Fetch a row 

            rc = SQLFetch(hstmt);
            if (rc == SQL_ERROR)
            {
                ec = make_error_code(odbc_errc::db_err);
                return;
            }

            if (rc == SQL_SUCCESS) 
            { 
                // Get long data values here
                for (size_t i = 0; i < rec.size(); ++i)
                {
                    value_impl& c = static_cast<value_impl&>(rec[i]);
                    c.get_data(hstmt,ec);
                    if (ec)
                    {
                        return;
                    }
                }
                callback(rec);
            } 
            else 
            { 
                fNoData = true; 
            }

        } 
        while (!fNoData); 
        rc = SQLCloseCursor(hstmt);
    }
}

}}
