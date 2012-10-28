
#include <boost/test/unit_test.hpp>
#include "../composer.hpp"

#include <boost/unordered_map.hpp>

BOOST_AUTO_TEST_SUITE( composer_test )

struct dummy_input : public composer::input
{

    std::string name_;
    dummy_input( const std::string &s ):
        name_(s)
    {
    
    }

    const std::string& name() const
    {
        return name_;
    }
    
    boost::unordered_map<std::string, std::string> map_;

    virtual std::string operator[]( const std::string &str ) const
    {
        auto f = map_.find(str);
        if(f == map_.end())
            return "";

        return f->second;
    }
};

BOOST_AUTO_TEST_CASE( compose_this_man )
{

    {
        auto cp = composer::message("test1record", "#2;test#10;#13;test\\#10;");
        dummy_input ipt("test1record");
        auto rs = cp.format(&ipt);
        BOOST_CHECK_EQUAL("\x02test\x0A\x0Dtest\\#10;", rs);
    }

    dummy_input ipt("testrecord");
    ipt.map_["feld1"] = "103";
    ipt.map_["name"] = "max brause";
    ipt.map_["feld2"] = "407";

    auto cp = composer::message("testrecord", 
            "\\$START $(feld1:%3.3d) $(name:%s)");

    auto result = cp.format(&ipt);
    BOOST_CHECK_EQUAL("$START 103 max brause", result);
}

BOOST_AUTO_TEST_SUITE_END()

