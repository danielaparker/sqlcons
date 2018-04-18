# sqlcons for C++

## Examples

```c++
#include <sqlcons_bindings/odbc/odbc_bindings.hpp>
#include <sqlcons/sqlcons.hpp>

void quotes(const std::string& databaseUrl, std::error_code& ec)
{
    // Connection pool that keeps up to two connections in the pool
    sqlcons::connection_pool<sqlcons::odbc::odbc_bindings> pool(databaseUrl,2);

    {
        auto connection = pool.get_connection(ec);
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
                data_fields NVARCHAR(MAX) NOT NULL
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
                DECLARE @data_fields NVARCHAR(MAX) = ?;

                INSERT INTO stock(symbol,data_fields)
                VALUES(@symbol,@data_fields)
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

            std::cout << "\n(1)\n";
            connection.execute("SELECT * FROM stock", f, ec);
        }

        // Connection automatically returned to free connection pool
        // when it goes out of scope if less than max in pool,
        // otherwise closed
    }

    // Transactions
    {
        // Transaction will be rolled back since we don't call connection.commit()
        {
            auto connection = pool.get_connection<sqlcons::transaction_policy::man_commit>(ec);
            if (ec)
            {
                return;
            }
            connection.execute("DELETE FROM stock", ec);
            if (ec)
            {
                return;
            }
            // connection.commit(ec);
        }

        // Let's check
        {
            auto connection = pool.get_connection(ec);
            if (ec)
            {
                return;
            }
            auto f = [](const sqlcons::row& row)
            {            
                std::cout << "\n(2) " << row[0].as_long() << std::endl;
            };
            connection.execute("SELECT count(*) FROM stock", f, ec);
        }

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
(1)
1,GOOG,2018-04-18 16:55:27,{"name":"Alphabet Inc."}
2,IBM,2018-04-18 16:55:27,{"name":"Alphabet Inc."}

(2) 2
```

## Resources

- [ODBC C Data Types](https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types)
- [SQL Data Types](https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/sql-data-types)
- [Unicode Data](https://docs.microsoft.com/en-us/sql/odbc/reference/develop-app/unicode-data)

- [ODBC Error Codes](https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/appendix-a-odbc-error-codes)
- [Prepared Execution ODBC](https://docs.microsoft.com/en-us/sql/odbc/reference/develop-app/prepared-execution-odbc)
- [Prepare and Execute a Statement](https://docs.microsoft.com/en-us/sql/relational-databases/native-client-odbc-how-to/execute-queries/prepare-and-execute-a-statement-odbc)
- [Retrieving Numeric Data with SQL_NUMERIC_STRUCT](https://support.microsoft.com/en-us/help/222831/howto-retrieving-numeric-data-with-sql-numeric-struct)
- [MySQL Bindings/ODBC Data Types](https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-reference-datatypes.html)

- [SQLBindParameter Function](https://docs.microsoft.com/en-us/sql/odbc/reference/syntax/sqlbindparameter-function)
- [SQLExecute Function](https://docs.microsoft.com/en-us/sql/odbc/reference/syntax/sqlexecute-function)





