/*Scanner.hpp
 *
 * This file contain the Scanner class.
 * It gets input by the method "parse" that will take a string as argument.
 * Calling the parse method can trigger one or more events, in case the string
 * could parsed successfully, or exactly one error event, in case the string
 * could not be parsed at all.
 * */

#ifndef __SCANNER_DECLARE_GUARD_
#define __SCANNER_DECLARE_GUARD_

//std
#include <vector>
#include <string>

//boost
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>

namespace parsers
{

//TODO: adding validators to rules.

class result_handler
{
    public:
        virtual void parsed_successful( bool success ) = 0;
        virtual void set_name( const std::string &name ) = 0;
        virtual bool field_parsed( const std::string &name, const std::string &value ) = 0;
        virtual ~result_handler(){};
};

class rule_impl;
class parser_rule
{
    boost::shared_ptr<rule_impl> pimpl_;
    public:

        //this constructor will create a new offset rule. it will try to cut the string
        //from the offset pos until the length argument.
        parser_rule( const std::string &ident, std::size_t offset, std::size_t length );
        
        //this creates a finding rule. it will search in the input string for "startdel"
        //and will cut any char until it encounters enddel.
        parser_rule( const std::string &ident, 
                const std::string &startdel, const std::string &enddel );

        parser_rule(); //invalid empty rule. Needed vor conformance with std::vector

        std::string name() const;
        bool parse( const std::string &input, result_handler *rs ) const;
};


template<typename ItemType>
class generic_factory
{
    typedef std::vector<ItemType> item_list_type;

    std::string identity_;
    item_list_type list_items_;

    public:
    
    item_list_type& items() { return list_items_; }
    void items( const item_list_type &it ) { list_items_ = it; }
    void identity( const std::string &id ) { identity_ = id; }
    std::string identity() const { return identity_; }

};

typedef generic_factory<parser_rule> message_parser_factory;

//class message_parser is responsible to parse exactly one message complete.
//for example an interface sent a message to start a print process that
//contains the label name, an x-pos and an y-pos.
//In this case we would need to apply three parser rules to the message_parser,
//one for each field.
//The parser will return true if, and only if all rules are succeeded, means
//the label name, y and y pos were readable.

class message_parser
{
    class impl;
    boost::shared_ptr<impl> pimpl_;
    public:
        message_parser( message_parser_factory &fc );
        
        //return true only if all rules applied successfull
        bool parse( const std::string &input, result_handler *rs ) const; 

};

//class scanner hosts at least one or more message parser.
//The parser method returns true if exactly ONE of the hosted
//parsers succeeded. If more that one parsers succeed, the the
//configuration contains ambigous parsing rules and is therefore
//invalid. 
//If none of the parsers succeeded, that means a protocol breakthrough
//or an illegal or unsupported message has been received.

typedef generic_factory<message_parser> scanner_factory;

class scanner
{
    class impl;
    boost::shared_ptr<impl> pimpl_;
    public:
        scanner( scanner_factory &f );

        //return true if exactly one parser was successfull
        bool parse( const std::string &input, result_handler *rs ) const;
};

} //namespace parsers
#endif
