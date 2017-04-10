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

struct parameter_info
{
    parameter_info()
        : sql_type_identifier_(0),
        c_type_identifier_(0)
    {
    }
    parameter_info(int sql_type_identifier,int c_type_identifier)
        : sql_type_identifier_(sql_type_identifier), 
          c_type_identifier_(c_type_identifier)
    {
    }
    int sql_type_identifier_;
    int c_type_identifier_;
};

namespace detail
{

template<size_t __pos, class parameter_info, class Tuple>
struct json_tuple_helper
{
    using element_type = typename std::tuple_element<__pos - 1, Tuple>::type;
    using next = json_tuple_helper<__pos - 1, parameter_info, Tuple>;

    static void to_json(const Tuple& tuple, std::array<parameter_info, std::tuple_size<Tuple>::value>& jsons)
    {
        jsons[__pos - 1] = parameter_info(sql_type_traits<element_type>::sql_type_identifier(), sql_type_traits<element_type>::c_type_identifier());
        next::to_json(tuple, jsons);
    }
};

template<class parameter_info, class Tuple>
struct json_tuple_helper<0, parameter_info, Tuple>
{
    static void to_json(const Tuple& tuple, std::array<parameter_info, std::tuple_size<Tuple>::value>& json)
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
        using helper = detail::json_tuple_helper<std::tuple_size<Tuple>::value, parameter_info, Tuple>;
        
        const size_t num_elements = std::tuple_size<Tuple>::value;
        std::array<parameter_info,std::tuple_size<Tuple>::value> params;
        helper::to_json(parameters, params);

        std::cout << "Tuple size = " << num_elements << std::endl;

        for (size_t i = 0; i < params.size(); ++i)
        {
            std::cout << "sql_type_identifier=" << params[i].sql_type_identifier_ << std::endl;
        }
    }
private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}

#endif
