# sqlcons for C++

## Query example

```c++
#include <sqlcons_connector/odbc/connector.hpp>
#include <sqlcons/sqlcons.hpp>

int main()
{
    std::error_code ec;

    const std::string databaseUrl = "Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;";

    sqlcons::connection_pool<sqlcons::odbc::odbc_connector> connection_pool(databaseUrl,2);

    auto connection = connection_pool.get_connection(ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    std::string query = "SELECT I.ticker, P.observation_date, P.close_price FROM equity_price P JOIN equity I ON P.instrument_id = I.instrument_id WHERE I.ticker='IBM'";

    auto action = [](const sqlcons::row& row)
    {
        std::wcout << row[0].as_wstring() << " " 
                  << row[1].as_wstring() << " " 
                  << row[2].as_double()  
                  << std::endl;
    };

    connection.execute(query, action, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }
} 
```

## Prepared statement example 1

```c++
#include <sqlcons_connector/odbc/connector.hpp>
#include <sqlcons/sqlcons.hpp>

int main()
{
    std::error_code ec;

    const std::string databaseUrl = "Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;";

    sqlcons::connection_pool<sqlcons::odbc::odbc_connector> connection_pool(databaseUrl,2);

    auto connection = connection_pool.get_connection(ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    std::string sql = "SELECT I.ticker, P.observation_date, P.close_price FROM equity_price P JOIN equity I ON P.instrument_id = I.instrument_id WHERE I.ticker=?";
    auto statment = make_prepared_statement(connection, sql, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    jsoncons::json parameters = jsoncons::json::array();
    parameters.push_back("IBM");

    auto action = [](const sqlcons::row& row)
    {
        std::cout << row[0].as_string() << " " 
                  << row[1].as_string() << " " 
                  << row[2].as_double()  
                  << std::endl;
    };

    statment.execute(parameters, action, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }
} 
```

## Resources

- [ODBC C Data Types](https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types)
- [SQL Data Types](https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/sql-data-types)
- [Unicode Data](https://docs.microsoft.com/en-us/sql/odbc/reference/develop-app/unicode-data)

- [ODBC Error Codes](https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/appendix-a-odbc-error-codes)
- [Prepared Execution ODBC](https://docs.microsoft.com/en-us/sql/odbc/reference/develop-app/prepared-execution-odbc)
- [Prepare and Execute a Statement](https://docs.microsoft.com/en-us/sql/relational-databases/native-client-odbc-how-to/execute-queries/prepare-and-execute-a-statement-odbc)
- [Retrieving Numeric Data with SQL_NUMERIC_STRUCT](https://support.microsoft.com/en-us/help/222831/howto-retrieving-numeric-data-with-sql-numeric-struct)
- [MySQL Connector/ODBC Data Types](https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-reference-datatypes.html)

- [SQLBindParameter Function](https://docs.microsoft.com/en-us/sql/odbc/reference/syntax/sqlbindparameter-function)
- [SQLExecute Function](https://docs.microsoft.com/en-us/sql/odbc/reference/syntax/sqlexecute-function)





