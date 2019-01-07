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

  };

}

#endif // TR_SERVICE
