#ifndef SQLCONS_HPP
#define SQLCONS_HPP

#include <memory>
#include <string>
#include <system_error>
#include <functional>
#include <vector>
#include <map>
#include <array>

namespace sqlcons {

template <class T, class Enable=void>
struct sql_type_traits
{
};

struct sql_data_types
{
    static const int integer_id;
};

struct sql_c_data_types
{
    static const int integer_id;
};

// integral

template<class T>
struct sql_type_traits<T,
                       typename std::enable_if<std::is_integral<T>::value &&
                       std::is_signed<T>::value &&
                       !std::is_same<T,bool>::value
>::type>
{
    typedef int64_t value_type;
    static int sql_type_identifier() { return sql_data_types::integer_id; }
    static int c_type_identifier() { return sql_c_data_types::integer_id; }
};

// conv_errc

enum class sql_errc 
{
    db_err = 1,
    E_01000,
    E_08S01,
    E_HY000,
    E_HY001,
    E_HY008,
    E_HY010,
    E_HY013,
    E_HY117,
    E_HYT01,
    E_IM001,
    E_IM017,
    E_IM018
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
        case sql_errc::E_01000:
            return "General warning";
        case sql_errc::E_08S01:
            return "Communication link failure";
        case sql_errc::E_HY000:
            return "General error";
        case sql_errc::E_HY001:
            return "Memory allocation error";
        case sql_errc::E_HY008:
            return "Operation canceled";
        case sql_errc::E_HY010:
            return "Function sequence error";
        case sql_errc::E_HY013:
            return "Memory management error";
        case sql_errc::E_HY117:
            return "Connection is suspended due to unknown transaction state";
        case sql_errc::E_HYT01:
            return "Connection timeout expired";
        case sql_errc::E_IM001:
            return "Driver does not support this function";
        case sql_errc::E_IM017:
            return "Polling is disabled in asynchronous notification mode";
        case sql_errc::E_IM018:
            return "SQLCompleteAsync has not been called to complete the previous asynchronous operation on this handle. If the previous function call on the handle returns SQL_STILL_EXECUTING and if notification mode is enabled, SQLCompleteAsync must be called on the handle to do post-processing and complete the operation";
        default:
            return "db error";
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

struct parameter_binding
{
    parameter_binding()
        : sql_type_identifier_(0),
        c_type_identifier_(0)
    {
    }
    parameter_binding(int sql_type_identifier,int c_type_identifier)
        : sql_type_identifier_(sql_type_identifier), 
          c_type_identifier_(c_type_identifier)
    {
    }
    virtual void* pvalue() = 0;

    int parameter_type() const
    {
        return sql_type_identifier_;
    }

    int value_type() const
    {
        return c_type_identifier_;
    }

    size_t column_size() const
    {
        return 0;
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
struct parameter : public parameter_binding
{
    parameter(int sql_type_identifier,int c_type_identifier, const T& value)
        : parameter_binding(sql_type_identifier, c_type_identifier), value_(value)
    {
    }

    void* pvalue() override
    {
        std::cout << "pvalue() value: " << value_ << std::endl;
        return &value_;
    }

    T value_;
    size_t ind_;
};

namespace detail
{

template<size_t Pos, class parameter_binding, class Tuple>
struct sql_parameters_tuple_helper
{
    using element_type = typename std::tuple_element<Pos - 1, Tuple>::type;
    using next = sql_parameters_tuple_helper<Pos - 1, parameter_binding, Tuple>;

    static void to_parameters(const Tuple& tuple, std::vector<std::unique_ptr<parameter_binding>>& bindings)
    {
        bindings[Pos - 1] = std::make_unique<parameter<typename sql_type_traits<element_type>::value_type>>(sql_type_traits<element_type>::sql_type_identifier(), sql_type_traits<element_type>::c_type_identifier(),std::get<Pos-1>(tuple));
        next::to_parameters(tuple, bindings);
    }
};

template<class parameter_binding, class Tuple>
struct sql_parameters_tuple_helper<0, parameter_binding, Tuple>
{
    static void to_parameters(const Tuple& tuple, std::vector<std::unique_ptr<parameter_binding>>& json)
    {
    }
};

}

class sql_prepared_statement
{
public:
    sql_prepared_statement();
    ~sql_prepared_statement();

    void prepare(sql_connection& conn, const std::string& query, std::error_code& ec);

    void execute(std::error_code& ec);

    template <typename Tuple>
    void execute(const Tuple& parameters,
                 const std::function<void(const sql_record& record)>& callback,
                 std::error_code& ec)
    {
        using helper = detail::sql_parameters_tuple_helper<std::tuple_size<Tuple>::value, parameter_binding, Tuple>;
        
        const size_t num_elements = std::tuple_size<Tuple>::value;
        std::vector<std::unique_ptr<parameter_binding>> params(std::tuple_size<Tuple>::value);
        helper::to_parameters(parameters, params);

        std::cout << "Tuple size = " << num_elements << std::endl;

        do_execute(params,callback,ec);
    }
private:
    void do_execute(std::vector<std::unique_ptr<parameter_binding>>& bindings,
        const std::function<void(const sql_record& record)>& callback,
        std::error_code& ec);

    class impl;
    std::unique_ptr<impl> pimpl_;
};

}

#endif
