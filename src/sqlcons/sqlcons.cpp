#include <sqlcons_connector/odbc/connector.hpp>
#include <sqlcons/sqlcons.hpp>

namespace sqlcons {

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
