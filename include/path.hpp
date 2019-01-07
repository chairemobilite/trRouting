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

  };

}

#endif // TR_PATH
