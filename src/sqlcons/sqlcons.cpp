#include <sqlcons_connector/odbc/connector.hpp>
#include <sqlcons/sqlcons.hpp>

namespace sqlcons {

// connection

connection::connection() : pimpl_(new connection_impl()) {}

connection::~connection() = default;

void connection::open(const std::string& connString, bool autoCommit, std::error_code& ec)
{
    pimpl_->open(connString, autoCommit, ec);
}

void connection::auto_commit(bool val, std::error_code& ec)
{
    pimpl_->auto_commit(val, ec);
}

void connection::connection_timeout(size_t val, std::error_code& ec)
{
    pimpl_->connection_timeout(val, ec);
}

prepared_statement connection::prepare_statement(const std::string& query, std::error_code& ec)
{
    return prepared_statement(pimpl_->prepare_statement(query, ec));
}

prepared_statement connection::prepare_statement(const std::string& query, transaction& trans)
{
    return prepared_statement(pimpl_->prepare_statement(query, trans));
}

void connection::execute(const std::string& query, 
                         std::error_code& ec)
{
    pimpl_->execute(query, ec);
}

void connection::execute(const std::string& query, 
                         const std::function<void(const row& rec)>& callback,
                         std::error_code& ec)
{
    pimpl_->execute(query, callback, ec);
}

// parameter<std::string>

parameter<std::string>::parameter(int sql_type_identifier,int c_type_identifier, const std::string& val)
    : base_parameter(sql_type_identifier, c_type_identifier)
{
    auto result1 = unicons::convert(val.begin(),val.end(),
                                    std::back_inserter(value_), 
                                    unicons::conv_flags::strict);
    value_.push_back(0);
}

// prepared_statement

prepared_statement::prepared_statement() : pimpl_(new prepared_statement_impl()) {}

prepared_statement::prepared_statement(std::unique_ptr<prepared_statement_impl>&& impl) : pimpl_(std::move(impl)) {}

prepared_statement::~prepared_statement() = default;

void prepared_statement::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                        const std::function<void(const row& rec)>& callback,
                                        std::error_code& ec)
{
    pimpl_->execute_(bindings, callback, ec);
}

void prepared_statement::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                        std::error_code& ec)
{
    pimpl_->execute_(bindings, ec);
}

void prepared_statement::execute_(std::vector<std::unique_ptr<base_parameter>>& bindings, 
                                  transaction& t)
{
    pimpl_->execute_(bindings, t);
}

// row

row::row(std::vector<value*>&& values)
    : values_(std::move(values))
{
}

row::~row() = default;

size_t row::size() const
{
    return values_.size();
}

const value& row::operator[](size_t index) const
{
    return *values_[index];
}

}
