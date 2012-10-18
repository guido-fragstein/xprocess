
#include "scanner.hpp"

#include <boost/unordered_map.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>

namespace parsers
{

namespace
{
    typedef boost::tuple<bool, std::string, std::string> rule_result_type;

    inline rule_result_type false_value( const std::string &n )
    { 
        return boost::make_tuple(false, n, "");
    }

    inline rule_result_type true_value(const std::string &n, const std::string &v )
    {
        return boost::make_tuple(true, n, v);
    }
}

struct rule_impl
{
    std::string identity_;

    rule_impl( const std::string &identity ):
        identity_(identity)
    {
    }
    
    virtual bool parse( const std::string &input, result_handler *rs ) = 0;
    virtual ~rule_impl(){};
};

struct offset_rule : public rule_impl
{
    std::size_t pos_, length_;
    offset_rule( const std::string &id, std::size_t pos, std::size_t len ):
        rule_impl(id),
        pos_(pos),
        length_(len)
    {

    }

    virtual bool parse( const std::string &input, result_handler *rs )
    {
        bool too_short = (input.size() < pos_) 
            or (input.size() < (pos_ + length_));
        
        if(too_short)
            return false;

        auto rpos = input.begin() + pos_;
        auto epos = input.begin() + (pos_ + length_);
        return rs->field_parsed(identity_, std::string(rpos, epos));
        //return true; 
    }
};

struct finder_rule : public rule_impl
{

    std::string start_, end_;
    finder_rule( const std::string &id, const std::string &start, const std::string &end ):
        rule_impl(id),
        start_(start),
        end_(end)
    {
    };

    virtual bool parse( const std::string &input, result_handler *rs )
    {
        auto apos = input.find(start_);
        if(apos == std::string::npos)
           return false; //false_value(identity_); 

        auto start_read = apos + start_.size();
        auto start_read_itr = input.begin() + start_read;
        auto end_pos = input.find(end_, start_read);
        if(end_pos == std::string::npos)
            return false; //false_value(identity_);
        
        
        return rs->field_parsed(identity_, std::string(start_read_itr, input.begin() + end_pos));
        //return true;
    }
};

//--------------------------------------------------------------------------------

parser_rule::parser_rule()
{
}


parser_rule::parser_rule( const std::string &ident, std::size_t offset, std::size_t length ):
    pimpl_( new offset_rule(ident, offset, length ))
{

}

parser_rule::parser_rule( const std::string &ident, const std::string &startdel, const std::string &enddel):
    pimpl_(new finder_rule(ident, startdel, enddel))
{

}

bool parser_rule::parse( const std::string &input, result_handler *rs ) const
{
    return pimpl_->parse(input, rs);
}

std::string parser_rule::name() const
{
    return pimpl_->identity_;
}


//--------------------------------------------------------------------------------
//end parser_rule 

struct message_parser::impl
{
    std::string name_;
    boost::unordered_map<std::string, parser_rule> parsers_;
};

message_parser::message_parser( message_parser_factory &fc ):
    pimpl_(new impl())
{
    pimpl_->name_ = fc.identity();
    for( const auto &item: fc.items()) {
        pimpl_->parsers_.insert(std::make_pair(item.name(), item));
    }
}

bool message_parser::parse( const std::string &input, result_handler *rs ) const
{

    bool found = true;

    for(const auto &item: pimpl_->parsers_ )
    {
        if(!item.second.parse(input, rs))
        {
            found = false;
            break;
        }
    }

    if(!found)
        return false;

    rs->set_name(pimpl_->name_);
    rs->parsed_successful(true);
    return true;
}

//--------------------------------------------------------------------------------
//end message_parser

struct scanner::impl
{
    std::string name_;
    std::vector<message_parser> values_;
};


scanner::scanner( scanner_factory &f ):
    pimpl_(new impl())
{
    pimpl_->name_ = f.identity();
    pimpl_->values_ = f.items();
}

bool scanner::parse( const std::string &input, result_handler *rs ) const
{
    for( const auto &item: pimpl_->values_)
    {
        if(item.parse(input, rs))
            return true;
    }
    rs->parsed_successful(false);
    return false;
}



} //namespace parsers
