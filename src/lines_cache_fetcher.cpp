
#ifndef TR_LINES_CACHE_FETCHER
#define TR_LINES_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "line.hpp"
#include "capnp/lineCollection.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  const std::pair<std::vector<Line>, std::map<boost::uuids::uuid, int>> CacheFetcher::getLines(std::map<boost::uuids::uuid, int> agencyIndexesByUuid, std::map<std::string, int> modeIndexesByShortname, Parameters& params, std::string customPath)
  { 

    using T           = Line;
    using TCollection = lineCollection::LineCollection;
    using cT          = lineCollection::Line;

    std::string tStr  = "lines";
    std::string TStr  = "Lines";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params, customPath))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getLines())
      {
        std::string uuid       {capnpT.getUuid()};
        std::string agencyUuid {capnpT.getAgencyUuid()};
        T * t                     = new T();
        t->uuid                   = uuidGenerator(uuid);
        t->shortname              = capnpT.getShortname();
        t->longname               = capnpT.getLongname();
        t->agencyIdx              = agencyIndexesByUuid[uuidGenerator(agencyUuid)];
        t->modeIdx                = modeIndexesByShortname[capnpT.getMode()];
        t->allowSameLineTransfers = capnpT.getAllowSameLineTransfers();
        ts.push_back(*t);
        tIndexesByUuid[t->uuid] = ts.size() - 1;
      }
      //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
      close(fd);
    }
    else
    {
      std::cerr << "missing " << tStr << " cache file!" << std::endl;
    }
    return std::make_pair(ts, tIndexesByUuid);
  }

}

#endif // TR_LINES_CACHE_FETCHER