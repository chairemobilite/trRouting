#ifndef TR_STOP
#define TR_STOP

#include "point.hpp"

namespace TrRouting
{
  
  struct Stop {
  
  public:
    
    int i; // stopm index
    //int at; // arrival time (second of day) to reach stop (added after calculation)
    //int tt; // travel time in seconds to reach stop (added after calculation)
    long long id; // stop id
    std::string code; // stop code
    std::string name; // stop name
    long long stId; // station id
    Point point; // point lat lon
  
  };

}

#endif // TR_STOP
