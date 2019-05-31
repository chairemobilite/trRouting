
#ifndef TR_SERVICES_CACHE_FETCHER
#define TR_SERVICES_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "service.hpp"
#include "capnp/serviceCollection.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  void CacheFetcher::getServices(
    std::vector<std::unique_ptr<Service>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    Parameters& params,
    std::string customPath
  ) {  

    using T           = Service;
    using TCollection = serviceCollection::ServiceCollection;
    using cT          = serviceCollection::Service;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "services";
    std::string TStr  = "Services";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params, customPath))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getServices())
      {
        std::string uuid       {capnpT.getUuid()};

        std::vector<boost::gregorian::date> onlyDates;
        std::vector<boost::gregorian::date> exceptDates;

        std::unique_ptr<T> t = std::make_unique<T>();

        t->uuid      = uuidGenerator(uuid);
        t->name      = capnpT.getName();
        t->monday    = capnpT.getMonday();
        t->tuesday   = capnpT.getTuesday();
        t->wednesday = capnpT.getWednesday();
        t->thursday  = capnpT.getThursday();
        t->friday    = capnpT.getFriday();
        t->saturday  = capnpT.getSaturday();
        t->sunday    = capnpT.getSunday();
        std::string startDate {capnpT.getStartDate()};
        if (startDate.length() > 0)
        {
          t->startDate = boost::gregorian::from_string(startDate);
        }
        std::string endDate {capnpT.getEndDate()};
        if (endDate.length() > 0)
        {
          t->endDate = boost::gregorian::from_string(endDate);
        }
        for (const auto & onlyDate : capnpT.getOnlyDates())
        {
          onlyDates.push_back(boost::gregorian::from_string(onlyDate));
        }
        for (const auto & exceptDate : capnpT.getExceptDates())
        {
          exceptDates.push_back(boost::gregorian::from_string(exceptDate));
        }
        t->onlyDates   = onlyDates;
        t->exceptDates = exceptDates;
        
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

#endif // TR_SERVICES_CACHE_FETCHER