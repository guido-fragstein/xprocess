
#include "composer.hpp"
#include "common.hpp"
#include <boost/unordered_map.hpp>
#include <boost/functional/factory.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include <boost/date_time.hpp>
#include <vector>
#include <sstream>
#include <iostream> //debug

namespace composer
{

struct fragment
{
    virtual void value( const input *ip, std::string *value ) const = 0;
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

    virtual void value( const input*, std::string *value ) const
    {
        (*value) += value_;
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

    virtual void value( const input *ip, std::string *value ) const
    {
        (*value) += boost::str( boost::format(format_) % (*ip)[key_name_] );
    }
};

namespace functions
{

    struct function_base
    {
        function_base( const common::string_list &list ){}
        virtual void operator()( std::string *str ) = 0;
        virtual ~function_base(){}
        
    };

    struct checksum : public function_base
    {
        bool int_output_;
        std::string format_;
        checksum( const common::string_list &list ):
            function_base(list)
        {
            if(list.size() < 1)
                throw std::string("arity error: in checksum function are at least 1 parameter expected");

            format_ = list[0];
            int_output_ = list[1] != "false";
        }

        virtual void operator()( std::string *rs )
        {
            auto itr = rs->begin();
            auto nd = rs->end();

            if(*itr == (char)2) ++itr;
            std::size_t int_res;
            char char_res;

            if( int_output_ )
            {
                for(; itr != nd; ++itr) int_res ^= *itr;
                *rs += boost::str(boost::format(format_) % int_res);
            }

            for(; itr != nd; ++itr) char_res ^= *itr;
                *rs += char_res;

        }
    };

    struct now : public function_base
    {
        std::stringstream ss_;
        now( const common::string_list &list ):
            function_base(list)
        {

            if(list.size() < 1 )
                throw std::string("arity error: now function needs at least one argument");

            namespace tm = boost::posix_time;
            auto facet = new tm::time_facet(list[0].c_str());
            std::stringstream ss_;
            ss_.imbue(std::locale(std::locale::classic(), facet));
            
        }

        virtual void operator()( std::string *rs )
        {
            namespace tm = boost::posix_time;
            ss_.str(std::string());
            ss_ << tm::second_clock::local_time();
            *rs += ss_.str();
        }
    };

    struct repeat : public function_base
    {
        std::size_t nr_repeats_;
        char char_;
        repeat( const common::string_list &list ):
            function_base(list)
        {
            if(list.size() < 2)
                throw std::string("arity error: repeat function needs two argument");

            std::stringstream ss(list[0]);
            std::size_t ascii_code;
            if( ss >> ascii_code )
                char_ = (char)(ascii_code);
            else
                char_ = list[0][0];

            ss.str(list[1]);
            if(!(ss >> nr_repeats_))
                throw std::string("argument error: function repeat needs a numeric argument at 2nd position");            
        }

        virtual void operator()( std::string *rs )
        {
            for( auto i = 0; i < nr_repeats_; ++i )
                *rs += char_;
        }
    };

}

//return a value calculated by a build in function like i.e. now or repeat
//the input parameter is ignored
//--------------------------------------------------------------------------------

struct functional : public fragment
{
    typedef boost::function <  functions::function_base*( const common::string_list &list ) > factory_type;        
    typedef boost::unordered_map< std::string, factory_type > functions_map_type;
   
    static functions_map_type functions_map_;

    static bool init_map()
    {
        functions_map_ = { 
            {"repeat", boost::factory<functions::repeat*>()}, 
            {"now", boost::factory<functions::now*>()},
            {"checksum", boost::factory<functions::checksum*>()} 
        }; 

        return true;
    }

    std::string function_name_;
    common::string_list params_;
    boost::shared_ptr<functions::function_base> function_;

    functional(const std::string &n, const common::string_list &p):
        function_name_(n),
        params_(p)
    {
        auto itr = functions_map_.find(function_name_);
        if(itr == functions_map_.end())
            throw std::string("no such function: ") + function_name_; 
        function_.reset( itr->second(params_) );
    }

    virtual void value( const input*, std::string *value ) const    
    {
        (*function_)(value);
    }

};

functional::functions_map_type functional::functions_map_;// = functional::functions_map_type();
static bool initialized = functional::init_map();

struct message::impl
{
    std::string name_;
    std::vector<fragment::pointer> fragment_vector_;

    static void parse( impl &imp, const std::string &format);
};

//\$START: $(id:%2.2d)\\ $now(%H%M%Y)01$(charge:%04.4d) \
// $(create)$repeat( ,12)$checksum(%2.2X,int)

//fragment buffer is used to add items to the object tree, built by the
//parser algorithm
//--------------------------------------------------------------------------------

struct fragment_buffer
{
    std::vector<fragment::pointer>& vec;
    common::string_list param_vec;
    std::string format, buffer, param, ascii_code;

    fragment_buffer( std::vector<fragment::pointer> &p ):
        vec(p)
    {

    }

    void operator += (char c)
    {
        buffer += c;
    }

    void shift_param()
    {
        param_vec.push_back(param);
        param.clear();
    }

    void shift_literal()
    {
        vec.push_back( boost::make_shared<literal>(buffer) );
        buffer.clear();
    }

    void shift_function()
    {
        vec.push_back( boost::make_shared<functional>(buffer, param_vec));
        param.clear();
        buffer.clear();
    }

    void shift_field()
    {
        vec.push_back( boost::make_shared<field>(buffer, format));
        buffer.clear();
        format.clear();
    }
    
    void apply_ascii_code()
    {
        std::stringstream ss(ascii_code);
        std::size_t ascii;
        if((ss >> ascii) && (ascii < 127) )
        {            
            buffer += (char)ascii;
            ascii_code.clear();
            return;
        }

        throw std::string("illegal asci char code ") + ascii_code;
    }
};

void message::impl::parse(message::impl &imp, const std::string &format )
{
    enum states{ sstart, sliteral, sesc, scommand, 
        sfield, sfield_format, sparam_list, sfunction, ascii_code };

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
                    case '#':
                        state = ascii_code;
                    case '$':
                        state = scommand;
                        break;
                    default:
                        state = sliteral;
                        buffer += *itr;
                        break;
                }
                break;
            case ascii_code:
                switch(*itr)
                {
                    case ';':
                        buffer.apply_ascii_code();
                        state = sstart;
                        break;
                    default:
                        buffer.ascii_code += *itr;
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
                    case '#':
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
                        std::cout << "function" << std::endl;
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
                        buffer.shift_param();
                        break;
                    case ')':
                        buffer.shift_function();
                        state = sstart;
                        break;
                    default:
                        buffer.param += *itr;
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
    //std::stringstream ss;
    std::string result;
    for( auto &val : pimpl_->fragment_vector_)
    {
        val->value(inp, &result);
        //ss << val->value(inp);
    }

    return result;
}

}
