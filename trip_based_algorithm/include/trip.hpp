#ifndef TR_TRIP
#define TR_TRIP

namespace TrRouting
{
  
  struct Trip {
    
    int i; // trip index
    long long id; // trip id
    int seq; // trip sequence in route path
    int rpI; // route path index
    
  };

}

#endif // TR_TRIP
