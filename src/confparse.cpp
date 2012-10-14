
#include "confparse.hpp"
#include "pugixml.hpp"

#include <sstream>
#include <boost/lexical_cast.hpp>

namespace configuration
{
//exception member functions
//--------------------------------------------------------------------------------
configuration_exception::configuration_exception( const std::string &str ):
    what_(str)
{

}

std::string configuration_exception::what() const
{
    return what_;
}

//implementation holds all object data of the parser
//--------------------------------------------------------------------------------
struct parser::impl
{
    boost::filesystem::path filename_;
    pugi::xml_document doc_;
    pugi::xml_node node_;
};



namespace validate
{

    void no_such_file( const boost::filesystem::path &filename )
    {
        namespace fs = boost::filesystem;
        if(fs::exists(filename))
            return;
        std::stringstream ss;
        ss << "during parser init: no such file: " 
            << filename << std::endl;
        throw configuration_exception(ss.str());

        std::cout << "fnord 3" << std::endl;
    }

    void invalid_xml( const pugi::xml_parse_result &rs )
    {
        if(((bool)rs) == true)
            return;

        std::stringstream ss;
        ss << "error in input xml file: " << rs.description() << std::endl;
        throw configuration_exception(ss.str());
    }

    void no_such_node( const pugi::xml_node &node )
    {
        if(node)
            return;
        
        throw configuration_exception("error reading config file: missing xmlnode devices");
    }
}
//--------------------------------------------------------------------------------
//CTOR
parser::parser( const boost::filesystem::path &location ):
    pimpl_( new impl())
{

    namespace fs = boost::filesystem;
    pimpl_->filename_ = location;
    if(fs::exists(location))
    {
        validate::invalid_xml(pimpl_->doc_.load_file(location.string().c_str()));    
        validate::no_such_node(
                pimpl_->node_ = pimpl_->doc_.select_single_node("/devices").node());        
    }

    else
    {
        pimpl_->node_ = pimpl_->doc_.append_child("devices");
        pimpl_->doc_.save_file(pimpl_->filename_.string().c_str());
    }
}

std::string get_attribute( const std::string &key, const pugi::xml_node &node )
{
    std::string rs = node.attribute(key.c_str()).value(); 
    if(!rs.empty())
        return rs;

    std::stringstream ss;
    ss << "error: required attribute " << key << " not present! ";
    throw configuration_exception(ss.str());
}

void parser::add(const std::string &address, const std::string port, std::size_t type )
{
    pugi::xml_node node = pimpl_->node_.append_child("dev");
    node.append_attribute("ip") = address.c_str();
    node.append_attribute("port") = port.c_str();
    node.append_attribute("type") = boost::lexical_cast<std::string>(type).c_str();
    pimpl_->doc_.save_file(pimpl_->filename_.string().c_str());
}

void parser::del( const std::string &address, const std::string port )
{
    auto node = pimpl_->node_.find_child([&]( const pugi::xml_node &nd){
            return (std::string(nd.attribute("ip").value()) == address) 
                and (std::string(nd.attribute("port").value()) == port);
            });

    if(!node)
        return;

    pimpl_->node_.remove_child(node);
    pimpl_->doc_.save_file(pimpl_->filename_.string().c_str());
}

std::size_t parser::each_device( device_iterator_f itr )
{
    try
    {
        std::for_each(
                pimpl_->node_.begin(), pimpl_->node_.end(), [&]( const pugi::xml_node &child){
                itr(
                    get_attribute("ip", child), 
                    get_attribute("port", child),
                    boost::lexical_cast<std::size_t>(get_attribute("type", child)) );
                });
    }

    catch( boost::bad_lexical_cast &bcs )
    {
        std::string errstr = 
            std::string("error in each_device: illegal value in list"); 
        throw configuration_exception(errstr);
    }
}

}
