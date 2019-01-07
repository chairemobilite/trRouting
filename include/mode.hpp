#ifndef TR_MODE
#define TR_MODE

#include <string>

namespace TrRouting
{
  
  struct Mode {
  
  public:
   
    std::string shortname;
    std::string name;
    int extendedGtfsId;
    int gtfsId;
 
  };

}

#endif // TR_MODE
