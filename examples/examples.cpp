#include <sqlcons_connector/odbc/connector.hpp>
#include <sqlcons/sqlcons.hpp>

using namespace sqlcons;

int main()
{
    std::error_code ec;

    sqlcons::connection<sqlcons::odbc::odbc_connector> connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=example;Trusted_Connection=Yes;", true, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return 1;
    }

    std::string query = "SELECT I.ticker, P.observation_date, P.price FROM equity_price P JOIN equity I ON P.instrument_id = I.instrument_id";

    auto action = [](const sqlcons::row& row)
    {
        std::cout << row[0].as_long() << " " 
                  << row[1].as_string() << " " 
                  << row[2].as_double()  
                  << std::endl;
    };

    connection.execute(query, action, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return 1;
    }
} 

