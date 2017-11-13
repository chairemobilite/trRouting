#include "database_fetcher.hpp"

namespace TrRouting
{

  class DatabaseFetcher {
    
    public:
      
      static DatabaseFetcher& getInstance(std::string customDbSetupStr = "")
      {
        static DatabaseFetcher instance(customDbSetupStr);
        return instance;
      }
      
      void DbFetcher::disconnect()
      {
        if (pgConnectionPtr)
        {
          pgConnectionPtr->disconnect();
          delete pgConnectionPtr;
          pgConnectionPtr = NULL;
        }
      }
      
      bool DbFetcher::isConnectionOpen()
      {
        return (*getConnectionPtr()).is_open();
      }
      
    private:
      
      DatabaseFetcher() {}
      DatabaseFetcher(std::string customDbSetupStr) {
        dbSetupStr = customDbSetupStr;
        if (pgConnectionPtr != NULL)
        {
          return pgConnectionPtr;
        }
        else
        {
          pgConnectionPtr = new pqxx::connection(dbSetupStr);
          return pgConnectionPtr;
        }
      }
      DatabaseFetcher(DatabaseFetcher const&);
      void operator=(DatabaseFetcher const&);
      
      pqxx::connection* DbFetcher::pgConnectionPtr = NULL;
      std::string DbFetcher::dbSetupStr = "";
      
    public:
      
      DatabaseFetcher(DatabaseFetcher const&) = delete;
      void operator=(DatabaseFetcher const&)  = delete;
    
  }
  

}
