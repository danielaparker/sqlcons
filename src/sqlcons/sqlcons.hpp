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

// parameter<T>

template <class T>
struct parameter : public base_parameter
{
    parameter(int sql_type_identifier,int c_type_identifier, const T& val)
        : base_parameter(sql_type_identifier, c_type_identifier), 
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
struct parameter<std::string> : public base_parameter
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

namespace detail
{

template<class Connector, size_t Pos, class base_parameter, class Tuple>
struct sql_parameters_tuple_helper
{
    using element_type = typename std::tuple_element<Pos - 1, Tuple>::type;
    using next = sql_parameters_tuple_helper<Connector, Pos - 1, base_parameter, Tuple>;

    static void to_parameters(const Tuple& tuple, std::vector<std::unique_ptr<base_parameter>>& bindings)
    {
        bindings[Pos - 1] = std::make_unique<parameter<element_type>>(sql_type_traits<Connector,element_type>::sql_type_identifier(), 
                                                                      sql_type_traits<Connector,element_type>::c_type_identifier(),
                                                                      std::get<Pos-1>(tuple));
        next::to_parameters(tuple, bindings);
    }
};

template<class Connector, class base_parameter, class Tuple>
struct sql_parameters_tuple_helper<Connector, 0, base_parameter, Tuple>
{
    static void to_parameters(const Tuple& tuple, std::vector<std::unique_ptr<base_parameter>>& json)
    {
    }
};

}

template <class Connector>
class prepared_statement
{
public:
    prepared_statement() = delete;
    prepared_statement(prepared_statement&&) = default;
    prepared_statement(std::unique_ptr<prepared_statement_impl>&& pimpl);
    ~prepared_statement();

    void execute(std::error_code& ec);

    template <typename Tuple>
    void execute(const Tuple& parameters,
                 const std::function<void(const row& rec)>& callback,
                 std::error_code& ec)
    {
        using helper = detail::sql_parameters_tuple_helper<Connector, std::tuple_size<Tuple>::value, base_parameter, Tuple>;
        
        const size_t num_elements = std::tuple_size<Tuple>::value;
        std::vector<std::unique_ptr<base_parameter>> params(std::tuple_size<Tuple>::value);
        helper::to_parameters(parameters, params);
        execute_(params,callback,ec);
    }

    template <typename Tuple>
    void execute(const Tuple& parameters,
                 std::error_code& ec)
    {
        using helper = detail::sql_parameters_tuple_helper<Connector, std::tuple_size<Tuple>::value, base_parameter, Tuple>;

        const size_t num_elements = std::tuple_size<Tuple>::value;
        std::vector<std::unique_ptr<base_parameter>> params(std::tuple_size<Tuple>::value);
        helper::to_parameters(parameters, params);
        execute_(params,ec);
    }

    template <typename Tuple>
    void execute(const Tuple& parameters,
                 transaction& t)
    {
        using helper = detail::sql_parameters_tuple_helper<Connector, std::tuple_size<Tuple>::value, base_parameter, Tuple>;

        const size_t num_elements = std::tuple_size<Tuple>::value;
        std::vector<std::unique_ptr<base_parameter>> params(std::tuple_size<Tuple>::value);
        helper::to_parameters(parameters, params);
        execute_(params,t);
    }
private:
    void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings,
        const std::function<void(const row& rec)>& callback,
        std::error_code& ec);
    void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings,
        std::error_code& ec);
    void execute_(std::vector<std::unique_ptr<base_parameter>>& bindings,
                    transaction& t);

    std::unique_ptr<prepared_statement_impl> pimpl_;
};

// prepared_statement

template <class Connector>
prepared_statement<Connector>::prepared_statement(std::unique_ptr<prepared_statement_impl>&& impl) : pimpl_(std::move(impl)) {}

template <class Connector>
prepared_statement<Connector>::~prepared_statement() = default;

template <class Connector>
void prepared_statement<Connector>::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                        const std::function<void(const row& rec)>& callback,
                                        std::error_code& ec)
{
    pimpl_->execute_(bindings, callback, ec);
}

template <class Connector>
void prepared_statement<Connector>::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                        std::error_code& ec)
{
    pimpl_->execute_(bindings, ec);
}

template <class Connector>
void prepared_statement<Connector>::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                  transaction& t)
{
    pimpl_->execute_(bindings, t);
}

// connection

template <class Connector>
class connection
{
public:
    connection();
    ~connection();

    void open(const std::string& connString, bool autoCommit, std::error_code& ec);

    void auto_commit(bool val, std::error_code& ec);

    void connection_timeout(size_t val, std::error_code& ec);

    transaction create_transaction();

    prepared_statement<Connector> prepare_statement(const std::string& query, transaction& trans);

    prepared_statement<Connector> prepare_statement(const std::string& query, std::error_code& ec);

    void commit(std::error_code& ec);

    void rollback(std::error_code& ec);

    void execute(const std::string& query, 
                 std::error_code& ec);

    void execute(const std::string& query, 
                 const std::function<void(const row& rec)>& callback,
                 std::error_code& ec);
private:
    std::unique_ptr<connection_impl> pimpl_;
};

// connection<Connector>

template <class Connector>
connection<Connector>::connection() : pimpl_(Connector::create_connection()) {}

template <class Connector>
connection<Connector>::~connection() = default;

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
transaction connection<Connector>::create_transaction()
{
    return transaction(pimpl_->create_transaction());
}

template <class Connector>
prepared_statement<Connector> connection<Connector>::prepare_statement(const std::string& query, std::error_code& ec)
{
    return prepared_statement<Connector>(pimpl_->prepare_statement(query, ec));
}

template <class Connector>
prepared_statement<Connector> connection<Connector>::prepare_statement(const std::string& query, transaction& trans)
{
    return prepared_statement<Connector>(pimpl_->prepare_statement(query, trans));
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
