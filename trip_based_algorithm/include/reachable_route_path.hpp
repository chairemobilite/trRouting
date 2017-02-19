#ifndef TR_REACHABLE_ROUTE_PATH
#define TR_REACHABLE_ROUTE_PATH

namespace TrRouting
{
  
  struct ReachableRoutePath {
    
    int rpI;     // route path index
    int stopSeq; // stop sequence
    int tt;      // footpath travel time (seconds)

    ReachableRoutePath() {}

    ReachableRoutePath(int _rpI, int _stopSeq, int _tt) : rpI(_rpI), stopSeq(_stopSeq), tt(_tt) {}

  };

}

#endif // TR_REACHABLE_ROUTE_PATH
