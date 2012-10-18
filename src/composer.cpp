
#include "composer.hpp"
#include <boost/unordered_map.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include <vector>
#include <sstream>

namespace composer
{

struct fragment
{
    virtual std::string value( const input *ip ) const = 0;
    virtual ~fragment(){};
    
    typedef boost::shared_ptr<fragment> pointer;
};

//literal just return the same value that is handled over
//by the constructor. the input parameter is ignored.
//--------------------------------------------------------------------------------

struct literal : public fragment
{
    std::string value_;
    literal( const std::string &v):
        value_(v)
    {
    }

    virtual std::string value( const input* ) const
    {
        return value_;
    }

};

//field must receive a value from an external dat source,
//provided by the input parameter.
//--------------------------------------------------------------------------------

struct field : public fragment
{
    std::string key_name_, format_;
    field( const std::string &key, const std::string &fm ):
        key_name_(key), 
        format_(fm)
    {
    
    }

    virtual std::string value( const input *ip ) const
    {
        return boost::str( boost::format(format_) % (*ip)[key_name_] );
    }
};

//return a value calculated by a build in function like i.e. now or repeat
//the input parameter is ignored
//--------------------------------------------------------------------------------

struct functional : public fragment
{
    std::string function_name_;
    std::vector<std::string> params_;

    functional(const std::string &n, const std::vector<std::string> &p):
        function_name_(n),
        params_(p)
    {
    
    }

    virtual std::string value( const input* ) const
    {
        return "dolly buster";
    }

};

struct message::impl
{
    std::string name_;
    std::vector<fragment::pointer> fragment_vector_;

    static void parse( impl &imp, const std::string &format);
};

//\$START: $(id:%2.2d)\\ $now(%H%M%Y)01$(charge:%04.4d) \
// $(create)$repeat( ,12)$checksum(%2.2X,int)

struct fragment_buffer
{
    std::vector<fragment::pointer>& vec;
    std::vector<std::string> param;
    std::string format, buffer;

    fragment_buffer( std::vector<fragment::pointer> &p ):
        vec(p)
    {

    }

    void operator += (char c)
    {
        buffer += c;
    }

    void shift_literal()
    {
        vec.push_back( boost::make_shared<literal>(buffer) );
        buffer.clear();
    }

    void shift_function()
    {
        vec.push_back( boost::make_shared<functional>(buffer, param));
        param.clear();
        buffer.clear();
    }

    void shift_field()
    {
        vec.push_back( boost::make_shared<field>(buffer, format));
        buffer.clear();
        format.clear();
    }
    
};

void message::impl::parse(message::impl &imp, const std::string &format )
{
    enum states{ sstart, sliteral, sesc, scommand, 
        sfield, sfield_format, sparam_list, sfunction };

    auto itr = format.begin();
    auto nd = format.end();
    states state = sstart;
    fragment_buffer buffer(imp.fragment_vector_);

    for(; itr != nd; ++itr )
    {
        switch(state)
        {
            case sstart:
                switch(*itr)
                {
                    case '\\':
                        state = sesc;
                        break;
                    case '$':
                        state = scommand;
                        break;
                    default:
                        state = sliteral;
                        buffer += *itr;
                        break;
                }
                break;
            case sesc:
                switch(*itr)
                {
                    case '\xA':
                    case '\xD':
                        break;
                    case '$':
                    case '\\':
                        buffer += *itr;
                        state = sliteral;
                        break;
                    default:
                        buffer += *itr;
                        state = sliteral;
                        break;
                }
                break;
            case sliteral:
                switch(*itr)
                {
                    case '$':
                        buffer.shift_literal();
                        state = scommand;
                        break;
                    case '\\':
                        state = sesc;
                        break;
                    default:
                        buffer += *itr;
                        break;
                        
                }
                break;
            case scommand:
                switch(*itr)
                {
                    case '(': //open bracet directly after dollar means
                            //we found a filed definition
                        state = sfield;
                        break;
                    default:
                        state = sfunction;
                        buffer += *itr;
                        break;
                }
                break;
            case sfunction:
                switch(*itr)
                {
                    case '(': //open bracet after a name means function call
                        state = sparam_list;
                        break;
                    default:
                        buffer += *itr;
                        break;
                }
                break;
            case sparam_list:
                switch(*itr)
                {
                    case ',':
                        buffer.param.push_back("");
                        break;
                    case ')':
                        buffer.shift_function();
                        state = sstart;
                        break;
                    default:
                        buffer.param.back() += *itr;
                        break;
                }
                break;
            case sfield:
                switch(*itr)
                {
                    case ':':
                        state = sfield_format;
                        break;
                    case ')':
                        state = sstart;
                        break;
                    default:
                        buffer += *itr; 
                }
                break;
            case sfield_format:
                switch(*itr)
                {
                    case ')':
                        buffer.shift_field();
                        state = sstart;
                        break;
                    default:
                        buffer.format += *itr;
                }
                break;
        }
        
    }
}


message::message( const std::string &name, const std::string &format ):
    pimpl_(new impl())
{
    message::impl::parse(*pimpl_, format);
}

std::string message::format( const input *inp )
{
    std::stringstream ss;
    for( auto &val : pimpl_->fragment_vector_)
    {
        ss << val->value(inp);
    }

    return ss.str();
}

}
