#ifndef TR_MODE
#define TR_MODE

#include <string>

namespace TrRouting
{
  
  struct Mode {
  
  public:
   
    std::string shortname;
    std::string name;
    int gtfsId;
    int extendedGtfsId;

    const std::string toString() {
      return "Mode\n  shortname " + shortname + "\n  name " + name + "\n  gtfsId " + std::to_string(gtfsId) + "\n  extendedGtfsId " + std::to_string(extendedGtfsId);
    }

  };

}

#endif // TR_MODE
