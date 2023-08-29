#ifndef TR_PROGRAM_OPTIONS
#define TR_PROGRAM_OPTIONS

#include <string>

#include <boost/program_options.hpp>

namespace TrRouting
{
    
  class ProgramOptions {
  
  public:
    

    std::string cachePath;
    int         port;
    bool        debug;
    bool        cacheAllConnectionSets;
    std::string algorithm;
    std::string dataFetcherShortname;
    std::string osrmWalkingPort;
    std::string osrmCyclingPort;
    std::string osrmDrivingPort;
    std::string osrmWalkingHost;
    std::string osrmCyclingHost;
    std::string osrmDrivingHost;

    ProgramOptions();
    void parseOptions(int argc, char** argv);

  private:

    boost::program_options::options_description options;

  };

}

#endif // TR_PROGRAM_OPTIONS
