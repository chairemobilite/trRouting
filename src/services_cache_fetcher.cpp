
#ifndef TR_SERVICES_CACHE_FETCHER
#define TR_SERVICES_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "service.hpp"
#include "capnp/servicesCollection.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  const std::pair<std::vector<Service>, std::map<boost::uuids::uuid, int>> CacheFetcher::getServices(Parameters& params)
  { 

    using T           = Service;
    using TCollection = servicesCollection::ServicesCollection;
    using cT          = servicesCollection::Service;

    std::string tStr  = "services";
    std::string TStr  = "Services";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(cacheFileName, params))
    {
      int fd = open((params.cacheDirectoryPath + params.projectShortname + "/" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getServices())
      {
        std::string uuid       {capnpT.getUuid()};

        std::vector<boost::gregorian::date> onlyDates;
        std::vector<boost::gregorian::date> exceptDates;

        T * t        = new T();
        t->uuid      = uuidGenerator(uuid);
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

#endif // TR_SERVICES_CACHE_FETCHER