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

void callback(const sql_record& record)
{
    const sql_column& column = record[0];
    std::cout << record[0].as_long() << " " 
              << record[1].as_string() << " " 
              << record[2].as_double()  
              << std::endl;
}

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

    connection.execute("select instrument_id, observation_date, price from instrument_price",
                       callback,
                       ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }
} 

