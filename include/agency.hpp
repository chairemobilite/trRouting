#ifndef TR_AGENCY
#define TR_AGENCY

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace TrRouting
{
  
  class Agency {
    
  public:

    boost::uuids::uuid uuid;
    std::string acronym;
    std::string name;
    std::string internalId;
    boost::uuids::uuid simulationUuid;

    const std::string toString() {
      return "Agency " + boost::uuids::to_string(uuid) + "\n  acronym " + acronym + "\n  name " + name;
    }

    // Equal operator. We only compare the uuid, since they should be unique.
    inline bool operator==(const Agency& other ) const { return uuid == other.uuid; }
  };

  // To use std::find with a vector<reference_wrapper<const Agency>>
  inline bool operator==(const std::reference_wrapper<const TrRouting::Agency>& lhs, const Agency& rhs)
  {
    return lhs.get() == rhs;
  }
}

#endif // TR_AGENCY
