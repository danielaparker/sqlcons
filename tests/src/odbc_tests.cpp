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

TEST_CASE("odbc_tests") 
{
    std::error_code ec;

    sqlcons::sql_connection connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    connection.execute("select * from instrument_price",
                       [](const sqlcons::sql_record& record){},
                       ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }
} 

