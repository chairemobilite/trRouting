
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
#include "spdlog/spdlog.h"

namespace TrRouting
{

  int CacheFetcher::getLines(
    std::map<boost::uuids::uuid, Line>& ts,
    const std::map<boost::uuids::uuid, Agency>& agencies,
    const std::map<std::string, Mode>& modes, 
    std::string customPath
  )
  {

    using T           = Line;
    using TCollection = lineCollection::LineCollection;
    using cT          = lineCollection::Line;
    int ret = 0;

    ts.clear();

    std::string tStr  = "lines";
    std::string TStr  = "Lines";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    spdlog::info("Fetching {} from cache... {}", tStr, customPath);

    std::string cacheFilePath = getFilePath(cacheFileName, customPath) + ".capnpbin";
      
    int fd = open(cacheFilePath.c_str(), O_RDWR);
    if (fd < 0)
    {
      int err = errno;
      if (err == ENOENT)
      {
        spdlog::error("missing {} cache files!", tStr);
      }
      else
      {
        spdlog::error("Error opening cache file {} : {} ", tStr, err);
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
        
        ts.emplace(uuidGenerator(uuid), T(uuidGenerator(uuid),
                                          agencies.at(uuidGenerator(agencyUuid)),
                                          modes.at(capnpT.getMode()),
                                          capnpT.getShortname(),
                                          capnpT.getLongname(),
                                          capnpT.getInternalId(),
                                          capnpT.getAllowSameLineTransfers()));
      }
    }
    catch (const kj::Exception& e)
    {
      spdlog::error("Error opening cache file {}: {}", tStr, e.getDescription().cStr());
      ret = -EBADMSG;
    }
    catch (const std::exception& e)
    {
      spdlog::error("Unknown error occurred {} {}", tStr, e.what());
      ret = -EINVAL;
    }
    catch (...)
    {
      spdlog::error("Unknown error occurred {} ", tStr);
      ret = -EINVAL;
    }

    close(fd);
    return ret;
  }

}

#endif // TR_LINES_CACHE_FETCHER
