#include <sqlcons/sqlcons.hpp>
#include <windows.h> 
#include <sql.h> 
#include <sqlext.h> 
#include <stdio.h> 
#include <conio.h> 
#include <tchar.h> 
#include <stdlib.h> 
#include <iostream>
#include <sqlcons/unicode_traits.hpp>
#include <vector>

#define DISPLAY_MAX 50          // Arbitrary limit on column width to display 
#define DISPLAY_FORMAT_EXTRA 3  // Per column extra display bytes (| <data> ) 
#define DISPLAY_FORMAT      L"%c %*.*s " 
#define DISPLAY_FORMAT_C    L"%c %-*.*s " 
#define NULL_SIZE           6   // <NULL> 
#define PIPE                L'|' 
 
namespace sqlcons {

typedef struct STR_BINDING { 
    SQLSMALLINT         cDisplaySize;           /* size to display  */ 
    WCHAR               *wszBuffer;             /* display buffer   */ 
    SQLLEN              indPtr;                 /* size or null     */ 
    BOOL                fChar;                  /* character col?   */ 
    struct STR_BINDING  *sNext;                 /* linked list      */ 
} BINDING; 

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
    std::vector<WCHAR> stringValue_;
    double doubleValue_;
    int32_t integerValue_;
    SQLLEN indPtr;  // size or null

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
          doubleValue_(0.0)
    {
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
                 const std::function<void(const sql_record& record)>& callback,
                 std::error_code& ec);
};

// sql_query

class sql_query
{
    SQLHSTMT    hStmt_; 
public:
    sql_query()
        : hStmt_(nullptr)
    {
    }

    ~sql_query()
    {
        if (hStmt_) 
        { 
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt_); 
        } 
    }

    void execute(sql_connection::impl* conn, 
                 const std::string query, 
                 const std::function<void(const sql_record& record)>& callback,
                 std::error_code& ec);
};

void sql_connection::impl::execute(const std::string query, 
                                   const std::function<void(const sql_record& record)>& callback,
                                   std::error_code& ec)
{
    sql_query q;
    q.execute(this,query,callback,ec);
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
        handle_diagnostic_record (hEnv_, SQL_ATTR_ODBC_VERSION, rc, ec);
        return;
    }

    rc = SQLAllocHandle(SQL_HANDLE_DBC, hEnv_, &hDbc_);
    if (rc != SQL_SUCCESS)
    {
        handle_diagnostic_record (hEnv_, SQL_ATTR_ODBC_VERSION, rc, ec);
        return;
    }

    // Connect to the driver.  Use the sql_connection string if supplied 
    // on the input, otherwise let the driver manager prompt for input. 
    rc = SQLDriverConnect(hDbc_, 
                         NULL, 
                         &cs[0], 
                         cs.size(), 
                         NULL, 
                         0, 
                         NULL,
                         SQL_DRIVER_NOPROMPT);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record (hEnv_, SQL_ATTR_ODBC_VERSION, rc, ec);
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
                             const std::function<void(const sql_record& record)>& callback,
                             std::error_code& ec)
{
    pimpl_->execute(query, callback, ec);
}


void sql_query::execute(sql_connection::impl* conn, 
                        const std::string query, 
                        const std::function<void(const sql_record& record)>& callback,
                        std::error_code& ec)
{
    std::wstring buf;
    auto result1 = unicons::convert(query.begin(), query.end(),
                                    std::back_inserter(buf), 
                                    unicons::conv_flags::strict);

    RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, conn->hDbc_, &hStmt_);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(conn->hEnv_, SQL_ATTR_ODBC_VERSION, rc, ec);
        return;
    }

    rc = SQLExecDirect(hStmt_, &buf[0], (SQLINTEGER)buf.size()); 
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(conn->hEnv_, SQL_ATTR_ODBC_VERSION, rc, ec);
        return;
    }

    SQLSMALLINT numColumns; 
    rc = SQLNumResultCols(hStmt_,&numColumns);
    if (rc == SQL_ERROR)
    {
        handle_diagnostic_record(conn->hEnv_, SQL_ATTR_ODBC_VERSION, rc, ec);
        return;
    }
    std::vector<sql_column_impl> columns;
    if (numColumns > 0) 
    { 

        for (size_t col = 1; col <= numColumns; col++) 
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

            std::wcout << std::wstring(&name[0],nameLength) << " columnSize: " << columnSize << std::endl;
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
                break;
            case SQL_INTEGER:
                type = sql_data_type::string_t;
                std::wcout << std::wstring(&name[0], nameLength) << " " << "INTEGER" << std::endl;
                break;
            case SQL_BIGINT:
                type = sql_data_type::string_t;
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
                        &(columns.back().indPtr)); 
                    if (rc == SQL_ERROR)
                    {
                        handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
                        return;
                    }
                }
                break;
            case sql_data_type::integer_t:
                std::cout << "BOUND to INTEGER" << std::endl;
                rc = SQLBindCol(hStmt_,
                    col, 
                    SQL_INTEGER,
                    (SQLPOINTER)&(columns.back().integerValue_), 
                    sizeof(columns.back().integerValue_), 
                    &(columns.back().indPtr)); 
                if (rc == SQL_ERROR)
                {
                    handle_diagnostic_record(hStmt_, SQL_HANDLE_STMT, rc, ec);
                    return;
                }
                break;
            case sql_data_type::double_t:
                rc = SQLBindCol(hStmt_, 
                    col, 
                    //SQL_C_TCHAR, 
                    SQL_C_DOUBLE,
                    (SQLPOINTER)&(columns.back().doubleValue_), 
                    sizeof(double), 
                    &(columns.back().indPtr)); 
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
                for (size_t i = 0; i < columns.size(); ++i)
                {
                    switch (columns[i].sql_data_type_)
                    {
                    case sql_data_type::string_t:
                        std::wcout << std::wstring(&columns[i].stringValue_[0], columns[i].stringValue_.size()) << std::endl;
                        break;
                    case sql_data_type::double_t:
                        std::wcout << columns[i].doubleValue_ << std::endl; 
                        break;
                    case sql_data_type::integer_t:
                        std::wcout << columns[i].integerValue_ << std::endl; 
                        break;
                    }
                }
            }

            callback(record);
        } while (!fNoData); 
    }  
}

}