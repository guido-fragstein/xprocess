
#include "validation.hpp"

#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>

namespace validation
{
    struct is_numeric : public base
    {
        is_numeric( signature *sig ){}
        virtual bool validate( const std::string &input )
        {
            try
            {
                float rs = boost::lexical_cast<float>(input);
                return true;
            }
            
            catch( boost::bad_lexical_cast &blc )
            {
                return false;
            }
        }
    };

    struct is_integer : public base
    {
        is_integer( signature *sig ){}
        virtual bool validate( const std::string &input )
        {
            return true;
        }
    };

    struct is_date : public base
    {
        is_date( signature *sig ){}
        virtual bool validate( const std::string &input )
        {
            return true;
        }
    };

    struct is_time : public base
    {
        is_time( signature *sig ){}
        virtual bool validate( const std::string &input )
        {
            return true;
        }
    };

    struct is_datetime : public base
    {
        is_datetime( signature *sig ){}
        virtual bool validate( const std::string &input )
        {
            return true;
        }
    };


    struct is_duration : public base
    {
        is_duration( signature *sig ){} 
        virtual bool validate( const std::string &input )
        {
            return true;
        }
    };

    struct factory::impl
    {
        boost::unordered_map<std::string, boost::function< base*( signature *sig )> > pointer_map_;        
    };

    factory::factory()
        :pimpl_(new impl())
    {
        pimpl_->pointer_map_ = {
            {"is_numeric", boost::factory<is_numeric*>() },
            {"is_integer", boost::factory<is_integer*>() },
            {"is_date", boost::factory<is_date*>() },
            {"is_time", boost::factory<is_time*>() },
            {"is_datetime", boost::factory<is_datetime*>() },
            {"is_duration", boost::factory<is_duration*>() },
        };
    }

    pointer factory::get( signature *sig )
    {
        auto rs = pimpl_->pointer_map_.find(sig->name());
        if(rs == pimpl_->pointer_map_.end())
            return pointer();

        return pointer( rs->second(sig) );
    }
}
