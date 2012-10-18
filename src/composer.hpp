#ifndef __COMPSER_INCLUDE_GUARD_18_16__
#define __COMPSER_INCLUDE_GUARD_18_16__

#include <string>
#include <boost/shared_ptr.hpp>

namespace composer
{

class input
{
    public:
        virtual const std::string& name() const = 0;
        virtual std::string operator[]( const std::string &key) const = 0;
        virtual ~input(){};
};

class message
{

    class impl;
    boost::shared_ptr<impl> pimpl_;
    public:
        message( const std::string &name, const std::string &format );
        std::string format( const input *inp );
};

}

#endif
