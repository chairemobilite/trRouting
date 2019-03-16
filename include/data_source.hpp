#ifndef TR_DATA_SOURCE
#define TR_DATA_SOURCE

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  
  struct DataSource {
  
  public:
   
    boost::uuids::uuid uuid;
    std::string shortname;
    std::string name;
    std::string type;

    const std::string toString() {
      return "DataSource " + boost::uuids::to_string(uuid) + "\n  name " + name + " (" + type + ")";
    }

  };

}

#endif // TR_DATA_SOURCE
