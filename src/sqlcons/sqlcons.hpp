#ifndef SQLCONS_HPP
#define SQLCONS_HPP

#include <memory>
#include <string>
#include <system_error>
#include <functional>
#include <vector>
#include <map>

namespace sqlcons {

// conv_errc

enum class sql_errc 
{
    db_err = 1
}; 

class sqlcons_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const noexcept
    {
        return "sqlcons error";
    }
    virtual std::string message(int ev) const
    {
        switch (static_cast<sql_errc>(ev))
        {
        case sql_errc::db_err:
            return "db error";
        default:
            return "";
            break;
        }
    }
};

const std::error_category& sqlcons_error_category();

std::error_code make_error_code(sql_errc result);


template <class Tuple>
using query_callback = std::function<void(const Tuple&)>;

// sql_column

class sql_column
{
public:
    virtual ~sql_column() = default;

    virtual std::string as_string() const = 0;

    virtual std::wstring as_wstring() const = 0;

    virtual double as_double() const = 0;

    virtual long as_long() const = 0;
};

// sql_record

class sql_record
{
public:
    sql_record(std::vector<sql_column*>&& columns);

    ~sql_record();

    size_t size() const;

    const sql_column& operator[](size_t index) const;
private:
    std::vector<sql_column*> columns_;
    std::map<std::string,sql_column*> column_map_;
};

// sql_connection

class sql_connection
{
public:
    friend class sql_statement;
    friend class sql_prepared_statement;

    sql_connection();
    ~sql_connection();

    void open(const std::string& connString, std::error_code& ec);
    void execute(const std::string query, 
                 std::error_code& ec);
    void execute(const std::string query, 
                 const std::function<void(const sql_record& record)>& callback,
                 std::error_code& ec);
private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

// sql_prepared_statement

class sql_prepared_statement
{
public:
    sql_prepared_statement();
    ~sql_prepared_statement();

    void prepare(sql_connection& conn, const std::string& query, std::error_code& ec);

    void execute(std::error_code& ec);
    void execute(const std::function<void(const sql_record& record)>& callback,
                 std::error_code& ec);
private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}

#endif
