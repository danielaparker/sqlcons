#include <sqlcons_connector/odbc/connector.hpp>
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

    sqlcons::connection<sqlcons::odbc::odbc_connector> connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    auto action = [](const sqlcons::row& row)
    {
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
} 

TEST_CASE("sqlcons::prepared_statement") 
{
    std::error_code ec;

    sqlcons::connection<sqlcons::odbc::odbc_connector> connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    auto statement = connection.prepare_statement(
        "select instrument_id, observation_date, price from instrument_price where instrument_id = ?",
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

} 

TEST_CASE("sqlcons::prepared_statement_with_string_param") 
{
    std::error_code ec;

    sqlcons::connection<sqlcons::odbc::odbc_connector> connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    auto statement = connection.prepare_statement(
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

} 

TEST_CASE("Prepared insert statement") 
{
    std::error_code ec;

    sqlcons::connection<sqlcons::odbc::odbc_connector> connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    auto statement = connection.prepare_statement(
        "INSERT INTO futures_contract(product_id,contract_date) VALUES(?,?)",
        ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    auto parameters = std::make_tuple<std::string,std::string>("HO","2017-03-31");
    statement.execute(parameters,ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }
} 

TEST_CASE("Transaction") 
{
    std::error_code ec;

    sqlcons::connection<sqlcons::odbc::odbc_connector> connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    sqlcons::transaction trans = connection.create_transaction();
}


