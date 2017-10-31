#include <sqlcons/sqlcons.hpp>
#include <windows.h> 
//#include <sql.h> 
#define UNICODE  
#include <sqlext.h> 
#include <stdio.h> 
#include <conio.h> 
#include <tchar.h> 
#include <stdlib.h> 
#include <iostream>
#include <sqlcons/unicode_traits.hpp>
#include <vector>
#include <sstream>
#include <sqlcons_connector/odbc/connector.hpp>

namespace sqlcons {

const int sql_data_types::smallint_id = SQL_SMALLINT;
const int sql_data_types::integer_id = SQL_INTEGER;
const int sql_data_types::string_id = SQL_WVARCHAR;

const int sql_c_data_types::smallint_id = SQL_C_SSHORT;
const int sql_c_data_types::integer_id = SQL_C_SLONG;
const int sql_c_data_types::string_id = SQL_C_WCHAR;

parameter<std::string>::parameter(int sql_type_identifier,int c_type_identifier, const std::string& val)
    : base_parameter(sql_type_identifier, c_type_identifier)
{
    auto result1 = unicons::convert(val.begin(),val.end(),
                                    std::back_inserter(value_), 
                                    unicons::conv_flags::strict);
    ind_ = value_.size();
    value_.push_back(0);

    //std::cout << "parameter<std::string> "
    //          << "sql_type_identifier: " << sql_type_identifier
    //          << ", c_type_identifier: " << c_type_identifier
    //          << ", val: " << val
    //          << std::endl;
}

void process_results(SQLHSTMT hstmt,
                     const std::function<void(const row& rec)>& callback,
                     std::error_code& ec);

void handle_diagnostic_record(SQLHANDLE hHandle,
                              SQLSMALLINT hType,
                              RETCODE RetCode,
                              std::error_code& ec);

// sqlcons_error_category_impl

const std::error_category& sqlcons_error_category()
{
  static sqlcons_error_category_impl instance;
  return instance;
}

std::error_code make_error_code(sql_errc result)
{
    return std::error_code(static_cast<int>(result),sqlcons_error_category());
}

std::string sqlcons_error_category_impl::message(int ev) const
{
    switch (static_cast<sql_errc>(ev))
    {
    case sql_errc::E_01000:
        return "[01000] General warning";
    case sql_errc::E_01S02:
        return "[01S02] Option value changed";
    case sql_errc::E_01001:
        return "[01001] Cursor operation conflict";
    case sql_errc::E_01003:
        return "[01003] Cursor operation conflict";
    case sql_errc::E_07002:
        return "[07002] COUNT field incorrect";
    case sql_errc::E_07006:
        return "[07006] Restricted data type attribute violation";
    case sql_errc::E_07007:
        return "[07007] Restricted parameter value violation";
    case sql_errc::E_07S01:
        return "[07S01] Invalid use of default parameter";
    case sql_errc::E_08S01:
        return "[08S01] Communication link failure";
    case sql_errc::E_21S02:
        return "[21S02] Degree of derived table does not match column list";
    case sql_errc::E_22001:
        return "[22001] String data, right truncation";
    case sql_errc::E_22002:
        return "[22002] Indicator variable required but not supplied";
    case sql_errc::E_22003:
        return "[22003] Numeric value out of range";
    case sql_errc::E_22007:
        return "[22007] Invalid datetime format";
    case sql_errc::E_22008:
        return "Datetime field overflow";
    case sql_errc::E_22012:
        return "[22012] Division by zero";
    case sql_errc::E_22015:
        return "[22015] Interval field overflow";
    case sql_errc::E_22018:
        return "[22018] Interval field overflowInvalid character value for cast specification";
    case sql_errc::E_22019:
        return "[22019] Invalid escape character";
    case sql_errc::E_22025:
        return "[22025] Invalid escape sequence";
    case sql_errc::E_23000:
        return "[23000] Integrity constraint violation";
    case sql_errc::E_24000:
        return "[24000] Invalid cursor state";
    case sql_errc::E_40001:
        return "[40001] Serialization failure";
    case sql_errc::E_40003:
        return "[40003] Statement completion unknown";
    case sql_errc::E_42000:
        return "[42000] Syntax error or access violation";
    case sql_errc::E_44000:
        return "[44000] WITH CHECK OPTION violation";
    case sql_errc::E_HY000:
        return "[HY000] General error";
    case sql_errc::E_HY001:
        return "[HY001] Memory allocation error";
    case sql_errc::E_HY008:
        return "[HY008] Operation canceled";
    case sql_errc::E_HY009:
        return "[HY009] Invalid use of null pointer";
    case sql_errc::E_HY010:
        return "[HY010] Function sequence error";
    case sql_errc::E_HY013:
        return "[HY013] Memory management error";
    case sql_errc::E_HY024:
        return "[HY024] Invalid attribute value";
    case sql_errc::E_HY090:
        return "[HY090] Invalid string or buffer length";
    case sql_errc::E_HY092:
        return "[HY092] Invalid attribute/option identifier";
    case sql_errc::E_HY104:
        return "[HY104] Invalid precision or scale value";
    case sql_errc::E_HY117:
        return "[HY117] Connection is suspended due to unknown transaction state";
    case sql_errc::E_HYT01:
        return "[HYT01] Connection timeout expired";
    case sql_errc::E_HYC00:
        return "[HYC00] Optional feature not implemented";
    case sql_errc::E_IM001:
        return "[IM001] Driver does not support this function";
    case sql_errc::E_IM017:
        return "[IM017] Polling is disabled in asynchronous notification mode";
    case sql_errc::E_IM018:
        return "[IM018] SQLCompleteAsync has not been called to complete the previous asynchronous operation on this handle. If the previous function call on the handle returns SQL_STILL_EXECUTING and if notification mode is enabled, SQLCompleteAsync must be called on the handle to do post-processing and complete the operation";
    case sql_errc::E_42S22:
        return "[E_42S22] Column not found";
    default:
        return "db error";
    }
}

// data_value_impl

enum class sql_data_type {wstring_t,string_t,integer_t,double_t};

class data_value_impl : public value
{
public:
    std::wstring name_;
    SQLSMALLINT dataType_;
    SQLULEN columnSize_;
    SQLSMALLINT decimalDigits_;
    SQLSMALLINT nullable_;

    sql_data_type sql_data_type_;
    long integer_value_;
    std::vector<WCHAR> wstring_value_;
    std::vector<CHAR> string_value_;
    double doubleValue_;
    SQLLEN length_or_null_;  // size or null

    data_value_impl(std::wstring&& name,
                    SQLSMALLINT dataType,
                    SQLULEN columnSize,
                    SQLSMALLINT decimalDigits,
                    SQLSMALLINT nullable)
        : name_(name),
          dataType_(dataType),
          columnSize_(columnSize),
          decimalDigits_(decimalDigits),
          nullable_(nullable),
          integer_value_(0),
          doubleValue_(0.0),
          length_or_null_(0)
    {
    }

    bool is_null() const
    {
        return length_or_null_ == SQL_NULL_DATA;
    }

    std::wstring as_wstring() const override
    {
        switch (sql_data_type_)
        {
        case sql_data_type::wstring_t:
            if (is_null())
            {
                return L"";
            }
            else
            {
                size_t len = length_or_null_/sizeof(wchar_t);
                return std::wstring(wstring_value_.data(), wstring_value_.data() + len);
            }
            break;
        case sql_data_type::string_t:
            if (is_null())
            {
                return L"";
            }
            else
            {
                size_t len = length_or_null_;
                std::wstring s;
                auto result1 = unicons::convert(string_value_.begin(),string_value_.begin() + len,
                                                std::back_inserter(s), 
                                                unicons::conv_flags::strict);
                return s;
            }
            break;
        default:
            return L"";
        }
    }

    std::string as_string() const override
    {
        switch (sql_data_type_)
        {
        case sql_data_type::wstring_t:
            if (is_null())
            {
                return "";
            }
            else
            {
                size_t len = length_or_null_/sizeof(wchar_t);
                std::string s;
                auto result1 = unicons::convert(wstring_value_.begin(),wstring_value_.begin() + len,
                                                std::back_inserter(s), 
                                                unicons::conv_flags::strict);
                return s;
            }
            break;
        case sql_data_type::string_t:
            if (is_null())
            {
                return "";
            }
            else
            {
                size_t len = length_or_null_;
                return std::string(string_value_.data(), string_value_.data() + len);
            }
            break;
        default:
            return "";
        }
    }

    double as_double() const override
    {
        switch (sql_data_type_)
        {
        case sql_data_type::integer_t:
            return (double)integer_value_;
        case sql_data_type::double_t:
            return doubleValue_;
        case sql_data_type::string_t:
            {
                size_t len = length_or_null_;
                std::istringstream is(std::string(string_value_.data(), string_value_.data() + len));
                double d;
                is >> d;
                return d;
            }
        case sql_data_type::wstring_t:
            {
                size_t len = length_or_null_;
                std::wistringstream is(std::wstring(wstring_value_.data(), wstring_value_.data() + len));
                double d;
                is >> d;
                return d;
            }
        default:
            return 0;
        }
    }

    long as_long() const override
    {
        switch (sql_data_type_)
        {
        case sql_data_type::integer_t:
            return integer_value_;
            break;
        case sql_data_type::double_t:
            return (long)doubleValue_;
            break;
        default:
            return 99;
        }
    }
};

// row

row::row(std::vector<value*>&& values)
    : values_(std::move(values))
{
}

row::~row() = default;

size_t row::size() const
{
    return values_.size();
}

const value& row::operator[](size_t index) const
{
    return *values_[index];
}

// connection_impl

class connection_impl
{
private:
    bool autoCommit_;
public:
    SQLHENV     henv_;
    SQLHDBC     hdbc_; 

    connection_impl()
        : henv_(nullptr), hdbc_(nullptr), autoCommit_(false)
    {
    }

    ~connection_impl()
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
public:
    transaction_impl(connection_impl* pimpl)
        : pimpl_(pimpl)
    {
        std::error_code ec;
        pimpl_->auto_commit(false,ec);
        update_error_code(ec);
    }
    ~transaction_impl()
    {
        std::error_code ec;
        end(ec);
    }

    std::error_code error_code() const
    {
        return ec_;
    }

    void update_error_code(std::error_code ec)
    {
        if (ec)
        {
            ec_ = ec;
        }
    }

    void end(std::error_code& ec)
    {
        if (pimpl_ != nullptr)
        {
            if (ec_)
            {
                pimpl_->rollback(ec);
            }
            else
            {
                pimpl_->commit(ec);
            }
            pimpl_ = nullptr;
        }
    }
private:
    connection_impl* pimpl_;
    std::error_code ec_;
};

// statement

class statement
{
    SQLHSTMT hstmt_; 
public:
    statement()
        : hstmt_(nullptr)
    {
    }

    ~statement()
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

// prepared_statement_impl

class prepared_statement_impl
{
public:
    SQLHSTMT hstmt_; 

    prepared_statement_impl()
        : hstmt_(nullptr)
    {
    }

    prepared_statement_impl(SQLHSTMT hstmt)
        : hstmt_(hstmt)
    {
    }

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

void prepared_statement_impl::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                          const std::function<void(const row& rec)>& callback,
                                          std::error_code& ec)
{
    RETCODE rc;

    std::vector<SQLLEN> lengths(bindings.size());

    for (size_t i = 0; i < bindings.size(); ++i)
    {
        lengths[i] = bindings[i]->buffer_length();
        //std::cout << "column_size: " << bindings[i]->column_size() << std::endl;
        rc = SQLBindParameter(hstmt_, i+1, SQL_PARAM_INPUT, bindings[i]->value_type(), bindings[i]->parameter_type(), bindings[i]->column_size(), 0,
                              bindings[i]->pvalue(), bindings[i]->buffer_capacity(), &lengths[i]);
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

void prepared_statement_impl::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                          std::error_code& ec)
{
    RETCODE rc;

    std::vector<SQLLEN> lengths(bindings.size());

    for (size_t i = 0; i < bindings.size(); ++i)
    {
        lengths[i] = bindings[i]->buffer_length();
        //std::cout << "column_size: " << bindings[i]->column_size() << std::endl;
        rc = SQLBindParameter(hstmt_, i+1, SQL_PARAM_INPUT, bindings[i]->value_type(), bindings[i]->parameter_type(), bindings[i]->column_size(), 0,
                              bindings[i]->pvalue(), bindings[i]->buffer_capacity(), &lengths[i]);
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

void prepared_statement_impl::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                          transaction& t)
{
    if (!t.error_code())
    {
        std::error_code ec;
        execute_(bindings,ec);
        t.update_error_code(ec);
    }
}

// prepared_statement

prepared_statement::prepared_statement() : pimpl_(new prepared_statement_impl()) {}

prepared_statement::prepared_statement(std::unique_ptr<prepared_statement_impl>&& impl) : pimpl_(std::move(impl)) {}

prepared_statement::~prepared_statement() = default;

void prepared_statement::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                        const std::function<void(const row& rec)>& callback,
                                        std::error_code& ec)
{
    pimpl_->execute_(bindings, callback, ec);
}

void prepared_statement::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                        std::error_code& ec)
{
    pimpl_->execute_(bindings, ec);
}

void prepared_statement::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                  transaction& t)
{
    pimpl_->execute_(bindings, t);
}

// connection_impl

void connection_impl::auto_commit(bool val, std::error_code& ec)
{
    autoCommit_ = val;

    RETCODE rc;
    if (autoCommit_)
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

void connection_impl::connection_timeout(size_t val, std::error_code& ec)
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

void connection_impl::execute(const std::string& query, 
                               const std::function<void(const row& rec)>& callback,
                               std::error_code& ec)
{
    statement q;
    q.execute(hdbc_,query,callback,ec);
}

void connection_impl::execute(const std::string& query, 
                               std::error_code& ec)
{
    statement q;
    q.execute(hdbc_,query,ec);
}

void connection_impl::open(const std::string& connString, bool autoCommit, std::error_code& ec)
{
    autoCommit_ = autoCommit;

    RETCODE rc = SQLAllocHandle(SQL_HANDLE_ENV, 
                                SQL_NULL_HANDLE, 
                                &henv_);
    if (rc == SQL_ERROR)
    {
        ec = make_error_code(sql_errc::db_err);
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

    if (autoCommit)
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

prepared_statement connection_impl::prepare_statement(const std::string& query, transaction& trans)
{
    std::error_code ec;
    prepared_statement stat = prepare_statement(query,ec);
    if (ec)
    {
        trans.update_error_code(ec);
    }
    return std::move(stat);
}

prepared_statement connection_impl::prepare_statement(const std::string& query, std::error_code& ec)
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
        return prepared_statement();
    }
    rc = SQLPrepare(hstmt, &wquery[0], (SQLINTEGER)wquery.size()); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
        return prepared_statement();
    }

    return prepared_statement(std::make_unique<prepared_statement_impl>(hstmt));
}

void connection_impl::commit(std::error_code& ec)
{
    if (!autoCommit_)
    {
        RETCODE rc = SQLEndTran (SQL_HANDLE_DBC, hdbc_, SQL_COMMIT);
        if (rc == SQL_ERROR)
        {
            ec = make_error_code(sql_errc::db_err);
            return;
        }
    }
}

void connection_impl::rollback(std::error_code& ec)
{
    if (!autoCommit_)
    {
        RETCODE rc = SQLEndTran (SQL_HANDLE_DBC, hdbc_, SQL_ROLLBACK);
        if (rc == SQL_ERROR)
        {
            ec = make_error_code(sql_errc::db_err);
            return;
        }
    }
}

// connection

connection::connection() : pimpl_(new connection_impl()) {}

connection::~connection() = default;

void connection::open(const std::string& connString, bool autoCommit, std::error_code& ec)
{
    pimpl_->open(connString, autoCommit, ec);
}

void connection::auto_commit(bool val, std::error_code& ec)
{
    pimpl_->auto_commit(val, ec);
}

void connection::connection_timeout(size_t val, std::error_code& ec)
{
    pimpl_->connection_timeout(val, ec);
}

prepared_statement connection::prepare_statement(const std::string& query, std::error_code& ec)
{
    return pimpl_->prepare_statement(query, ec);
}

prepared_statement connection::prepare_statement(const std::string& query, transaction& trans)
{
    return pimpl_->prepare_statement(query, trans);
}

void connection::execute(const std::string& query, 
                         std::error_code& ec)
{
    pimpl_->execute(query, ec);
}

void connection::execute(const std::string& query, 
                         const std::function<void(const row& rec)>& callback,
                         std::error_code& ec)
{
    pimpl_->execute(query, callback, ec);
}


void statement::execute(SQLHDBC hDbc, 
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

void statement::execute(SQLHDBC hDbc, 
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

struct compare_states
{
    bool operator()(const wchar_t* key1, const wchar_t* key2) const
    {
        return wcsncmp(key1, key2, 5) < 0;
    }
};

struct odbc_error_codes
{
    std::map<const wchar_t*,sql_errc,compare_states> code_map;

    odbc_error_codes()
    {
        code_map[L"01000"] = sql_errc::E_01000;
        code_map[L"01S02"] = sql_errc::E_01S02; 
        code_map[L"01001"] = sql_errc::E_01001;
        code_map[L"01003"] = sql_errc::E_01003;
        code_map[L"07002"] = sql_errc::E_07002;
        code_map[L"07006"] = sql_errc::E_07006;
        code_map[L"07007"] = sql_errc::E_07007;
        code_map[L"07S01"] = sql_errc::E_07S01;
        code_map[L"08S01"] = sql_errc::E_08S01;
        code_map[L"21S02"] = sql_errc::E_21S02;
        code_map[L"22001"] = sql_errc::E_22001;
        code_map[L"22002"] = sql_errc::E_22002;
        code_map[L"22003"] = sql_errc::E_22003;
        code_map[L"22007"] = sql_errc::E_22007;
        code_map[L"22008"] = sql_errc::E_22008;
        code_map[L"22012"] = sql_errc::E_22012;
        code_map[L"22015"] = sql_errc::E_22015;
        code_map[L"22018"] = sql_errc::E_22018;
        code_map[L"22019"] = sql_errc::E_22019;
        code_map[L"22025"] = sql_errc::E_22025;
        code_map[L"23000"] = sql_errc::E_23000;
        code_map[L"24000"] = sql_errc::E_24000;
        code_map[L"40001"] = sql_errc::E_40001;
        code_map[L"40003"] = sql_errc::E_40003;
        code_map[L"42000"] = sql_errc::E_42000;
        code_map[L"44000"] = sql_errc::E_44000;
        code_map[L"HY000"] = sql_errc::E_HY000;
        code_map[L"HY001"] = sql_errc::E_HY001;
        code_map[L"HY008"] = sql_errc::E_HY008;
        code_map[L"HY009"] = sql_errc::E_HY009; //
        code_map[L"HY010"] = sql_errc::E_HY010;
        code_map[L"HY013"] = sql_errc::E_HY013;
        code_map[L"HY024"] = sql_errc::E_HY024; //
        code_map[L"HY090"] = sql_errc::E_HY090; //
        code_map[L"HY092"] = sql_errc::E_HY092; //
        code_map[L"HY104"] = sql_errc::E_HY104;
        code_map[L"HY117"] = sql_errc::E_HY117;
        code_map[L"HYT01"] = sql_errc::E_HYT01;
        code_map[L"HYC00"] = sql_errc::E_HYC00; //
        code_map[L"IM001"] = sql_errc::E_IM001;
        code_map[L"IM017"] = sql_errc::E_IM017;
        code_map[L"IM018"] = sql_errc::E_IM018;
        code_map[L"42S22"] = sql_errc::E_42S22;
    }

    std::error_code get_error_code(const wchar_t* state)
    {
        auto it = code_map.find(state);
        sql_errc ec = (it == code_map.end()) ? sql_errc::db_err : it->second;
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
        ec = make_error_code(sql_errc::db_err);
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
    std::vector<data_value_impl> values;
    values.reserve(numColumns);
    if (numColumns > 0) 
    { 
        for (SQLUSMALLINT col = 1; col <= numColumns; col++) 
        { 
            SQLSMALLINT columnNameLength = 100;

            // Figure out the length of the value name 
            rc = SQLColAttribute(hstmt, 
                                 col, 
                                 SQL_DESC_NAME, 
                                 NULL, 
                                 0, 
                                 &columnNameLength, 
                                 NULL); 
            if (rc == SQL_ERROR)
            {
                handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                return;
            }
            columnNameLength /= sizeof(WCHAR);
            std::vector<WCHAR> name(columnNameLength+1);

            SQLSMALLINT nameLength;
            SQLSMALLINT dataType;
            SQLULEN columnSize;
            SQLSMALLINT decimalDigits;
            SQLSMALLINT nullable;
            SQLDescribeCol(hstmt,  
                           col,  
                           &name[0],  
                           columnNameLength+1,  
                           &nameLength,  
                           &dataType,  
                           &columnSize,  
                           &decimalDigits,  
                           &nullable);  

            //std::wcout << std::wstring(&name[0],nameLength) << " columnSize: " << columnSize << " int32_t size: " << sizeof(int32_t) << std::endl;
            values.push_back(
                data_value_impl(std::wstring(&name[0],nameLength),
                                dataType,
                                columnSize,
                                decimalDigits,
                                nullable)
            );
            sql_data_type type;

            switch (dataType)
            {
            case SQL_DATE:
            case SQL_TYPE_DATE:
                type = sql_data_type::string_t;
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "Date" << std::endl;
                break;
            case SQL_TYPE_TIME:
                type = sql_data_type::string_t;
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "Time" << std::endl;
                break;
            case SQL_TYPE_TIMESTAMP:
                type = sql_data_type::string_t;
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "SQL_TYPE_TIMESTAMP" << std::endl;
                break;
            case SQL_SMALLINT:
            case SQL_TINYINT:
            case SQL_INTEGER:
            case SQL_BIGINT:
                type = sql_data_type::integer_t;
                //std::wcout << std::wstring(&name[0], nameLength) << " " << "BIGINT" << std::endl;
                break;
            case SQL_VARCHAR:
            case SQL_CHAR:
                type = sql_data_type::string_t;
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "wchar" << std::endl;
                break;
            case SQL_WVARCHAR:
            case SQL_WLONGVARCHAR:
            case SQL_WCHAR:
                type = sql_data_type::wstring_t;
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "wchar" << std::endl;
                break;
            case SQL_DECIMAL:
            case SQL_NUMERIC:
            case SQL_REAL:
            case SQL_FLOAT:
            case SQL_DOUBLE:
                type = sql_data_type::double_t;
                //std::wcout << std::wstring(&name[0],nameLength) << " " << "Float" << std::endl;
                break;
            default:
                //std::wcout << std::wstring(&name[0],nameLength) << ", dataType= " << dataType << std::endl;
                break;
            }
            switch (type)
            {
            case sql_data_type::string_t:
                {
                    RETCODE rc;
                    SQLLEN cchDisplay; 

                    rc = SQLColAttribute(hstmt, 
                                         col, 
                                         SQL_DESC_DISPLAY_SIZE, 
                                         NULL, 
                                         0, 
                                         NULL, 
                                         &cchDisplay);
                    if (rc == SQL_ERROR)
                    {
                        handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                        return;
                    }

                    SQLLEN size = cchDisplay + 1;
                    values.back().string_value_.resize(size);
                    rc = SQLBindCol(hstmt, 
                        col, 
                        SQL_C_CHAR, 
                        (SQLPOINTER)&(values.back().string_value_[0]), 
                        size, 
                        &(values.back().length_or_null_)); 
                    if (rc == SQL_ERROR)
                    {
                        handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                        return;
                    }
                }
                break;
            case sql_data_type::wstring_t:
                {
                    RETCODE rc;
                    SQLLEN cchDisplay; 

                    rc = SQLColAttribute(hstmt, 
                                         col, 
                                         SQL_DESC_DISPLAY_SIZE, 
                                         NULL, 
                                         0, 
                                         NULL, 
                                         &cchDisplay);
                    if (rc == SQL_ERROR)
                    {
                        handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                        return;
                    }

                    SQLLEN size = (cchDisplay + 1) * sizeof(WCHAR);
                    values.back().wstring_value_.resize(size);
                    rc = SQLBindCol(hstmt, 
                        col, 
                        SQL_C_WCHAR, 
                        (SQLPOINTER)&(values.back().wstring_value_[0]), 
                        size, 
                        &(values.back().length_or_null_)); 
                    if (rc == SQL_ERROR)
                    {
                        handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                        return;
                    }
                }
                break;
            case sql_data_type::integer_t:
                //std::cout << "BIND TO INT" << std::endl;
                rc = SQLBindCol(hstmt,
                    col, 
                    SQL_C_ULONG,
                    (SQLPOINTER)&(values.back().integer_value_), 
                    0, 
                    &(values.back().length_or_null_)); 
                if (rc == SQL_ERROR)
                {
                    handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                    return;
                }
                break;
            case sql_data_type::double_t:
                rc = SQLBindCol(hstmt, 
                    col, 
                    SQL_C_DOUBLE,
                    (SQLPOINTER)&(values.back().doubleValue_), 
                    0, 
                    &(values.back().length_or_null_)); 
                if (rc == SQL_ERROR)
                {
                    handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                    return;
                }
                break;
            }
            //std::wcout << "value name:" << std::wstring(&name[0], nameLength) << ", length:" << columnNameLength << std::endl;

            values.back().sql_data_type_ = type;

        }

    }  
    if (numColumns > 0) 
    { 
        bool fNoData = false; 

        std::vector<value*> cols;
        cols.reserve(values.size());
        for (auto& c : values)
        {
            cols.push_back(&c);
        }

        row rec(std::move(cols));

        do { 
            // Fetch a row 

            rc = SQLFetch(hstmt);
            if (rc == SQL_ERROR)
            {
                ec = make_error_code(sql_errc::db_err);
                return;
            }
            if (rc == SQL_SUCCESS) 
            { 
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

// transaction

transaction::transaction(connection& conn) 
    : pimpl_(new transaction_impl(conn.pimpl_.get()))
{
    std::error_code ec;
    if (ec)
    {
        update_error_code(ec);
    }
}

transaction::~transaction() = default;

std::error_code transaction::error_code() const
{
    return pimpl_->error_code();
}

void transaction::update_error_code(std::error_code ec)
{
    pimpl_->update_error_code(ec);
}

void transaction::end(std::error_code& ec)
{
    pimpl_->end(ec);
}

}
