#ifndef SQLCONS_CONNECTOR_CONNECTORFWD_HPP
#define SQLCONS_CONNECTOR_CONNECTORFWD_HPP

#include <memory>
#include <system_error>
#include <functional>
#include <vector>
#include <map>
#include <string>

namespace sqlcons { 

class transaction_impl;

// transaction

class transaction
{
public:
    transaction(transaction&&) = default;
    transaction(std::unique_ptr<transaction_impl>&& pimpl);
    ~transaction();

    std::error_code error_code() const;

    void update_error_code(std::error_code ec);

    void end(std::error_code& ec);
private:
    std::unique_ptr<transaction_impl> pimpl_;
};

// value

class value
{
public:
    virtual ~value() = default;

    virtual std::string as_string() const = 0;

    virtual std::wstring as_wstring() const = 0;

    virtual double as_double() const = 0;

    virtual long as_long() const = 0;
};

// row

class row
{
public:
    row(std::vector<value*>&& values);

    ~row();

    size_t size() const;

    const value& operator[](size_t index) const;
private:
    std::vector<value*> values_;
    std::map<std::string,value*> column_map_;
};


// base_parameter

struct base_parameter
{
    base_parameter()
        : sql_type_identifier_(0),
          c_type_identifier_(0)
    {
    }
    base_parameter(int sql_type_identifier,int c_type_identifier)
        : sql_type_identifier_(sql_type_identifier), 
          c_type_identifier_(c_type_identifier)
    {
    }
    virtual void* pvalue() = 0;

    virtual size_t column_size() const = 0;

    virtual size_t buffer_capacity() const = 0;

    virtual size_t buffer_length() const = 0;

    int parameter_type() const
    {
        return sql_type_identifier_;
    }

    int value_type() const
    {
        return c_type_identifier_;
    }

    int64_t* pind() 
    {
        return &ind_;
    }

    int64_t ind_;
    int sql_type_identifier_;
    int c_type_identifier_;
};

template <class T>
struct sql_type_traits
{
    typedef T value_type;
    static int sql_type_identifier();
    static int c_type_identifier();
};

// transaction_impl

class transaction_impl
{
public:
    virtual std::error_code error_code() const = 0;

    virtual void update_error_code(std::error_code ec) = 0;

    virtual void end(std::error_code& ec) = 0;
};

// prepared_statement_impl

class prepared_statement_impl
{
public:
    virtual void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                          const std::function<void(const row& rec)>& callback,
                          std::error_code& ec) = 0;

    virtual void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                          std::error_code& ec) = 0;

    virtual void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                          transaction& t) = 0;
};

// connection_impl

class connection_impl
{
public:
    virtual void open(const std::string& connString, bool autoCommit, std::error_code& ec) = 0;

    virtual void auto_commit(bool val, std::error_code& ec) = 0;

    virtual void connection_timeout(size_t val, std::error_code& ec) = 0;

    virtual std::unique_ptr<transaction_impl> create_transaction() = 0;

    virtual std::unique_ptr<prepared_statement_impl> prepare_statement(const std::string& query, std::error_code& ec) = 0;

    virtual std::unique_ptr<prepared_statement_impl> prepare_statement(const std::string& query, transaction& trans) = 0;

    virtual void commit(std::error_code& ec) = 0;
    virtual void rollback(std::error_code& ec) = 0;
    virtual void execute(const std::string& query, 
                         std::error_code& ec) = 0;
    virtual void execute(const std::string& query, 
                         const std::function<void(const row& rec)>& callback,
                         std::error_code& ec) = 0;
};


namespace odbc {
class connector
{
public:
    static std::unique_ptr<connection_impl> create_connection();
};

}

}

#endif
