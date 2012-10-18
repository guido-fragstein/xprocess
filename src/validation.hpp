/*
 possible validations:
    is_numeric
    is_integer
    is_date(format)
    is_time(format)
    is_datetime(format)
    is_equal_to(string)

 * */
#ifndef __VALIDATION_INCLUDE_WATCH_2328__
#define __VALIDATION_INCLUDE_WATCH_2328__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>

namespace validation
{

class base
{
    public:
        virtual bool validate( const std::string &input ) = 0;
        virtual ~base(){}
};


class signature
{
    public:
        virtual const std::string& name() const = 0;
        virtual const std::vector<std::string>& params() const = 0;
        virtual ~signature(){};
};

typedef boost::shared_ptr<base> pointer;

class factory
{
    class impl;
    boost::shared_ptr<impl> pimpl_;
    public:
        factory();
        pointer get( signature *sig );
};

}
#endif
