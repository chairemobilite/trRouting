#ifndef TR_FOOTPATH
#define TR_FOOTPATH

namespace TrRouting
{
  
  struct Footpath {
    
    int i;    // footpath index
    int srcI; // source stop index
    int tgtI; // target stop index
    int tt;   // travel time seconds
  };

}

#endif // TR_FOOTPATH
