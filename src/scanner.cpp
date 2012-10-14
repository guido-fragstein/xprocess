
#include "scanner.hpp"

#include <boost/unordered_map.hpp>
#include <boost/range/adapters.hpp>
#include <boost/range/algorithm.hpp>

namespace parsers
{

struct rule_impl
{
    std::string identity_, result_;

    rule_impl( const std::string &identity ):
        identity_(identity)
    {
    }
    
    virtual bool parse( const std::string &input ) = 0;
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

    virtual bool parse( const std::string &input )
    {
        bool too_short = (input.size() < pos_) 
            or (input.size() < (pos_ + length_));
        
        if(too_short)
            return false;

        auto rpos = input.begin() + pos_;
        auto epos = input.begin() + (pos_ + length_);
        result_ = std::string(rpos, epos);
        return true;
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

    virtual bool parse( const std::string &input )
    {
        auto apos = input.find(start_);
        if(apos == std::string::npos)
           return false;

        auto start_read = apos + start_.size();
        auto start_read_itr = input.begin() + start_read;
        auto end_pos = input.find(end_, start_read);
        if(end_pos == std::string::npos)
            return false;

        result_ = std::string(start_read_itr, input.begin() + end_pos);
        return true;
    }
};

parser_rule::parser_rule( const std::string &ident, std::size_t offset, std::size_t length ):
    pimpl_( new offset_rule(ident, offset, length ))
{

}

parser_rule::parser_rule( const std::string &ident, const std::string &startdel, const std::string &enddel):
    pimpl_(new finder_rule(ident, startdel, enddel))
{

}

bool parser_rule::parse( const std::string &input ) const
{
    return pimpl_->parse(input);
}

std::string parser_rule::name() const
{
    return pimpl_->identity_;
}

std::string parser_rule::result() const
{
    return pimpl_->result_;
}

//--------------------------------------------------------------------------------
//end parser_rule 

struct message_parser::impl
{
    std::string name_;
    boost::unordered_map<std::string, parser_rule> parsers_;
    boost::unordered_map<std::string, std::string> result_map_;
};

message_parser::message_parser( const message_parser_factory &fc ):
    pimpl_(new message_parser::impl())
{
    pimpl_->name_ = fc.identity();
    for( auto &item: fc.items()) {
        pimpl_->parsers_.insert(boost::make_pair(item.identity(), item));
    }
}

bool message_parser::parse( const std::string &input )
{
    using boost::adaptors;

    bool found = true;
    boost::for_each(pimpl_->parsers_ | values, [&]( const parser_rule &rule ){
            if(!rule.parse(input)) 
            {
                found = false;
                break;
            }
            pimpl_->result_map_.insert(
                std::make_pair(rule.name(), rule.result()));
                        
        });

    if(!found)
        pimpl_->result_map_.clear();

    return found;
}

} //namespace parsers
