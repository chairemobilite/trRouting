#ifndef TR_PROGRAM_OPTIONS
#define TR_PROGRAM_OPTIONS

#include <string>
#include <vector>

#include <boost/program_options.hpp>

namespace TrRouting
{
    
  class ProgramOptions {
  
  public:
    

    std::string projectShortname;
    int         port;
    bool        debug;
    std::string algorithm;
    std::string dataFetcherShortname;
    std::string osrmWalkingPort;
    std::string osrmCyclingPort;
    std::string osrmDrivingPort;
    std::string osrmWalkingHost;
    std::string osrmCyclingHost;
    std::string osrmDrivingHost;
    std::string osrmWalkingFilePath;
    std::string osrmCyclingFilePath;
    std::string osrmDrivingFilePath;
    bool        osrmWalkingUseLib;
    bool        osrmCyclingUseLib;
    bool        osrmDrivingUseLib;

    ProgramOptions();
    void parseOptions(int argc, char** argv);

  private:

    boost::program_options::options_description options;

  };

}

#endif // TR_PROGRAM_OPTIONS
