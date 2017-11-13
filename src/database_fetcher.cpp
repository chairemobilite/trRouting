#include "database_fetcher.hpp"

namespace TrRouting
{

    
  void DatabaseFetcher::disconnect()
  {
    if (pgConnectionPtr != NULL)
    {
      pgConnectionPtr->disconnect();
      delete pgConnectionPtr;
      pgConnectionPtr = NULL;
    }
  }
  
  bool DatabaseFetcher::isConnectionOpen()
  {
    return (*pgConnectionPtr).is_open();
  }
    
  

}
