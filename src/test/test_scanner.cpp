
#include <boost/test/unit_test.hpp>
#include "../scanner.hpp"

#include <boost/unordered_map.hpp>
#include <string>

BOOST_AUTO_TEST_SUITE( scanner_tests )

struct test_result_handler : public parsers::result_handler
{
    boost::unordered_map<std::string, std::string> items_;
    std::string name_;

    virtual void parsed_successful( bool parsed )
    {

    }

    virtual void set_name( const std::string &name )
    {
        name_ = name;
    }

    virtual bool field_parsed( const std::string &name, const std::string &value )
    {
        items_.insert(std::make_pair(name,  value));
        return true;
    } 
};


//REQUIREMENT 001:
//parsing values by offset and lenght    
BOOST_AUTO_TEST_CASE( parser_rule )
{
    test_result_handler rs;
    const std::string teststring_valid("    12345   ");
    const std::string teststring_invalid("@@");

    bool success; std::string name, value;

    auto rule = parsers::parser_rule("name", 4, 5);
    
    bool sc = rule.parse(teststring_invalid, &rs);

    BOOST_CHECK_EQUAL(false, sc);

    sc = rule.parse(teststring_valid, &rs);

    BOOST_CHECK_EQUAL(true, sc);
    BOOST_CHECK_EQUAL(1, rs.items_.count("name"));
    BOOST_CHECK_EQUAL("12345", rs.items_["name"]);
}

//REQUIREMENT 002:
//parsing values by start and end patttern
BOOST_AUTO_TEST_CASE( finder_rule )
{
    test_result_handler rs;

    const std::string TEST1("@@GI|testentry@@");
    auto rule = parsers::parser_rule("name", "GI|", "@@");

    bool success = rule.parse(TEST1, &rs);

    BOOST_CHECK_EQUAL(true, success);
    BOOST_CHECK_EQUAL(1, rs.items_.count("name"));
    BOOST_CHECK_EQUAL("testentry", rs.items_["name"]);
}

//REQUIREMENT 003
//parsing a messages out of multiple values

BOOST_AUTO_TEST_CASE( message_parsing )
{
    const std::string TESTSTRING("001 002 12345@@BEGIN_TEXT|!!NEXT_TEXT<<");

    test_result_handler rs;
    //using namespace parsers;
    parsers::message_parser_factory fc;

    fc.identity("testparser");
    //fc.list_items_.push_back(parsers::parser_rule("name", 0, 3));
    fc.items( {{"ts1", 0, 3}, {"ts2", 4, 3}, 
        {"ts3", "@@", "|"}, {"ts4", "!!", "<<"}} );

    auto parser = parsers::message_parser(fc);

    bool sc = parser.parse(TESTSTRING, &rs);

    BOOST_CHECK_EQUAL(true, sc);
    BOOST_CHECK_EQUAL("001", rs.items_["ts1"]);
    BOOST_CHECK_EQUAL("002", rs.items_["ts2"]);
    BOOST_CHECK_EQUAL("BEGIN_TEXT", rs.items_["ts3"]);
}

BOOST_AUTO_TEST_SUITE_END()
