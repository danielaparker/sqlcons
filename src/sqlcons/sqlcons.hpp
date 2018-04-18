#ifndef SQLCONS_HPP
#define SQLCONS_HPP

#include <mutex>
#include <stack>
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

// value

class value
{
public:
    virtual ~value() = default;

    virtual std::string as_string() const = 0;

    virtual std::wstring as_wstring() const = 0;

    virtual double as_double() const = 0;

    virtual int64_t as_integer() const = 0;
};

// row

class row
{
    std::vector<value*> values_;
    std::map<std::string,value*> column_map_;
public:
    typedef std::vector<value*>::iterator iterator;

    row(std::vector<value*>&& values)
        : values_(std::move(values))
    {
    }

    ~row() = default;

    size_t size() const
    {
        return values_.size();
    }

    const value& operator[](size_t index) const
    {
        return *values_[index];
    }

    value& operator[](size_t index)
    {
        return *values_[index];
    }

    iterator begin() 
    {
        return values_.begin();
    }

    iterator end() 
    {
        return values_.end();
    }
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

template <class Bindings, class T>
struct sql_type_traits
{
    typedef T value_type;
    static int sql_type_identifier();
    static int c_type_identifier();
};

// prepared_statement_impl

class prepared_statement_impl
{
public:
    virtual ~prepared_statement_impl() = default;

    virtual void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                          const std::function<void(const row& rec)>& callback,
                          std::error_code& ec) = 0;

    virtual void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings, 
                          std::error_code& ec) = 0;
};

// connection_impl

class connection_impl
{
public:
    virtual ~connection_impl() = default;

    virtual void open(const std::string& connString, std::error_code& ec) = 0;

    virtual void auto_commit(bool val, std::error_code& ec) = 0;

    virtual void connection_timeout(size_t val, std::error_code& ec) = 0;

    virtual std::unique_ptr<prepared_statement_impl> prepare_statement(const std::string& query, std::error_code& ec) = 0;

    virtual void commit(std::error_code& ec) = 0;
    virtual void rollback(std::error_code& ec) = 0;
    virtual void execute(const std::string& query, std::error_code& ec) = 0;
    virtual void execute(const std::string& query, 
                         const std::function<void(const row& rec)>& callback,
                         std::error_code& ec) = 0;

    virtual bool is_valid() const = 0;
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
    parameter(int sql_type_identifier,int c_type_identifier, const std::string& val)
       : parameter_base(sql_type_identifier, c_type_identifier)
   {
       auto result1 = unicons::convert(val.begin(),val.end(),
                                       std::back_inserter(value_), 
                                       unicons::conv_flags::strict);
       value_.push_back(0);
   }

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

namespace transaction_policy {

class transaction
{
public:
    virtual bool fail() const = 0;

    virtual void rollback() = 0;

    virtual void commit(connection_impl& impl, std::error_code& ec) = 0;
};

class auto_commit : public virtual transaction
{
public:
    auto_commit() = default;
    auto_commit(const auto_commit&) = default;
    auto_commit(auto_commit&& other) = default;

    bool is_auto_commit() const
    {
        return true;
    }

    bool fail() const override
    {
        return false;
    }

    void rollback() override
    {
    }

    void commit(connection_impl& impl, std::error_code& ec) override
    {
    }

    void init(connection_impl* pimpl, std::error_code& ec) 
    {
        pimpl->auto_commit(true,ec);
    }

    void end_transaction(connection_impl* pimpl, std::error_code& ec) 
    {
    }
};

class man_commit : public virtual transaction
{
    bool rollback_;
    bool committed_;
public:
    man_commit()
        : rollback_(false), committed_(false)
    {
    }
    man_commit(const man_commit&) = default;
    man_commit(man_commit&& other) = default;
    ~man_commit() = default;

    bool is_auto_commit() const
    {
        return false;
    }

    bool fail() const override
    {
        return rollback_;
    }

    void rollback() override
    {
        rollback_ = true;
    }

    void commit(connection_impl& impl, std::error_code& ec) override
    {
        if (!rollback_)
        {
            impl.commit(ec);
            if (!ec)
            {
                committed_ = true;
            }
        }
        else
        {
            impl.rollback(ec);
            if (!ec)
            {
                rollback_ = false;
            }
        }
    }

    void init(connection_impl* pimpl, std::error_code& ec) 
    {
        pimpl->auto_commit(false,ec);
    }

    void end_transaction(connection_impl* pimpl, std::error_code& ec) 
    {
        if (rollback_ || !committed_)
        {
            pimpl->rollback(ec);
        }
    }
};

}

template <class Bindings>
class prepared_statement
{
    std::unique_ptr<prepared_statement_impl> pimpl_;
    transaction_policy::transaction* tp_;
public:
    prepared_statement() = delete;
    prepared_statement(prepared_statement&&) = default;
    prepared_statement(std::unique_ptr<prepared_statement_impl>&& pimpl, transaction_policy::transaction* tp)
         : pimpl_(std::move(pimpl)), tp_(tp) 
    {
    }
    ~prepared_statement() = default;

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
                    bindings.push_back(std::make_unique<parameter<bool>>(sql_type_traits<Bindings,bool>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,bool>::c_type_identifier(),
                                       val.as_bool()));
                    break;
                case jsoncons::json_type_tag::uinteger_t:
                    bindings.push_back(std::make_unique<parameter<uint64_t>>(sql_type_traits<Bindings,uint64_t>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,uint64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::integer_t:
                    bindings.push_back(std::make_unique<parameter<int64_t>>(sql_type_traits<Bindings,int64_t>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,int64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::double_t:
                    bindings.push_back(std::make_unique<parameter<double>>(sql_type_traits<Bindings,double>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,double>::c_type_identifier(),
                                       val.as_double()));
                    break;
                case jsoncons::json_type_tag::small_string_t:
                case jsoncons::json_type_tag::string_t:
                    bindings.push_back(std::make_unique<parameter<std::string>>(sql_type_traits<Bindings,std::string>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,std::string>::c_type_identifier(),
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
                    bindings.push_back(std::make_unique<parameter<bool>>(sql_type_traits<Bindings,bool>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,bool>::c_type_identifier(),
                                       val.as_bool()));
                    break;
                case jsoncons::json_type_tag::uinteger_t:
                    bindings.push_back(std::make_unique<parameter<uint64_t>>(sql_type_traits<Bindings,uint64_t>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,uint64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::integer_t:
                    bindings.push_back(std::make_unique<parameter<int64_t>>(sql_type_traits<Bindings,int64_t>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,int64_t>::c_type_identifier(),
                                       val.as_integer()));
                    break;
                case jsoncons::json_type_tag::double_t:
                    bindings.push_back(std::make_unique<parameter<double>>(sql_type_traits<Bindings,double>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,double>::c_type_identifier(),
                                       val.as_double()));
                    break;
                case jsoncons::json_type_tag::small_string_t:
                case jsoncons::json_type_tag::string_t:
                    bindings.push_back(std::make_unique<parameter<std::string>>(sql_type_traits<Bindings,std::string>::sql_type_identifier(), 
                                       sql_type_traits<Bindings,std::string>::c_type_identifier(),
                                       val.as_string()));
                    break;
                }
            }
        }
        execute_(bindings,ec);
    }
private:
    void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings,
        const std::function<void(const row& rec)>& callback,
        std::error_code& ec)
    {
        if (!tp_->fail())
        {
            pimpl_->execute_(bindings, callback, ec);
            if (ec)
            {
                tp_->rollback();
            }
        }
    }
    void execute_(std::vector<std::unique_ptr<parameter_base>>& bindings,
                  std::error_code& ec)
    {
        if (!tp_->fail())
        {
            pimpl_->execute_(bindings, ec);
            if (ec)
            {
                tp_->rollback();
            }
        }
    }
};

// connection

template <class Bindings>
class connection_pool;

template <class Bindings,class TP>
class connection
{
    std::unique_ptr<connection_impl> pimpl_;
    TP transaction_policy_;
    connection_pool<Bindings>* pool_;
public:
    connection(std::unique_ptr<connection_impl> ptr, TP&& tp, connection_pool<Bindings>* pool) 
        : pimpl_(std::move(ptr)), transaction_policy_(std::move(tp)), pool_(pool) 
    {
    }
    ~connection()
    {
        std::error_code ec;
        transaction_policy_.end_transaction(pimpl_.get(),ec);
        pool_->free_connection(pimpl_);
    }

    connection() = delete;
    connection(const connection& other) = delete;

    connection(connection&& other)
        : pimpl_(std::move(other.pimpl_))
    {
    }

    void connection_timeout(size_t val, std::error_code& ec)
    {
        pimpl_->connection_timeout(val, ec);
    }

    void execute(const std::string& query, std::error_code& ec)
    {
        if (!transaction_policy_.fail())
        {
            pimpl_->execute(query, ec);
            if (ec)
            {
                transaction_policy_.rollback();
            }
        }
    }

    void execute(const std::string& query, 
                 const std::function<void(const row& rec)>& callback,
                 std::error_code& ec)
    {
        if (!transaction_policy_.fail())
        {
            pimpl_->execute(query, callback, ec);
            if (ec)
            {
                transaction_policy_.rollback();
            }
        }
    }

    void commit(std::error_code& ec) 
    {
        transaction_policy_.commit(*pimpl_,ec);
    }

    friend prepared_statement<Bindings> make_prepared_statement(connection<Bindings,TP>& conn, const std::string& query, std::error_code& ec)
    {
        return prepared_statement<Bindings>(conn.pimpl_->prepare_statement(query, ec),&conn.transaction_policy_);
    }
};

// connection_pool

template <class Bindings>
class connection_pool
{
    std::string conn_string_;
    std::mutex connection_pool_mutex_;
    std::stack<std::unique_ptr<connection_impl>> free_connections_;
    size_t max_pool_size_ = 0;
public:
    connection_pool(const std::string& conn_string, size_t pool_size)
        : conn_string_(conn_string), max_pool_size_(pool_size)
    {
    }

    template <class TP = transaction_policy::auto_commit>
    connection<Bindings,TP> get_connection(std::error_code& ec)
    {
        std::lock_guard<std::mutex> lock(connection_pool_mutex_);
        TP tp;
        if (!free_connections_.empty())
        {
            auto conn_ptr = std::move(free_connections_.top());
            conn_ptr->auto_commit(tp.is_auto_commit(), ec);
            free_connections_.pop();
            return connection<Bindings,TP>(std::move(conn_ptr), std::move(tp), this);;
        }

        auto conn_ptr = Bindings::create_connection(conn_string_, ec);
        conn_ptr->auto_commit(tp.is_auto_commit(), ec);
        return connection<Bindings,TP>(std::move(conn_ptr), std::move(tp), this);
    }

    void free_connection(std::unique_ptr<connection_impl>& connection)
    {
        std::lock_guard<std::mutex> lock(connection_pool_mutex_);
        if (free_connections_.size() >= max_pool_size_)
        {
            // Do nothing
        }
        else
        {
            if (connection->is_valid())
            {
                free_connections_.push(std::move(connection));
            }
            else
            {
                // Do nothing
            }
        }
    }
};
 

}

#endif
