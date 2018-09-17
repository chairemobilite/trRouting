#ifndef TR_TRANSFER
#define TR_TRANSFER

namespace TrRouting
{
  
  struct Transfer {
  
  public:
    
    int srcStopSeq; // source stop index
    int srcTripI;   // source trip index
    int tgtStopSeq; // target stop index
    int tgtTripI;   // target trip index
  
  };

}

#endif // TR_TRANSFER
