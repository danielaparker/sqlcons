#ifndef SQLCONS_CONNECTOR_CONNECTORFWD_HPP
#define SQLCONS_CONNECTOR_CONNECTORFWD_HPP

namespace sqlcons { 

class connection_impl;
class transaction_impl;
class prepared_statement_impl;

template <class T>
struct sql_type_traits
{
    typedef T value_type;
    static int sql_type_identifier();
    static int c_type_identifier();
};

}

#endif
