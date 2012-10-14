#ifndef CONFIG_PARSER__
#define CONFIG_PARSER__

#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <string>

namespace configuration
{

    typedef boost::function<
        void( 
                const std::string &address, 
                const std::string &port, 
                std::size_t type )
        > device_iterator_f;

    class configuration_exception
    {
        std::string what_;
        public:
            configuration_exception( const std::string &what);
            std::string what() const;
    };

    //this object holds the configuration data and allows
    //traversion over all items as well as manipulation of
    //the config data.
    class parser
    {
        class impl;
        boost::shared_ptr<impl> pimpl_;
        public:
        
        //CTOR: needs a a 
        parser( const boost::filesystem::path &location );


        //iterate over each device. Takes a call back function as argument
        std::size_t each_device( device_iterator_f itr ); 
        
        void add( const std::string &address, const std::string port, std::size_t type ); //add a new device
        void del( const std::string &address, const std::string port ); //delete a device
    };

}

#endif
