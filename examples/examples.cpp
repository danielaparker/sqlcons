#include <sqlcons_connector/odbc/connector.hpp>
#include <sqlcons/sqlcons.hpp>

using namespace sqlcons;

int query1()
{
    std::error_code ec;

    sqlcons::connection<sqlcons::odbc::odbc_connector> connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return 1;
    }

    std::string query = "SELECT I.ticker, P.observation_date, P.close_price FROM equity_price P JOIN equity I ON P.instrument_id = I.instrument_id WHERE I.ticker='IBM'";

    auto action = [](const sqlcons::row& row)
    {
        std::cout << row[0].as_string() << " " 
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
    return 0;
} 

int query2()
{
    std::error_code ec;

    sqlcons::connection<sqlcons::odbc::odbc_connector> connection;
    connection.open("Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;", true, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return 1;
    }

    std::string statement = "SELECT I.ticker, P.observation_date, P.close_price FROM equity_price P JOIN equity I ON P.instrument_id = I.instrument_id WHERE I.ticker=?";
    auto query = connection.prepare_statement(statement,ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return 1;
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

    query.execute(parameters, action, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return 1;
    }
    return 0;
} 

int main()
{
    int rc = 0;
    rc = query2();
    return rc;
} 

