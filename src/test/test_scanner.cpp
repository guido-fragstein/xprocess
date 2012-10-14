
#include <boost/test/unit_test.hpp>
#include "../scanner.hpp"

BOOST_AUTO_TEST_SUITE( scanner_tests )

BOOST_AUTO_TEST_CASE( parser_rule )
{
    const std::string teststring_valid("    12345   ");
    const std::string teststring_invalid("@@");

    auto rule = parsers::parser_rule("name", 4, 5);

    bool rs1 = rule.parse(teststring_invalid);
    BOOST_CHECK_EQUAL(false, rs1);

    bool rs = rule.parse(teststring_valid);
    std::cout << rule.result() << std::endl;

    BOOST_CHECK_EQUAL(true, rs);
    BOOST_CHECK_EQUAL("12345", rule.result());
}

BOOST_AUTO_TEST_CASE( finder_rule )
{
    const std::string TEST1("@@GI|testentry@@");
    auto rule = parsers::parser_rule("name", "GI|", "@@");
    rule.parse(TEST1);

    BOOST_CHECK_EQUAL("testentry", rule.result());
}

BOOST_AUTO_TEST_SUITE_END()
