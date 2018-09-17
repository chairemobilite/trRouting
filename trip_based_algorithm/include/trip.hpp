#ifndef TR_TRIP
#define TR_TRIP

namespace TrRouting
{
  
  struct Trip {
    
    int i;         // trip index
    int seq;       // trip sequence in route path
    int rpI;       // route path index
    long long id;  // trip id
    
  };

}

#endif // TR_TRIP
