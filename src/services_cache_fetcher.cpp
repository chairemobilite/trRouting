
#ifndef TR_SERVICES_CACHE_FETCHER
#define TR_SERVICES_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <kj/exception.h>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "service.hpp"
#include "capnp/serviceCollection.capnp.h"
#include "spdlog/spdlog.h"

namespace TrRouting
{

  int CacheFetcher::getServices(
    std::map<boost::uuids::uuid, Service>& ts,
    std::string customPath
  )
  {

    using T           = Service;
    using TCollection = serviceCollection::ServiceCollection;
    using cT          = serviceCollection::Service;
    int ret = 0;

    ts.clear();

    std::string tStr  = "services";
    std::string TStr  = "Services";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;
    boost::uuids::nil_generator    uuidNilGenerator;

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
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getServices())
      {
        std::string uuid           {capnpT.getUuid()};
        std::string simulationUuid {capnpT.getSimulationUuid()};

        std::vector<boost::gregorian::date> onlyDates;
        std::vector<boost::gregorian::date> exceptDates;

        T t;

        t.uuid           = uuidGenerator(uuid);
        t.name           = capnpT.getName();
        t.internalId     = capnpT.getInternalId();
        t.simulationUuid = simulationUuid.empty() ? uuidNilGenerator() : uuidGenerator(simulationUuid);
        t.monday         = capnpT.getMonday();
        t.tuesday        = capnpT.getTuesday();
        t.wednesday      = capnpT.getWednesday();
        t.thursday       = capnpT.getThursday();
        t.friday         = capnpT.getFriday();
        t.saturday       = capnpT.getSaturday();
        t.sunday         = capnpT.getSunday();
        std::string startDate {capnpT.getStartDate()};
        if (startDate.length() > 0)
        {
          t.startDate = boost::gregorian::from_string(startDate);
        }
        std::string endDate {capnpT.getEndDate()};
        if (endDate.length() > 0)
        {
          t.endDate = boost::gregorian::from_string(endDate);
        }
        for (const auto & onlyDate : capnpT.getOnlyDates())
        {
          onlyDates.push_back(boost::gregorian::from_string(onlyDate));
        }
        for (const auto & exceptDate : capnpT.getExceptDates())
        {
          exceptDates.push_back(boost::gregorian::from_string(exceptDate));
        }
        t.onlyDates   = onlyDates;
        t.exceptDates = exceptDates;
        
        ts[t.uuid] = t;
      }
    }
    catch (const kj::Exception& e)
    {
      spdlog::error("Error opening cache file {}: {}", tStr, e.getDescription().cStr());
      ret = -EBADMSG;
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

#endif // TR_SERVICES_CACHE_FETCHER
