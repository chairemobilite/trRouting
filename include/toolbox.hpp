#ifndef TR_TOOLBOX
#define TR_TOOLBOX

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <math.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace TrRouting
{
    
  constexpr int MAX_INT {std::numeric_limits<int>::max()};
  
  class Toolbox {
  
  public:
    
    static const std::string convertSecondsToFormattedTime(int timeInSeconds)
    {
      std::string formattedTime {""};
      int hour   {timeInSeconds / 3600};
      int minute {(int)round((float)timeInSeconds / 60.0 - (float)(hour * 60))};
      if (minute == 60)
      {
        hour++;
        minute = 0;
      }
      formattedTime += (hour <= 9 ? "0" : "") + std::to_string(hour) + ":" + (minute <= 9 ? "0" : "") + std::to_string(minute);
      return formattedTime;
    }
    
    static const int convertSecondsToMinutes(int timeInSeconds)
    {
      return (int)round((float)timeInSeconds / 60.0);
    }

    static const std::vector<std::string> uuidsToStrings(std::vector<boost::uuids::uuid> uuids)
    {
      std::vector<std::string> strings;
      for (auto & uuid : uuids)
      {
        strings.push_back(boost::uuids::to_string(uuid));
      }
      return strings;
    }

    template <typename T>
    static std::string prettyPrintStructVector(std::vector<T> vect)
    {
      std::string str;
      int i {0};
      for (auto & obj : vect)
      {
        str += std::to_string(i) + ". " + obj.toString() + "\n";
        i++;
      }
      return str;
    }

  
  };

}

#endif // TR_TOOLBOX
