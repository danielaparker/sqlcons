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

const int sql_data_types::integer_id = SQL_BIGINT;

const int sql_c_data_types::integer_id = SQL_C_SBIGINT;

void handle_diagnostic_record(SQLHANDLE hHandle,
                              SQLSMALLINT hType,
                              RETCODE RetCode,
                              std::error_code& ec);

const std::error_category& sqlcons_error_category()
{
  static sqlcons_error_category_impl instance;
  return instance;
}

std::error_code make_error_code(sql_errc result)
{
    return std::error_code(static_cast<int>(result),sqlcons_error_category());
}

// sql_column_impl

enum class sql_data_type {string_t,integer_t,double_t};

class sql_column_impl : public sql_column
{
public:
    std::wstring name_;
    SQLSMALLINT dataType_;
    SQLULEN columnSize_;
    SQLSMALLINT decimalDigits_;
    SQLSMALLINT nullable_;

    sql_data_type sql_data_type_;
    long integerValue_;
    std::vector<WCHAR> stringValue_;
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
          integerValue_(0),
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
        case sql_data_type::string_t:
            if (is_null())
            {
                return L"";
            }
            else
            {
                size_t len = length_or_null_/sizeof(wchar_t);
                return std::wstring(stringValue_.data(), stringValue_.data() + len);
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
        case sql_data_type::string_t:
            if (is_null())
            {
                return "";
            }
            else
            {
                size_t len = length_or_null_/sizeof(wchar_t);
                std::string s;
                auto result1 = unicons::convert(stringValue_.begin(),stringValue_.begin() + len,
                                                std::back_inserter(s), 
                                                unicons::conv_flags::strict);
                return s;
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
            return (double)integerValue_;
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
            return integerValue_;
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
    SQLHENV     hEnv_;
    SQLHDBC     hDbc_; 

    impl()
        : hEnv_(nullptr), hDbc_(nullptr)
    {
    }

    ~impl()
    {
        if (hDbc_) 
        { 
            SQLDisconnect(hDbc_); 
            SQLFreeHandle(SQL_HANDLE_DBC, hDbc_); 
        } 

        if (hEnv_) 
        { 
            SQLFreeHandle(SQL_HANDLE_ENV, hEnv_); 
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
    SQLHSTMT    hStmt_; 
public:
    sql_statement()
        : hStmt_(nullptr)
    {
    }

    ~sql_statement()
    {
        if (hStmt_) 
        { 
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt_); 
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
    SQLHSTMT hStmt_; 

    impl()
        : hStmt_(nullptr)
    {
    }

    ~impl()
    {
        if (hStmt_) 
        { 
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt_); 
        } 
    }

    void prepare(SQLHDBC hDbc, const std::string& query, std::error_code& ec);
};

void sql_prepared_statement::impl::prepare(SQLHDBC hDbc,
                                           const std::string& query,
                                           std::error_code& ec)
{
    std::wstring wquery;
    auto result1 = unicons::convert(query.begin(), query.end(),
                                    std::back_inserter(wquery), 
                                    unicons::conv_flags::strict);

    RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt_);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hDbc, SQL_HANDLE_DBC, rc, ec);
        return;
    }
    rc = SQLPrepare(hStmt_, &wquery[0], (SQLINTEGER)wquery.size()); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
        return;
    }
}

// sql_prepared_statement

sql_prepared_statement::sql_prepared_statement() : pimpl_(new impl()) {}

sql_prepared_statement::~sql_prepared_statement() = default;

void sql_prepared_statement::prepare(sql_connection& conn, const std::string& connString, std::error_code& ec)
{
    pimpl_->prepare(conn.pimpl_->hDbc_,connString, ec);
}

// sql_connection::impl

void sql_connection::impl::execute(const std::string query, 
                                   const std::function<void(const sql_record& record)>& callback,
                                   std::error_code& ec)
{
    sql_statement q;
    q.execute(hDbc_,query,callback,ec);
}

void sql_connection::impl::execute(const std::string query, 
                                   std::error_code& ec)
{
    sql_statement q;
    q.execute(hDbc_,query,ec);
}

void sql_connection::impl::open(const std::string& connString, std::error_code& ec)
{
    RETCODE rc = SQLAllocHandle(SQL_HANDLE_ENV, 
                                SQL_NULL_HANDLE, 
                                &hEnv_);
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

    rc = SQLSetEnvAttr(hEnv_, 
                       SQL_ATTR_ODBC_VERSION, 
                       (SQLPOINTER)SQL_OV_ODBC3, 
                       0);
    if (rc != SQL_SUCCESS)
    {
        handle_diagnostic_record(hEnv_, SQL_HANDLE_ENV, rc, ec);
        return;
    }

    rc = SQLAllocHandle(SQL_HANDLE_DBC, hEnv_, &hDbc_);
    if (rc != SQL_SUCCESS)
    {
        handle_diagnostic_record (hEnv_, SQL_HANDLE_ENV, rc, ec);
        return;
    }

    // Connect to the driver.  Use the sql_connection string if supplied 
    // on the input, otherwise let the driver manager prompt for input. 
    rc = SQLDriverConnect(hDbc_, 
                         NULL, 
                         &cs[0], 
                         (SQLSMALLINT)cs.size(), 
                         NULL, 
                         0, 
                         NULL,
                         SQL_DRIVER_NOPROMPT);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record (hDbc_, SQL_HANDLE_DBC, rc, ec);
        return;
    }
}

 
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
    } 
      
    ec = make_error_code(sql_errc::db_err);

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

    RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt_);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hDbc, SQL_HANDLE_DBC, rc, ec);
        return;
    }

    rc = SQLExecDirect(hStmt_, &buf[0], (SQLINTEGER)buf.size()); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
        return;
    }

    SQLSMALLINT numColumns; 
    rc = SQLNumResultCols(hStmt_,&numColumns);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
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
            rc = SQLColAttribute(hStmt_, 
                                 col, 
                                 SQL_DESC_NAME, 
                                 NULL, 
                                 0, 
                                 &columnNameLength, 
                                 NULL); 
            if (rc == SQL_ERROR)
            {
                handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
                return;
            }
            columnNameLength /= sizeof(WCHAR);
            std::vector<WCHAR> name(columnNameLength+1);

            SQLSMALLINT nameLength;
            SQLSMALLINT dataType;
            SQLULEN columnSize;
            SQLSMALLINT decimalDigits;
            SQLSMALLINT nullable;
            SQLDescribeCol(hStmt_,  
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
            case SQL_WVARCHAR:
            case SQL_VARCHAR:
            case SQL_WCHAR:
                type = sql_data_type::string_t;
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

                    rc = SQLColAttribute(hStmt_, 
                                         col, 
                                         SQL_DESC_DISPLAY_SIZE, 
                                         NULL, 
                                         0, 
                                         NULL, 
                                         &cchDisplay);
                    if (rc == SQL_ERROR)
                    {
                        handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
                        return;
                    }

                    SQLLEN size = (cchDisplay + 1) * sizeof(WCHAR);
                    columns.back().stringValue_.resize(size);
                    rc = SQLBindCol(hStmt_, 
                        col, 
                        SQL_C_WCHAR, 
                        (SQLPOINTER)&(columns.back().stringValue_[0]), 
                        size, 
                        &(columns.back().length_or_null_)); 
                    if (rc == SQL_ERROR)
                    {
                        handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
                        return;
                    }
                }
                break;
            case sql_data_type::integer_t:
                std::cout << "BIND TO INT" << std::endl;
                rc = SQLBindCol(hStmt_,
                    col, 
                    SQL_C_ULONG,
                    (SQLPOINTER)&(columns.back().integerValue_), 
                    0, 
                    &(columns.back().length_or_null_)); 
                if (rc == SQL_ERROR)
                {
                    handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
                    return;
                }
                break;
            case sql_data_type::double_t:
                rc = SQLBindCol(hStmt_, 
                    col, 
                    SQL_C_DOUBLE,
                    (SQLPOINTER)&(columns.back().doubleValue_), 
                    0, 
                    &(columns.back().length_or_null_)); 
                if (rc == SQL_ERROR)
                {
                    handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
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

            rc = SQLFetch(hStmt_);
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

void sql_statement::execute(SQLHDBC hDbc, 
                            const std::string query, 
                            std::error_code& ec)
{
    std::wstring buf;
    auto result1 = unicons::convert(query.begin(), query.end(),
                                    std::back_inserter(buf), 
                                    unicons::conv_flags::strict);

    RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt_);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hDbc, SQL_HANDLE_DBC, rc, ec);
        return;
    }

    rc = SQLExecDirect(hStmt_, &buf[0], (SQLINTEGER)buf.size()); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
        return;
    }
}

}
