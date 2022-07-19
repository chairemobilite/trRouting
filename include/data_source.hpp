#ifndef TR_DATA_SOURCE
#define TR_DATA_SOURCE

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace TrRouting
{
  
  class DataSource {
  
  public:
   
    boost::uuids::uuid uuid;
    std::string shortname;
    std::string name;
    std::string type;

    const std::string toString() {
      return "DataSource " + boost::uuids::to_string(uuid) + " name " + name + " type " + type;
    }

    // Equal operator. We only compare the uuid, since they should be unique.
    inline bool operator==(const DataSource& other ) const { return uuid == other.uuid; }
    inline bool operator!=(const DataSource& other ) const { return !operator==(other); }

  };

}

#endif // TR_DATA_SOURCE
