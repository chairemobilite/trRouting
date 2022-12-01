#ifndef TR_MODE
#define TR_MODE

#include <string>

namespace TrRouting
{
  
  class Mode {
  
  public:
    inline static const std::string TRANSFERABLE {"transferable"}; 

    Mode(const std::string &ashortname,
         const std::string &aname,
         int agtfsId,
         int aextendedGtfsId) : shortname(ashortname),
                                name(aname),
                                gtfsId(agtfsId),
                                extendedGtfsId(aextendedGtfsId) {}

    std::string shortname;
    std::string name;
    int gtfsId;
    int extendedGtfsId;

    const std::string toString() {
      return "Mode\n  shortname " + shortname + "\n  name " + name + "\n  gtfsId " + std::to_string(gtfsId) + "\n  extendedGtfsId " + std::to_string(extendedGtfsId);
    }

    //TODO Maybe this function should be part of the Line object?
    bool isTransferable() const {
      return TRANSFERABLE == shortname;
    }

    // Equal operator. We only compare the shortname, since they should be unique.
    inline bool operator==(const Mode& other ) const { return shortname == other.shortname; }
  };

  // To use std::find with a vector<reference_wrapper<const Mode>>
  inline bool operator==(const std::reference_wrapper<const TrRouting::Mode>& lhs, const Mode& rhs)
  {
    return lhs.get().shortname == rhs.shortname;
  }
}

#endif // TR_MODE
