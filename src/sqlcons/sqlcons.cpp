#include <sqlcons_connector/odbc/connector.hpp>
#include <sqlcons/sqlcons.hpp>

namespace sqlcons {

// parameter<std::string>

parameter<std::string>::parameter(int sql_type_identifier,int c_type_identifier, const std::string& val)
    : parameter_base(sql_type_identifier, c_type_identifier)
{
    auto result1 = unicons::convert(val.begin(),val.end(),
                                    std::back_inserter(value_), 
                                    unicons::conv_flags::strict);
    value_.push_back(0);
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

void transaction::end_transaction(std::error_code& ec)
{
    pimpl_->end_transaction(ec);
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
