#include <sqlcons/sqlcons.hpp>
#include <windows.h> 
#include <sql.h> 
#include <sqlext.h> 
#include <stdio.h> 
#include <conio.h> 
#include <tchar.h> 
#include <stdlib.h> 
#include <sal.h> 

#include <catch.hpp>

#include <iostream>
#include <sqlcons/unicode_traits.hpp>

using namespace sqlcons;

TEST_CASE("odbc_tests") 
{
    std::error_code ec;

    sql_connection connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    auto action = [](const sql_record& record)
    {
        const sql_column& column = record[0];
        std::cout << record[0].as_long() << " " 
                  << record[1].as_string() << " " 
                  << record[2].as_double()  
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
} 

TEST_CASE("sql_prepared_statement") 
{
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

    auto action = [](const sql_record& record)
    {
        const sql_column& column = record[0];
        std::cout << record[0].as_long() << " " 
                  << record[1].as_string() << " " 
                  << record[2].as_double()  
                  << std::endl;
    };

    auto parameters = std::make_tuple(1);
    statement.execute(parameters, action, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

} 

TEST_CASE("sql_prepared_statement_with_string_param") 
{
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
        "select instrument_id, contract_date from futures_contract where product_id = ?",
        ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    auto action = [](const sql_record& record)
    {
        std::cout << record[0].as_long() << " " 
                  << record[1].as_string() << " " 
                  << std::endl;
    };

    auto parameters = std::make_tuple(std::string("HO"));
    statement.execute(parameters, action, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

} 


