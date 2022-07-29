#ifndef TR_SERVICE
#define TR_SERVICE

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace TrRouting
{
  
  struct Service {
  
  public:
   
    boost::uuids::uuid uuid;
    std::string name;
    std::string internalId;
    boost::uuids::uuid simulationUuid;
    short monday;
    short tuesday;
    short wednesday;
    short thursday;
    short friday;
    short saturday;
    short sunday;
    std::vector<boost::gregorian::date> onlyDates;
    std::vector<boost::gregorian::date> exceptDates;
    boost::gregorian::date startDate;
    boost::gregorian::date endDate;

    const std::string toString() {
      return "Service " + boost::uuids::to_string(uuid) + "\n  name " + name;
    }

    // Equal operator. We only compare the uuid, since they should be unique.
    inline bool operator==(const Service& other ) const { return uuid == other.uuid; }
  };

  // To use std::find with a vector<reference_wrapper<const Service>>
  // TODO Could this be a template?
  inline bool operator==(const std::reference_wrapper<const Service>& lhs, const Service& rhs)
  {
    return lhs.get() == rhs;
  }
}

#endif // TR_SERVICE
