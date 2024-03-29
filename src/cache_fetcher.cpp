#include <fstream>
#include <fcntl.h>
#include <capnp/serialize-packed.h>
#include "spdlog/spdlog.h"
#include "cache_fetcher.hpp"
#include "parameters.hpp"

namespace TrRouting
{
  CacheFetcher::CacheFetcher(const std::string &cacheDir)
    : cacheDirectoryPath(cacheDir) {
  }
  CacheFetcher::~CacheFetcher() {}

  template<class T>
  void CacheFetcher::saveToCapnpCacheFile(T& data, std::string cacheFilePath) {
    std::ofstream oCacheFile;
    oCacheFile.open(cacheFilePath, std::ios::out | std::ios::trunc | std::ios::binary);
    oCacheFile.close();
    int fd = open(cacheFilePath.c_str(), O_WRONLY);
    ::capnp::writePackedMessageToFd(fd, data);
    close(fd);
  }

  bool CacheFetcher::capnpCacheFileExists(std::string cacheFilePath) {
    std::ifstream iCacheFile;
    bool notEmpty = false;
    iCacheFile.open(cacheFilePath);
    notEmpty = iCacheFile.is_open();

    iCacheFile.close();
    return notEmpty;
  }

  int CacheFetcher::getCacheFilesCount(std::string cacheFilePath) {
    std::ifstream iCacheFile;
    iCacheFile.open(cacheFilePath);
    std::string strCount;
    int count {1};
    if (iCacheFile.is_open())
    {
      iCacheFile >> count;
      spdlog::info("{} has {} cache files", cacheFilePath, count);
    }
    iCacheFile.close();
    return count;
  }
  
  std::string CacheFetcher::getFilePath(std::string cacheFilePath, std::string customPath) {
    std::string filePath = "";
    if (!cacheDirectoryPath.empty()) {
      filePath += cacheDirectoryPath + "/";
    }
    if (customPath.empty())
    {
      filePath += cacheFilePath;
    }
    else
    {
      filePath += customPath + "/" + cacheFilePath;
    }
    spdlog::debug("reading {} cache file", filePath);
    return filePath;
  }

}
