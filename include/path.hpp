#ifndef TR_PATH
#define TR_PATH

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  
  struct Path {
  
  public:
   
    boost::uuids::uuid uuid;
    int lineIdx;
    std::string direction;
    std::vector<int> nodesIdx;

    const std::string toString() {
      return "Path " + boost::uuids::to_string(uuid) + "\n  direction " + direction;
    }

  };

}

#endif // TR_PATH
