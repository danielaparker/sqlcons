#include <sqlcons_connector/odbc/connector.hpp>
#include <sqlcons/sqlcons.hpp>

void query1()
{
    std::error_code ec;

    const std::string databaseUrl = "Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;";

    sqlcons::connection_pool<sqlcons::odbc::odbc_connector> connection_pool(databaseUrl,2);

    auto connection = connection_pool.get_connection(ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    std::string query = "SELECT I.ticker, P.observation_date, P.close_price FROM equity_price P JOIN equity I ON P.instrument_id = I.instrument_id WHERE I.ticker='IBM'";

    auto action = [](const sqlcons::row& row)
    {
        std::wcout << row[0].as_wstring() << " " 
                  << row[1].as_wstring() << " " 
                  << row[2].as_double()  
                  << std::endl;
    };

    connection.execute(query, action, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }
} 

void query2()
{
    std::error_code ec;

    const std::string databaseUrl = "Driver={SQL Server};Server=localhost;Database=RiskSnap;Trusted_Connection=Yes;";

    sqlcons::connection_pool<sqlcons::odbc::odbc_connector> connection_pool(databaseUrl,2);

    auto connection = connection_pool.get_connection(ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    std::string sql = "SELECT I.ticker, P.observation_date, P.close_price FROM equity_price P JOIN equity I ON P.instrument_id = I.instrument_id WHERE I.ticker=?";
    auto statment = make_prepared_statement(connection, sql, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
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

    statment.execute(parameters, action, ec);
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }
} 

int main()
{
    query1();
    query2();
} 

