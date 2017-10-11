# sqlcons for C++

## Query example

```c++
std::error_code ec;

sqlcons::connection connection;
connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}

auto action = [](const sqlcons::row& row)
{
    const sqlcons::data_value& value = row[0];
    std::cout << row[0].as_long() << " " 
              << row[1].as_string() << " " 
              << row[2].as_double()  
              << std::endl;
};

connection.execute("select instrument_id, observation_date, price from instrument_price",
                   action,
                   ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}
```

## Prepared statement example 1

```c++
std::error_code ec;

sqlcons::connection connection;
connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}

sqlcons::prepared_statement statement = connection.prepare_statement(
    "select instrument_id, observation_date, price from instrument_price where instrument_id = ?",
    ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}

auto action = [](const sqlcons::row& row)
{
    const sqlcons::data_value& value = row[0];
    std::cout << row[0].as_long() << " " 
              << row[1].as_string() << " " 
              << row[2].as_double()  
              << std::endl;
};

auto parameters = std::make_tuple(1);
statement.execute(parameters, action, ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}
```

## Prepared statement example 2

```c++
std::error_code ec;

sqlcons::connection connection;
connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}

sqlcons::prepared_statement statement = connection.prepare_statement(
    "select instrument_id, contract_date from futures_contract where product_id = ?",
    ec);
if (ec)
{
    std::cerr << ec.message() << std::endl;
    return;
}

auto action = [](const sqlcons::row& row)
{
    std::cout << row[0].as_long() << " " 
              << row[1].as_string() << " " 
              << std::endl;
};

auto parameters = std::make_tuple(std::string("HO"));
statement.execute(parameters, action, ec);
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





