
#include <boost/test/unit_test.hpp>

#include "../validation.hpp"

BOOST_AUTO_TEST_SUITE( validations )

struct test_signature : validation::signature
{
    std::string s_;
    std::vector<std::string> vc_;
    test_signature( const std::string &s):
    s_(s)
    {
        
    }

    virtual const std::string& name() const
    {
        return s_;
    } 

    virtual const std::vector<std::string>& params() const
    {
        return vc_;
    }
};

BOOST_AUTO_TEST_CASE( validation_factory )
{
    test_signature sg("is_numeric");
    auto fc = validation::factory();
    auto val = fc.get(&sg);
    auto rs = val->validate("12938");

    BOOST_CHECK_EQUAL(true, rs);

    rs = val->validate("Jochen Fugalla");
    BOOST_CHECK_EQUAL(false, rs);

    sg.s_ = "is_date";
    sg.vc_.push_back("%d.%m.%y");

    auto dval = fc.get(&sg);
    rs = dval->validate("22.10.2014");

    BOOST_CHECK_EQUAL(true, rs);

}

BOOST_AUTO_TEST_SUITE_END()

