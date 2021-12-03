#include "cache_fetcher.hpp"

namespace TrRouting
{
  
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
    //std::cout << "capnpCacheFileExists: " << CacheFetcher::getFilePath(cacheFilePath, params, customPath) << " : " << (notEmpty ? "TRUE" : "FALSE") << std::endl;
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
      std::cout << cacheFilePath << " has " << count << " cache files" << std::endl;
    }
    iCacheFile.close();
    return count;
  }
  
  std::string CacheFetcher::getFilePath(std::string cacheFilePath, ServerParameters& params, std::string customPath) {
    std::string filePath = "";
    if (!params.cacheDirectoryPath.empty()) {
      filePath += params.cacheDirectoryPath + "/";
    }
    if (!params.projectShortname.empty()) {
      filePath += params.projectShortname + "/";
    }
    if (customPath.empty())
    {
      return filePath + cacheFilePath;
    }
    else
    {
      return filePath + customPath + "/" + cacheFilePath;
    }
  }

}
