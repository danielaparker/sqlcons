# sqlcons for C++

## Query example

```c++
std::error_code ec;

sql_connection connection;
connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}

connection.execute("select instrument_id, observation_date, price from instrument_price",
                   callback,
                   ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}
```

## Prepared statement example

```c++
std::error_code ec;

sql_connection connection;
connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}

sql_prepared_statement statement;
statement.prepare(connection,
    "select instrument_id, observation_date, price from instrument_price where instrument_id = ?",
    ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}

auto parameters = std::make_tuple(1);
statement.execute(parameters,callback,ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
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





