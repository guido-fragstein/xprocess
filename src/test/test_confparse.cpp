
#include "../confparse.hpp"

//std
#include <sstream>
#include <string>
#include <vector>

//boost
#include <boost/tuple/tuple.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

//inline libs
#include "../pugixml.hpp"

//system
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

typedef std::vector
< 
    boost::tuple<std::string, std::string, std::size_t>     
> 
endpoint_vec_type;

//fixture create a xml file and deletes it on destruction
struct xml_file_fixture
{
    boost::filesystem::path file_path_;

    xml_file_fixture( boost::filesystem::path &path ):
        file_path_(path)
    {
        namespace fs = boost::filesystem;
        if(fs::exists(path))
            fs::remove(path);
        pugi::xml_document doc;
        auto devices = doc.append_child("devices");
        
        endpoint_vec_type endpoints = {
            {"127.0.0.1", "10266", 1},
            {"127.0.0.1", "10267", 2},
            {"127.0.0.1", "10269", 3}
        };

        for( auto itr: endpoints ) {
            std::string adr, p; std::size_t tp;
            boost::tie(adr, p, tp) = itr;
            auto device = devices.append_child("dev");
            device.append_attribute("ip") = adr.c_str();
            device.append_attribute("port") = p.c_str();
            device.append_attribute("type") = boost::lexical_cast<std::string>(tp).c_str();
        }        
        doc.save_file(path.string().c_str());

    }

    ~xml_file_fixture()
    {
        namespace fs = boost::filesystem;
        if(fs::exists(file_path_))
            fs::remove(file_path_);
    }
};



BOOST_AUTO_TEST_SUITE( configuration )

const boost::filesystem::path FIXTURE_PATH("/home/guido/Documents/playground/cpp/bserv/src/test/fixtures");

BOOST_AUTO_TEST_CASE( find_file )
{
    namespace fs = boost::filesystem;

    fs::path xml_file(FIXTURE_PATH / "xml/devices.xml");
    xml_file_fixture fixture(xml_file);

    try
    {
        {
            auto parser = configuration::parser(xml_file);    

            endpoint_vec_type endpoint_vec;

            parser.each_device(
                    [&]( const std::string &adr, const std::string &port, std::size_t type ){
                    endpoint_vec.push_back(boost::make_tuple(adr, port, type));            
                    });

            BOOST_CHECK_EQUAL(3, endpoint_vec.size());

            std::string adr, port; std::size_t type;
            boost::tie(adr, port, type) = endpoint_vec[0];

            BOOST_CHECK_EQUAL("10266", port);
            BOOST_CHECK_EQUAL("127.0.0.1", adr);
            BOOST_CHECK_EQUAL(1, type);

            boost::tie(adr, port, type) = endpoint_vec[2];

            BOOST_CHECK_EQUAL("10269", port);
            BOOST_CHECK_EQUAL("127.0.0.1", adr);
            BOOST_CHECK_EQUAL(3, type);

            parser.add("10.0.0.124", "10444", 2);
        }

        {
            endpoint_vec_type endpoint_vec;
            auto parser = configuration::parser(xml_file);
            parser.each_device(
                    [&]( const std::string &adr, const std::string &port, std::size_t type ){
                    endpoint_vec.push_back(boost::make_tuple(adr, port, type));            
                    });

            BOOST_REQUIRE_EQUAL(4, endpoint_vec.size());
            std::string adr, port; std::size_t type;
            boost::tie(adr, port, type) = endpoint_vec[3];

            BOOST_CHECK_EQUAL("10.0.0.124", adr);
            BOOST_CHECK_EQUAL("10444", port);
            
            parser.del("127.0.0.1", "10266");
        }

        {
            endpoint_vec_type endpoint_vec;
            auto parser = configuration::parser(xml_file);
            parser.each_device(
                    [&]( const std::string &adr, const std::string &port, std::size_t type ){
                    endpoint_vec.push_back(boost::make_tuple(adr, port, type));            
                    });

            BOOST_REQUIRE_EQUAL(3, endpoint_vec.size());
        }
    }

    catch(configuration::configuration_exception &cex )
    {
        std::stringstream ss;
        ss << "error in configuration: " << cex.what() << std::endl;
        BOOST_FAIL(ss.str());
    }
}

std::size_t copy_configuration( configuration::parser parser, endpoint_vec_type &eps )
{
    parser.each_device([&]( const std::string &a, const std::string &p, std::size_t t){
               eps.push_back(boost::make_tuple(a, p, t)); 
            });
    return eps.size();
}

BOOST_AUTO_TEST_CASE( empty_file )
{
    namespace fs = boost::filesystem;

    auto xml_path = fs::absolute(".").parent_path().parent_path() / "faxoblitz.xml";
    if(fs::exists(xml_path))
        fs::remove(xml_path);

    try
    {
        {
            auto parser = configuration::parser(xml_path);
            endpoint_vec_type ep_vec;
            auto size = copy_configuration(parser, ep_vec);

            BOOST_REQUIRE_EQUAL(0, size);

            parser.add("192.168.1.102", "10456", 1);
            parser.add("214.198,20.107", "15444", 4);
        }

        {
            auto parser = configuration::parser(xml_path);
            endpoint_vec_type ep_vec;
            auto size = copy_configuration(parser, ep_vec);

            BOOST_REQUIRE_EQUAL(2, size);
        }
    }

    catch( configuration::configuration_exception &exp )
    {
        auto errstr = std::string("in test case empty_file: ") + exp.what();
        BOOST_FAIL(errstr);
    }
}

BOOST_AUTO_TEST_SUITE_END()

