# sqlcons for C++

## Examples

```c++
#include <sqlcons_bindings/odbc/odbc_bindings.hpp>
#include <sqlcons/sqlcons.hpp>

void quotes(const std::string& databaseUrl, std::error_code& ec)
{
    sqlcons::connection_pool<sqlcons::odbc::odbc_bindings> connection_pool(databaseUrl,2);

    auto connection = connection_pool.get_connection(ec);
    if (ec)
    {
        return;
    }

    // Create stock table
    connection.execute("DROP TABLE IF EXISTS stock", ec);
    if (ec)
    {
        return;
    }

    std::string createStockTable = R"(
        CREATE TABLE stock
        (
            stock_skey BIGINT IDENTITY(1,1) NOT NULL,
            symbol NVARCHAR(30) NOT NULL PRIMARY KEY,
            last_updated DATETIME2(0) DEFAULT GETDATE(),
            properties NVARCHAR(MAX) NOT NULL
        )
    )";

    connection.execute(createStockTable, ec);
    if (ec)
    {
        return;
    }

    // Add some stocks
    {
        std::string sql = R"(
            DECLARE @symbol NVARCHAR(30) = ?;
            DECLARE @properties NVARCHAR(MAX) = ?;

            INSERT INTO stock(symbol,properties)
            VALUES(@symbol,@properties)
        )";

        auto statement = make_prepared_statement(connection, sql, ec);
        if (ec)
        {
            return;
        }

        // Add GOOG
        jsoncons::json properties1;
        properties1["name"] = "Alphabet Inc.";

        jsoncons::json parameters1 = jsoncons::json::array();
        parameters1.push_back("GOOG");
        parameters1.push_back(properties1.to_string());

        statement.execute(parameters1,ec);
        if (ec)
        {
            return;
        }

        // Add GOOG
        jsoncons::json properties2;
        properties2["name"] = "IBM";

        jsoncons::json parameters2 = jsoncons::json::array();
        parameters2.push_back("IBM");
        parameters2.push_back(properties1.to_string());

        statement.execute(parameters2,ec);
        if (ec)
        {
            return;
        }
    }

    // Select all
    {
        auto f = [](const sqlcons::row& row)
        {            
            for (size_t i = 0; i < row.size(); ++i)
            {
                if (i > 0)
                {
                    std::cout << ",";
                }
                std::cout << row[i].as_string();
            }
            std::cout << std::endl;
        };

        connection.execute("SELECT * FROM stock", f, ec);
    }
}

int main()
{
    std::error_code ec;

    const std::string& databaseUrl = "Driver={SQL Server};Server=localhost;Database=quotes;Trusted_Connection=Yes;";
    quotes(databaseUrl, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
    }
} 
```
Output:
```
1,GOOG,2018-04-17 16:36:15,{"name":"Alphabet Inc."}
2,IBM,2018-04-17 16:36:15,{"name":"Alphabet Inc."}
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





