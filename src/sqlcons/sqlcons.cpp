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

namespace sqlcons {

const int sql_data_types::integer_id = SQL_INTEGER;
const int sql_data_types::string_id = SQL_WVARCHAR;

const int sql_c_data_types::integer_id = SQL_C_SLONG;
const int sql_c_data_types::string_id = SQL_C_WCHAR;

parameter<std::string>::parameter(int sql_type_identifier,int c_type_identifier, const std::string& value)
    : parameter_binding(sql_type_identifier, c_type_identifier)
{
    auto result1 = unicons::convert(value.begin(),value.end(),
                                    std::back_inserter(value_), 
                                    unicons::conv_flags::strict);
    ind_ = value_.size();
    value_.push_back(0);
}

void process_results(SQLHSTMT hstmt,
                     const std::function<void(const sql_record& record)>& callback,
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
        return "General warning";
    case sql_errc::E_08S01:
        return "Communication link failure";
    case sql_errc::E_HY000:
        return "General error";
    case sql_errc::E_HY001:
        return "Memory allocation error";
    case sql_errc::E_HY008:
        return "Operation canceled";
    case sql_errc::E_HY010:
        return "Function sequence error";
    case sql_errc::E_HY013:
        return "Memory management error";
    case sql_errc::E_HY117:
        return "Connection is suspended due to unknown transaction state";
    case sql_errc::E_HYT01:
        return "Connection timeout expired";
    case sql_errc::E_IM001:
        return "Driver does not support this function";
    case sql_errc::E_IM017:
        return "Polling is disabled in asynchronous notification mode";
    case sql_errc::E_IM018:
        return "SQLCompleteAsync has not been called to complete the previous asynchronous operation on this handle. If the previous function call on the handle returns SQL_STILL_EXECUTING and if notification mode is enabled, SQLCompleteAsync must be called on the handle to do post-processing and complete the operation";
    default:
        return "db error";
    }
}

// sql_column_impl

enum class sql_data_type {wstring_t,string_t,integer_t,double_t};

class sql_column_impl : public sql_column
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

    sql_column_impl(std::wstring&& name,
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
            break;
        case sql_data_type::double_t:
            return doubleValue_;
            break;
        default:
            return 99;
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

// sql_record

sql_record::sql_record(std::vector<sql_column*>&& columns)
    : columns_(std::move(columns))
{
}

sql_record::~sql_record() = default;

size_t sql_record::size() const
{
    return columns_.size();
}

const sql_column& sql_record::operator[](size_t index) const
{
    return *columns_[index];
}

// sql_connection::impl

class sql_connection::impl
{
public:
    SQLHENV     henv_;
    SQLHDBC     hdbc_; 

    impl()
        : henv_(nullptr), hdbc_(nullptr)
    {
    }

    ~impl()
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

    void open(const std::string& connString, std::error_code& ec);
    void execute(const std::string query, 
                 std::error_code& ec);
    void execute(const std::string query, 
                 const std::function<void(const sql_record& record)>& callback,
                 std::error_code& ec);
};

// sql_statement

class sql_statement
{
    SQLHSTMT    hstmt_; 
public:
    sql_statement()
        : hstmt_(nullptr)
    {
    }

    ~sql_statement()
    {
        if (hstmt_) 
        { 
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt_); 
        } 
    }

    void execute(SQLHDBC hDbc, 
                 const std::string query, 
                 std::error_code& ec);

    void execute(SQLHDBC hDbc, 
                 const std::string query, 
                 const std::function<void(const sql_record& record)>& callback,
                 std::error_code& ec);
};

// sql_prepared_statement::impl

class sql_prepared_statement::impl
{
public:
    SQLHSTMT hstmt_; 

    impl()
        : hstmt_(nullptr)
    {
    }

    ~impl()
    {
        if (hstmt_) 
        { 
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt_); 
        } 
    }

    void prepare(SQLHDBC hDbc, const std::string& query, std::error_code& ec);

    void do_execute(std::vector<std::unique_ptr<parameter_binding>>& bindings, 
                    const std::function<void(const sql_record& record)>& callback,
                    std::error_code& ec);

    void do_execute(std::vector<std::unique_ptr<parameter_binding>>& bindings, 
                    std::error_code& ec);
};

void sql_prepared_statement::impl::do_execute(std::vector<std::unique_ptr<parameter_binding>>& bindings, 
                                              const std::function<void(const sql_record& record)>& callback,
                                              std::error_code& ec)
{
    RETCODE rc;

    std::vector<SQLLEN> lengths(bindings.size());

    for (size_t i = 0; i < bindings.size(); ++i)
    {
        lengths[i] = bindings[i]->buffer_length();
        std::cout << "column_size: " << bindings[i]->column_size() << std::endl;
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

void sql_prepared_statement::impl::do_execute(std::vector<std::unique_ptr<parameter_binding>>& bindings, 
                                              std::error_code& ec)
{
    RETCODE rc;

    std::vector<SQLLEN> lengths(bindings.size());

    for (size_t i = 0; i < bindings.size(); ++i)
    {
        lengths[i] = bindings[i]->buffer_length();
        std::cout << "column_size: " << bindings[i]->column_size() << std::endl;
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
}

void sql_prepared_statement::impl::prepare(SQLHDBC hDbc,
                                           const std::string& query,
                                           std::error_code& ec)
{
    std::wstring wquery;
    auto result1 = unicons::convert(query.begin(), query.end(),
                                    std::back_inserter(wquery), 
                                    unicons::conv_flags::strict);

    RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt_);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hDbc, SQL_HANDLE_DBC, rc, ec);
        return;
    }
    rc = SQLPrepare(hstmt_, &wquery[0], (SQLINTEGER)wquery.size()); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hstmt_, SQL_HANDLE_STMT, rc, ec);
        return;
    }

}

// sql_prepared_statement

sql_prepared_statement::sql_prepared_statement() : pimpl_(new impl()) {}

sql_prepared_statement::~sql_prepared_statement() = default;

void sql_prepared_statement::prepare(sql_connection& conn, const std::string& connString, std::error_code& ec)
{
    pimpl_->prepare(conn.pimpl_->hdbc_,connString, ec);
}

void sql_prepared_statement::do_execute(std::vector<std::unique_ptr<parameter_binding>>& bindings, 
                                        const std::function<void(const sql_record& record)>& callback,
                                        std::error_code& ec)
{
    pimpl_->do_execute(bindings, callback, ec);
}

void sql_prepared_statement::do_execute(std::vector<std::unique_ptr<parameter_binding>>& bindings, 
                                        std::error_code& ec)
{
    pimpl_->do_execute(bindings, ec);
}

// sql_connection::impl

void sql_connection::impl::execute(const std::string query, 
                                   const std::function<void(const sql_record& record)>& callback,
                                   std::error_code& ec)
{
    sql_statement q;
    q.execute(hdbc_,query,callback,ec);
}

void sql_connection::impl::execute(const std::string query, 
                                   std::error_code& ec)
{
    sql_statement q;
    q.execute(hdbc_,query,ec);
}

void sql_connection::impl::open(const std::string& connString, std::error_code& ec)
{
    RETCODE rc = SQLAllocHandle(SQL_HANDLE_ENV, 
                                SQL_NULL_HANDLE, 
                                &henv_);
    if (rc == SQL_ERROR)
    {
        ec = make_error_code(sql_errc::db_err);
        return;
    }

    std::wstring cs;
    auto result1 = unicons::convert(connString.begin(),connString.end(),
                                    std::back_inserter(cs), 
                                    unicons::conv_flags::strict);
    std::cout << connString << std::endl;
    std::wcout << cs << std::endl;

    WCHAR* pwszConnStr = L"Driver={SQL Server};Server=localhost;Database=master;Trusted_Connection=Yes;"; 
    std::wcout << pwszConnStr << std::endl;


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

    // Connect to the driver.  Use the sql_connection string if supplied 
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

// sql_connection

sql_connection::sql_connection() : pimpl_(new impl()) {}

sql_connection::~sql_connection() = default;

void sql_connection::open(const std::string& connString, std::error_code& ec)
{
    pimpl_->open(connString, ec);
}

void sql_connection::execute(const std::string query, 
                             std::error_code& ec)
{
    pimpl_->execute(query, ec);
}

void sql_connection::execute(const std::string query, 
                             const std::function<void(const sql_record& record)>& callback,
                             std::error_code& ec)
{
    pimpl_->execute(query, callback, ec);
}


void sql_statement::execute(SQLHDBC hDbc, 
                           const std::string query, 
                           const std::function<void(const sql_record& record)>& callback,
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

void sql_statement::execute(SQLHDBC hDbc, 
                            const std::string query, 
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

struct odbc_error_codes
{
    std::map<const wchar_t*,sql_errc> code_map;

    odbc_error_codes()
    {
        code_map[L"01000"] = sql_errc::E_01000;
        code_map[L"08S01"] = sql_errc::E_08S01;
        code_map[L"HY000"] = sql_errc::E_HY000;
        code_map[L"HY001"] = sql_errc::E_HY001;
        code_map[L"HY008"] = sql_errc::E_HY008;
        code_map[L"HY013"] = sql_errc::E_HY013;
        code_map[L"HY117"] = sql_errc::E_HY117;
        code_map[L"HYT01"] = sql_errc::E_HYT01;
        code_map[L"IM001"] = sql_errc::E_IM001;
        code_map[L"IM017"] = sql_errc::E_IM017;
        code_map[L"IM018"] = sql_errc::E_IM018;
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
 
void handle_diagnostic_record (SQLHANDLE      hHandle,     
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
        if (wcsncmp(wszState, L"01004", 5)) 
        { 
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError); 
        }
        else
        {
            ec = error_codes.get_error_code(wszState);
            break;
        }
    } 
} 

void process_results(SQLHSTMT hstmt,
                     const std::function<void(const sql_record& record)>& callback,
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
    std::cout << "numColumns = " << numColumns << std::endl;
    std::vector<sql_column_impl> columns;
    columns.reserve(numColumns);
    if (numColumns > 0) 
    { 
        for (SQLUSMALLINT col = 1; col <= numColumns; col++) 
        { 
            SQLSMALLINT columnNameLength = 100;

            // Figure out the length of the column name 
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

            std::wcout << std::wstring(&name[0],nameLength) << " columnSize: " << columnSize << " int32_t size: " << sizeof(int32_t) << std::endl;
            columns.push_back(
                sql_column_impl(std::wstring(&name[0],nameLength),
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
                std::wcout << std::wstring(&name[0],nameLength) << " " << "Date" << std::endl;
                break;
            case SQL_TYPE_TIME:
                type = sql_data_type::string_t;
                std::wcout << std::wstring(&name[0],nameLength) << " " << "Time" << std::endl;
                break;
            case SQL_TYPE_TIMESTAMP:
                type = sql_data_type::string_t;
                std::wcout << std::wstring(&name[0],nameLength) << " " << "SQL_TYPE_TIMESTAMP" << std::endl;
                break;
            case SQL_SMALLINT:
            case SQL_TINYINT:
            case SQL_INTEGER:
            case SQL_BIGINT:
                type = sql_data_type::integer_t;
                std::wcout << std::wstring(&name[0], nameLength) << " " << "BIGINT" << std::endl;
                break;
            case SQL_VARCHAR:
            case SQL_CHAR:
                type = sql_data_type::string_t;
                std::wcout << std::wstring(&name[0],nameLength) << " " << "wchar" << std::endl;
                break;
            case SQL_WVARCHAR:
            case SQL_WCHAR:
                type = sql_data_type::wstring_t;
                std::wcout << std::wstring(&name[0],nameLength) << " " << "wchar" << std::endl;
                break;
            case SQL_DECIMAL:
            case SQL_NUMERIC:
            case SQL_REAL:
            case SQL_FLOAT:
            case SQL_DOUBLE:
                type = sql_data_type::double_t;
                std::wcout << std::wstring(&name[0],nameLength) << " " << "Float" << std::endl;
                break;
            default:
                std::wcout << std::wstring(&name[0],nameLength) << " " << dataType << std::endl;
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
                    columns.back().string_value_.resize(size);
                    rc = SQLBindCol(hstmt, 
                        col, 
                        SQL_C_CHAR, 
                        (SQLPOINTER)&(columns.back().string_value_[0]), 
                        size, 
                        &(columns.back().length_or_null_)); 
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
                    columns.back().wstring_value_.resize(size);
                    rc = SQLBindCol(hstmt, 
                        col, 
                        SQL_C_WCHAR, 
                        (SQLPOINTER)&(columns.back().wstring_value_[0]), 
                        size, 
                        &(columns.back().length_or_null_)); 
                    if (rc == SQL_ERROR)
                    {
                        handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                        return;
                    }
                }
                break;
            case sql_data_type::integer_t:
                std::cout << "BIND TO INT" << std::endl;
                rc = SQLBindCol(hstmt,
                    col, 
                    SQL_C_ULONG,
                    (SQLPOINTER)&(columns.back().integer_value_), 
                    0, 
                    &(columns.back().length_or_null_)); 
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
                    (SQLPOINTER)&(columns.back().doubleValue_), 
                    0, 
                    &(columns.back().length_or_null_)); 
                if (rc == SQL_ERROR)
                {
                    handle_diagnostic_record(hstmt, SQL_HANDLE_STMT, rc, ec);
                    return;
                }
                break;
            }
            std::wcout << "column name:" << std::wstring(&name[0], nameLength) << ", length:" << columnNameLength << std::endl;

            columns.back().sql_data_type_ = type;

        }

    }  
    if (numColumns > 0) 
    { 
        bool fNoData = false; 

        std::vector<sql_column*> cols;
        cols.reserve(columns.size());
        for (auto& c : columns)
        {
            cols.push_back(&c);
        }

        sql_record record(std::move(cols));

        do { 
            // Fetch a row 

            rc = SQLFetch(hstmt);
            if (rc == SQL_ERROR)
            {
                ec = make_error_code(sql_errc::db_err);
                return;
            }
            if (rc == SQL_NO_DATA_FOUND) 
            { 
                fNoData = true; 
            } 
            else 
            { 
                callback(record);
            }

        } while (!fNoData); 
    }  
}

}
