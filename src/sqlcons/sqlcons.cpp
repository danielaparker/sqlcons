#include <sqlcons_connector/odbc/connector.hpp>
#include <sqlcons/sqlcons.hpp>

namespace sqlcons {

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

transaction::transaction(std::unique_ptr<transaction_impl>&& impl) : pimpl_(std::move(impl)) {}

transaction::~transaction() = default;

std::error_code transaction::error_code() const
{
    return pimpl_->error_code();
}

void transaction::update_error_code(std::error_code ec)
{
    pimpl_->update_error_code(ec);
}

void transaction::end(std::error_code& ec)
{
    pimpl_->end(ec);
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
