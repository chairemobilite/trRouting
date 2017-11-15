#ifndef TR_TOOLBOX
#define TR_TOOLBOX

#include <vector>

namespace TrRouting
{
  
  class Toolbox {
  
  public:
   
    static const std::string convertSecondsToFormattedTime(int timeInSeconds)
    {
      std::string formattedTime {""};
      int hour   {timeInSeconds / 3600};
      int minute {(int)ceil((float)timeInSeconds / 60.0 - (float)(hour * 60))};
      formattedTime += (hour <= 9 ? "0" : "") + std::to_string(hour) + ":" + (minute <= 9 ? "0" : "") + std::to_string(minute);
      return formattedTime;
    }
  
  
  };

}

#endif // TR_TOOLBOX
