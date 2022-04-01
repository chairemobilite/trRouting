
#ifndef TR_LINES_CACHE_FETCHER
#define TR_LINES_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <kj/exception.h>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "line.hpp"
#include "capnp/lineCollection.capnp.h"

namespace TrRouting
{

  int CacheFetcher::getLines(
    std::vector<std::unique_ptr<Line>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
    std::map<std::string, int>& modeIndexesByShortname,
    Parameters& params,
    std::string customPath
  )
  {

    using T           = Line;
    using TCollection = lineCollection::LineCollection;
    using cT          = lineCollection::Line;
    int ret = 0;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "lines";
    std::string TStr  = "Lines";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    std::string cacheFilePath = CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin";
      
    int fd = open(cacheFilePath.c_str(), O_RDWR);
    if (fd < 0)
    {
      int err = errno;
      if (err == ENOENT)
      {
        std::cerr << "missing " << tStr << " cache file!" << std::endl;
      }
      else
      {
        std::cerr << "Error opening cache file " << tStr << ": " << err << std::endl;
      }
      return -err;
    }

    try
    {
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
    }
    catch (const kj::Exception& e)
    {
      std::cerr << "Error opening cache file " << tStr << ": " << e.getDescription().cStr() << std::endl;
      ret = -EBADMSG;
    }
    catch (...)
    {
      std::cerr << "Unknown error occurred " << tStr << std::endl;
      ret = -EINVAL;
    }

    close(fd);
    return ret;
  }

}

#endif // TR_LINES_CACHE_FETCHER
