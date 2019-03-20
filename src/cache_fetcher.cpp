#include "cache_fetcher.hpp"

namespace TrRouting
{
  
  template<class T>
  void CacheFetcher::saveToCapnpCacheFile(T& data, std::string cacheFilePath, Parameters& params) {
    std::ofstream oCacheFile;
    oCacheFile.open(CacheFetcher::getFilePath(cacheFilePath, params), std::ios::out | std::ios::trunc | std::ios::binary);
    oCacheFile.close();
    int fd = open(CacheFetcher::getFilePath(cacheFilePath, params).c_str(), O_WRONLY);
    ::capnp::writePackedMessageToFd(fd, data);
    close(fd);
  }

  bool CacheFetcher::capnpCacheFileExists(std::string cacheFilePath, Parameters& params) {
    std::ifstream iCacheFile;
    bool notEmpty = false;
    iCacheFile.open(CacheFetcher::getFilePath(cacheFilePath, params));
    notEmpty = iCacheFile.is_open();
    //std::cout << "capnpCacheFileExists: " << CacheFetcher::getFilePath(cacheFilePath, params) << " : " << (notEmpty ? "TRUE" : "FALSE") << std::endl;
    iCacheFile.close();
    return notEmpty;
  }

  int CacheFetcher::getCacheFilesCount(std::string cacheFilePath, Parameters& params) {
    std::ifstream iCacheFile;
    iCacheFile.open(CacheFetcher::getFilePath(cacheFilePath, params));
    std::string strCount;
    int count {1};
    if (iCacheFile.is_open())
    {
      iCacheFile >> count;
      std::cout << cacheFilePath << " has " << count << " cache files" << std::endl;
    }
    iCacheFile.close();
    return count;
  }

  std::string CacheFetcher::getFilePath(std::string cacheFilePath, Parameters& params) {
    return params.cacheDirectoryPath + params.projectShortname + "/" + cacheFilePath;
  }

}
