
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

  void CacheFetcher::getLines(
    std::vector<std::unique_ptr<Line>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
    std::map<std::string, int>& modeIndexesByShortname,
    Parameters& params,
    std::string customPath
  ) {

    using T           = Line;
    using TCollection = lineCollection::LineCollection;
    using cT          = lineCollection::Line;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "lines";
    std::string TStr  = "Lines";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    std::string cacheFilePath = CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin";
    if (CacheFetcher::capnpCacheFileExists(cacheFilePath))
    {
      int fd = open(cacheFilePath.c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getLines())
      {
        std::string uuid       {capnpT.getUuid()};
        std::string agencyUuid {capnpT.getAgencyUuid()};
        
        std::unique_ptr<T> t = std::make_unique<T>();

        t->uuid                   = uuidGenerator(uuid);
        t->shortname              = capnpT.getShortname();
        t->longname               = capnpT.getLongname();
        t->internalId             = capnpT.getInternalId();
        t->agencyIdx              = agencyIndexesByUuid[uuidGenerator(agencyUuid)];
        t->modeIdx                = modeIndexesByShortname[capnpT.getMode()];
        t->allowSameLineTransfers = capnpT.getAllowSameLineTransfers();
        
        tIndexesByUuid[t->uuid] = ts.size();
        ts.push_back(std::move(t));
      }
      //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
      close(fd);
    }
    else
    {
      std::cerr << "missing " << tStr << " cache file!" << std::endl;
    }
  }

}

#endif // TR_LINES_CACHE_FETCHER