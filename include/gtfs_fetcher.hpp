#ifndef TR_GTFS_FETCHER
#define TR_GTFS_FETCHER

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <math.h>
#include <boost/algorithm/string.hpp>
//#include <cereal/archives/binary.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <stdlib.h>

#include "calculation_time.hpp"
#include "agency.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"


namespace TrRouting
{
  
  class GtfsFetcher
  {
  
  public:
    
    GtfsFetcher() {}
    GtfsFetcher(std::string gtfsDirectoryPath) {
      
    }
    
    //const std::pair<std::vector<Stop> , std::map<unsigned long long, int>> getStops( std::string projectShortname);
    //const std::pair<std::vector<Route>, std::map<unsigned long long, int>> getRoutes(std::string projectShortname);
    //const std::pair<std::vector<Trip> , std::map<unsigned long long, int>> getTrips( std::string projectShortname);
    //const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short,int>>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>> getConnections(std::string projectShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById);
    //const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<long long,long long>>> getFootpaths(std::string projectShortname, std::map<unsigned long long, int> stopIndexesById);
    //const std::vector<std::pair<int,int>> getOdTripFootpaths(std::string projectShortname, ServerParameters& params);
    //const std::pair<std::vector<OdTrip>, std::map<unsigned long long, int>> getOdTrips(std::string projectShortname, std::vector<Stop> stops, ServerParameters& params);

    
  };
  
}

#endif // TR_GTFS_FETCHER
