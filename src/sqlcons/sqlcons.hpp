#ifndef SQLCONS_HPP
#define SQLCONS_HPP

#include <memory>
#include <system_error>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sqlcons/unicode_traits.hpp>
#include <jsoncons/json.hpp>

namespace sqlcons {

class transaction_impl;

// transaction

class transaction
{
public:
    transaction(transaction&&) = default;
    transaction(std::unique_ptr<transaction_impl>&& pimpl);
    ~transaction();

    bool fail() const;

    void set_fail();

    void end_transaction(std::error_code& ec);
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


// parameter_base

struct parameter_base
{
    parameter_base()
        : sql_type_identifier_(0),
          c_type_identifier_(0)
    {
    }
    parameter_base(int sql_type_identifier,int c_type_identifier)
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

template <class Connector, class T>
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
    virtual ~transaction_impl() = default;

    virtual bool fail() const = 0;

    virtual void set_fail() = 0;

    virtual void end_transaction(std::error_code& ec) = 0;
};

// prepared_statement_impl

class prepared_statement_impl
{
public:
    virtual ~prepared_statement_impl() = default;

    virtual void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                          const std::function<void(const row& rec)>& callback,
                          transaction& t,
                          std::error_code& ec) = 0;

    virtual void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                          const std::function<void(const row& rec)>& callback,
                          std::error_code& ec) = 0;

    virtual void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                          std::error_code& ec) = 0;

    virtual void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                          transaction& t,
                          std::error_code& ec) = 0;
};

// connection_impl

class connection_impl
{
public:
    virtual ~connection_impl() = default;

    virtual void open(const std::string& connString, bool autoCommit, std::error_code& ec) = 0;

    virtual void auto_commit(bool val, std::error_code& ec) = 0;

    virtual void connection_timeout(size_t val, std::error_code& ec) = 0;

    virtual std::unique_ptr<transaction_impl> make_transaction() = 0;

    virtual std::unique_ptr<prepared_statement_impl> prepare_statement(const std::string& query, std::error_code& ec) = 0;

    virtual void commit(std::error_code& ec) = 0;
    virtual void rollback(std::error_code& ec) = 0;
    virtual void execute(const std::string& query, std::error_code& ec) = 0;
    virtual void execute(const std::string& query, 
                         const std::function<void(const row& rec)>& callback,
                         std::error_code& ec) = 0;
};

// parameter<T>

template <class T>
struct parameter : public parameter_base
{
    parameter(int sql_type_identifier,int c_type_identifier, const T& val)
        : parameter_base(sql_type_identifier, c_type_identifier), 
          value_(val)
    {
    }

    void* pvalue() override
    {
        return &value_;
    }

    size_t column_size() const override
    {
        return 0;
    }

    size_t buffer_capacity() const override
    {
        return 0;
    }

    size_t buffer_length() const override
    {
        return 0;
    }

    T value_;
};

// parameter<std::string>

template <>
struct parameter<std::string> : public parameter_base
{
    parameter(int sql_type_identifier,int c_type_identifier, const std::string& val);

    void* pvalue() override
    {
        //std::cout << "pvalue() val: " << value_ << std::endl;
        return &value_[0];
    }

    size_t column_size() const override
    {
        return value_.size();
    }

    size_t buffer_capacity() const override
    {
        return (value_.size()-1)*sizeof(wchar_t);
    }

    size_t buffer_length() const override
    {
        return (value_.size()-1)*sizeof(wchar_t);
    }

    std::vector<wchar_t> value_;
};

template <class Connector>
class prepared_statement
{
public:
    prepared_statement() = delete;
    prepared_statement(prepared_statement&&) = default;
    prepared_statement(std::unique_ptr<prepared_statement_impl>&& pimpl);
    ~prepared_statement();

    void execute(std::error_code& ec);

    void execute(const jsoncons::json& parameters,
                 const std::function<void(const row& rec)>& callback,
                 std::error_code& ec)
    {
        std::vector<std::unique_ptr<parameter_base>> bindings;
        if (parameters.is_array())
        {
            bindings.reserve(parameters.size());
            for (const auto& val : parameters.array_range())
            {
                switch (val.type_id())
                {
                case jsoncons::json_type_tag::bool_t:
                    bindings.push_back(std::make_unique<parameter<bool>>(sql_type_traits<Connector,bool>::sql_type_identifier(), 
                                       sql_type_traits<Connector,bool>::c_type_identifier(),
                                       val.as_bool()));
                    break;
                case jsoncons::json_type_tag::uinteger_t:
                    bindings.push_back(std::make_unique<parameter<uint64_t>>(sql_type_traits<Connector,uint64_t>::sql_type_identifier(), 
                                       sql_type_traits<Connector,uint64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::integer_t:
                    bindings.push_back(std::make_unique<parameter<int64_t>>(sql_type_traits<Connector,int64_t>::sql_type_identifier(), 
                                       sql_type_traits<Connector,int64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::double_t:
                    bindings.push_back(std::make_unique<parameter<double>>(sql_type_traits<Connector,double>::sql_type_identifier(), 
                                       sql_type_traits<Connector,double>::c_type_identifier(),
                                       val.as_double()));
                    break;
                case jsoncons::json_type_tag::small_string_t:
                case jsoncons::json_type_tag::string_t:
                    bindings.push_back(std::make_unique<parameter<std::string>>(sql_type_traits<Connector,std::string>::sql_type_identifier(), 
                                       sql_type_traits<Connector,std::string>::c_type_identifier(),
                                       val.as_string()));
                    break;
                }
            }
        }
        execute_(bindings,callback,ec);
    }

    void execute(const jsoncons::json& parameters, std::error_code& ec)
    {
        std::vector<std::unique_ptr<parameter_base>> bindings;
        if (parameters.is_array())
        {
            bindings.reserve(parameters.size());
            for (const auto& val : parameters.array_range())
            {
                switch (val.type_id())
                {
                case jsoncons::json_type_tag::bool_t:
                    bindings.push_back(std::make_unique<parameter<bool>>(sql_type_traits<Connector,bool>::sql_type_identifier(), 
                                       sql_type_traits<Connector,bool>::c_type_identifier(),
                                       val.as_bool()));
                    break;
                case jsoncons::json_type_tag::uinteger_t:
                    bindings.push_back(std::make_unique<parameter<uint64_t>>(sql_type_traits<Connector,uint64_t>::sql_type_identifier(), 
                                       sql_type_traits<Connector,uint64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::integer_t:
                    bindings.push_back(std::make_unique<parameter<int64_t>>(sql_type_traits<Connector,int64_t>::sql_type_identifier(), 
                                       sql_type_traits<Connector,int64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::double_t:
                    bindings.push_back(std::make_unique<parameter<double>>(sql_type_traits<Connector,double>::sql_type_identifier(), 
                                       sql_type_traits<Connector,double>::c_type_identifier(),
                                       val.as_double()));
                    break;
                case jsoncons::json_type_tag::small_string_t:
                case jsoncons::json_type_tag::string_t:
                    bindings.push_back(std::make_unique<parameter<std::string>>(sql_type_traits<Connector,std::string>::sql_type_identifier(), 
                                       sql_type_traits<Connector,std::string>::c_type_identifier(),
                                       val.as_string()));
                    break;
                }
            }
        }
        execute_(bindings,ec);
    }

    void execute(const jsoncons::json& parameters, transaction& t, std::error_code& ec)
    {
        std::vector<std::unique_ptr<parameter_base>> bindings;
        if (parameters.is_array())
        {
            bindings.reserve(parameters.size());
            for (const auto& val : parameters.array_range())
            {
                switch (val.type_id())
                {
                case jsoncons::json_type_tag::bool_t:
                    bindings.push_back(std::make_unique<parameter<bool>>(sql_type_traits<Connector,bool>::sql_type_identifier(), 
                                       sql_type_traits<Connector,bool>::c_type_identifier(),
                                       val.as_bool()));
                    break;
                case jsoncons::json_type_tag::uinteger_t:
                    bindings.push_back(std::make_unique<parameter<uint64_t>>(sql_type_traits<Connector,uint64_t>::sql_type_identifier(), 
                                       sql_type_traits<Connector,uint64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::integer_t:
                    bindings.push_back(std::make_unique<parameter<int64_t>>(sql_type_traits<Connector,int64_t>::sql_type_identifier(), 
                                       sql_type_traits<Connector,int64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::double_t:
                    bindings.push_back(std::make_unique<parameter<double>>(sql_type_traits<Connector,double>::sql_type_identifier(), 
                                       sql_type_traits<Connector,double>::c_type_identifier(),
                                       val.as_double()));
                    break;
                case jsoncons::json_type_tag::small_string_t:
                case jsoncons::json_type_tag::string_t:
                    bindings.push_back(std::make_unique<parameter<std::string>>(sql_type_traits<Connector,std::string>::sql_type_identifier(), 
                                       sql_type_traits<Connector,std::string>::c_type_identifier(),
                                       val.as_string()));
                    break;
                }
            }
        }
        execute_(bindings, t, ec);
    }
private:
    void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings,
        const std::function<void(const row& rec)>& callback,
        std::error_code& ec);
    void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings,
        const std::function<void(const row& rec)>& callback,
        transaction& t,
        std::error_code& ec);
    void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings,
        std::error_code& ec);
    void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                  transaction& t,
                  std::error_code& ec);

    std::unique_ptr<prepared_statement_impl> pimpl_;
};

// prepared_statement

template <class Connector>
prepared_statement<Connector>::prepared_statement(std::unique_ptr<prepared_statement_impl>&& impl) : pimpl_(std::move(impl)) {}

template <class Connector>
prepared_statement<Connector>::~prepared_statement() = default;

template <class Connector>
void prepared_statement<Connector>::execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                                             const std::function<void(const row& rec)>& callback,
                                             std::error_code& ec)
{
    pimpl_->execute_(bindings, callback, ec);
}

template <class Connector>
void prepared_statement<Connector>::execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                                             const std::function<void(const row& rec)>& callback,
                                             transaction& t, 
                                             std::error_code& ec)
{
    pimpl_->execute_(bindings, callback, t, ec);
}

template <class Connector>
void prepared_statement<Connector>::execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                                             std::error_code& ec)
{
    pimpl_->execute_(bindings, ec);
}

template <class Connector>
void prepared_statement<Connector>::execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                                             transaction& t, 
                                             std::error_code& ec)
{
    pimpl_->execute_(bindings, t, ec);
}

// connection

template <class Connector>
class connection
{
    std::unique_ptr<connection_impl> pimpl_;
public:
    connection() : pimpl_(Connector::create_connection()) 
    {
    }
    ~connection()
    {
    }

    void open(const std::string& connString, bool autoCommit, std::error_code& ec);

    void auto_commit(bool val, std::error_code& ec);

    void connection_timeout(size_t val, std::error_code& ec);

    friend transaction make_transaction(connection<Connector>& conn)
    {
        return transaction(conn.pimpl_->make_transaction());
    }

    friend prepared_statement<Connector> make_prepared_statement(connection<Connector>& conn, const std::string& query, std::error_code& ec)
    {
        return prepared_statement<Connector>(conn.pimpl_->prepare_statement(query, ec));
    }

    void commit(std::error_code& ec);

    void rollback(std::error_code& ec);

    void execute(const std::string& query, 
                 std::error_code& ec);

    void execute(const std::string& query, 
                 const std::function<void(const row& rec)>& callback,
                 std::error_code& ec);
};

// connection<Connector>

template <class Connector>
void connection<Connector>::open(const std::string& connString, bool autoCommit, std::error_code& ec)
{
    pimpl_->open(connString, autoCommit, ec);
}

template <class Connector>
void connection<Connector>::auto_commit(bool val, std::error_code& ec)
{
    pimpl_->auto_commit(val, ec);
}

template <class Connector>
void connection<Connector>::connection_timeout(size_t val, std::error_code& ec)
{
    pimpl_->connection_timeout(val, ec);
}

template <class Connector>
void connection<Connector>::execute(const std::string& query, 
                                    std::error_code& ec)
{
    pimpl_->execute(query, ec);
}

template <class Connector>
void connection<Connector>::execute(const std::string& query, 
                         const std::function<void(const row& rec)>& callback,
                         std::error_code& ec)
{
    pimpl_->execute(query, callback, ec);
}

}

#endif
