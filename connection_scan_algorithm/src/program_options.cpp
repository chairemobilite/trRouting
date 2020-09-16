#include "program_options.hpp"

namespace TrRouting {

  ProgramOptions::ProgramOptions() : options(boost::program_options::options_description("Options")) {

    options.add_options()
      ("port",                                              boost::program_options::value<int>()        ->default_value(4000), "http server port");
    options.add_options()
      ("debug",                                             boost::program_options::value<int>()        ->default_value(0), "debug");
    options.add_options()
      ("dataFetcher,data",                                  boost::program_options::value<std::string>()->default_value("cache"), "data fetcher (csv, gtfs or cache)"); // only cache implemented for now
    options.add_options()
      ("cachePath",                                         boost::program_options::value<std::string>()->default_value("cache"), "cache path");
    options.add_options()
      ("project,projectShortname",                          boost::program_options::value<std::string>()->default_value("demo_transition"), "project shortname (shortname of the project to use or data to use)");
    options.add_options()
      ("osrmPort,osrmWalkPort,osrmWalkingPort",             boost::program_options::value<std::string>()->default_value("5000"), "osrm walking port");
    options.add_options()
      ("osrmCyclingPort",                                   boost::program_options::value<std::string>()->default_value("8000"), "osrm cycling port");
    options.add_options()
      ("osrmDrivingPort",                                   boost::program_options::value<std::string>()->default_value("7000"), "osrm driving port");
    options.add_options()
      ("osrmHost,osrmWalkHost,osrmWalkingHost",             boost::program_options::value<std::string>()->default_value("localhost"), "osrm walking host");
    options.add_options()
      ("osrmCyclingHost",                                   boost::program_options::value<std::string>()->default_value("localhost"), "osrm cycling host");
    options.add_options()
      ("osrmDrivingHost",                                   boost::program_options::value<std::string>()->default_value("localhost"), "osrm driving host");
    options.add_options()
      ("osrmFilePath,osrmWalkFilePath,osrmWalkingFilePath", boost::program_options::value<std::string>()->default_value(""), "osrm file path (walking) (PATH/TO/ROUTING_FILE.osrm)");
    options.add_options()
      ("osrmCyclingFilePath",                               boost::program_options::value<std::string>()->default_value(""), "osrm file path (cycling) (PATH/TO/ROUTING_FILE.osrm)");
    options.add_options()
      ("osrmDrivingFilePath",                               boost::program_options::value<std::string>()->default_value(""), "osrm file path (driving) (PATH/TO/ROUTING_FILE.osrm)");
    options.add_options()
      ("osrmUseLib,osrmWalkUseLib,osrmWalkingUseLib",       boost::program_options::value<int>()        ->default_value(0), "osrm use libosrm (walking) instead of server (1 or 0)");
    options.add_options()
      ("osrmCyclingUseLib",                                 boost::program_options::value<int>()        ->default_value(0), "osrm use libosrm (cycling) instead of server (1 or 0)");
    options.add_options()
      ("osrmDrivingUseLib",                                 boost::program_options::value<int>()        ->default_value(0), "osrm use libosrm (driving) instead of server (1 or 0)");



  }

  void ProgramOptions::parseOptions(int argc, char** argv) {

    boost::program_options::variables_map variablesMap;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), variablesMap);

    port                 = 4000;
    debug                = false;
    dataFetcherShortname = "cache";
    cachePath            = "cache";
    projectShortname     = "demo_transition";
    osrmWalkingPort      = "5000";
    osrmCyclingPort      = "8000";
    osrmDrivingPort      = "7000";
    osrmWalkingHost      = "localhost";
    osrmCyclingHost      = "localhost";
    osrmDrivingHost      = "localhost";
    osrmWalkingFilePath  = "";
    osrmCyclingFilePath  = "";
    osrmDrivingFilePath  = "";
    osrmWalkingUseLib    = false;
    osrmCyclingUseLib    = false;
    osrmDrivingUseLib    = false;

    if(variablesMap.count("port") == 1)
    {
      port = variablesMap["port"].as<int>();
    }
    if(variablesMap.count("debug") == 1)
    {
      debug = (variablesMap["debug"].as<int>() == 1) ? true : false;
    }
    if(variablesMap.count("dataFetcher") == 1)
    {
      dataFetcherShortname = variablesMap["dataFetcher"].as<std::string>();
    }
    else if (variablesMap.count("data") == 1)
    {
      dataFetcherShortname = variablesMap["data"].as<std::string>();
    }
    if(variablesMap.count("projectShortname") == 1)
    {
      projectShortname = variablesMap["projectShortname"].as<std::string>();
    }
    else if(variablesMap.count("project") == 1)
    {
      projectShortname = variablesMap["project"].as<std::string>();
    }
    if(variablesMap.count("cachePath") == 1)
    {
      cachePath = variablesMap["cachePath"].as<std::string>();
    }

    if(variablesMap.count("osrmWalkPort") == 1)
    {
      osrmWalkingPort = variablesMap["osrmWalkPort"].as<std::string>();
    }
    else if(variablesMap.count("osrmWalkingPort") == 1)
    {
      osrmWalkingPort = variablesMap["osrmWalkingPort"].as<std::string>();
    }
    else if(variablesMap.count("osrmPort") == 1)
    {
      osrmWalkingPort = variablesMap["osrmPort"].as<std::string>();
    }
    if(variablesMap.count("osrmCyclingPort") == 1)
    {
      osrmCyclingPort = variablesMap["osrmCyclingPort"].as<std::string>();
    }
    if(variablesMap.count("osrmDrivingPort") == 1)
    {
      osrmDrivingPort = variablesMap["osrmDrivingPort"].as<std::string>();
    }

    if(variablesMap.count("osrmWalkHost") == 1)
    {
      osrmWalkingHost = variablesMap["osrmWalkHost"].as<std::string>();
    }
    else if(variablesMap.count("osrmWalkingHost") == 1)
    {
      osrmWalkingHost = variablesMap["osrmWalkingHost"].as<std::string>();
    }
    else if(variablesMap.count("osrmHost") == 1)
    {
      osrmWalkingHost = variablesMap["osrmHost"].as<std::string>();
    }
    if(variablesMap.count("osrmCyclingHost") == 1)
    {
      osrmCyclingHost = variablesMap["osrmCyclingHost"].as<std::string>();
    }
    if(variablesMap.count("osrmDrivingHost") == 1)
    {
      osrmDrivingHost = variablesMap["osrmDrivingHost"].as<std::string>();
    }

    if(variablesMap.count("osrmFilePath") == 1)
    {
      osrmWalkingFilePath = variablesMap["osrmFilePath"].as<std::string>();
    }
    else if(variablesMap.count("osrmWalkingFilePath") == 1)
    {
      osrmWalkingFilePath = variablesMap["osrmWalkingFilePath"].as<std::string>();
    }
    else if(variablesMap.count("osrmWalkFilePath") == 1)
    {
      osrmWalkingFilePath = variablesMap["osrmWalkFilePath"].as<std::string>();
    }
    if(variablesMap.count("osrmCyclingFilePath") == 1)
    {
      osrmCyclingFilePath = variablesMap["osrmCyclingFilePath"].as<std::string>();
    }
    if(variablesMap.count("osrmDrivingFilePath") == 1)
    {
      osrmDrivingFilePath = variablesMap["osrmDrivingFilePath"].as<std::string>();
    }
    
    if(variablesMap.count("osrmUseLib") == 1)
    {
      osrmWalkingUseLib = (variablesMap["osrmUseLib"].as<int>() == 1) ? true : false;
    }
    else if(variablesMap.count("osrmWalkUseLib") == 1)
    {
      osrmWalkingUseLib = (variablesMap["osrmWalkUseLib"].as<int>() == 1) ? true : false;
    }
    else if(variablesMap.count("osrmWalkingUseLib") == 1)
    {
      osrmWalkingUseLib = (variablesMap["osrmWalkingUseLib"].as<int>() == 1) ? true : false;
    }
    if(variablesMap.count("osrmCyclingUseLib") == 1)
    {
      osrmCyclingUseLib = (variablesMap["osrmCyclingUseLib"].as<int>() == 1) ? true : false;
    }
    if(variablesMap.count("osrmDrivingUseLib") == 1)
    {
      osrmDrivingUseLib = (variablesMap["osrmDrivingUseLib"].as<int>() == 1) ? true : false;
    }

  }

}