#ifndef TR_TRIP_SEGMENT
#define TR_TRIP_SEGMENT

namespace TrRouting
{
  
  struct TripSegment {
    
    int rpI;          // route path index
    int tripI;        // trip index
    int firstStopSeq; // first stop sequence
    int lastStopSeq;  // last stop sequence
    std::vector<std::shared_ptr<TripSegment>> prevTripSegments; // journey previous trip segments (to recreate journey after calculation)

    TripSegment(){}

    // constructor without journey previous trip segments and without new journey trip segment:
    TripSegment(int _rpI, int _tripI, int _firstStopSeq, int _lastStopSeq) : rpI(_rpI), tripI(_tripI), firstStopSeq(_firstStopSeq), lastStopSeq(_lastStopSeq) {}
    
    // constructor including journey previous trip segments and new journey trip segment as a shared pointer to TripSegment:
    TripSegment(int _rpI, int _tripI, int _firstStopSeq, int _lastStopSeq, std::vector<std::shared_ptr<TripSegment>> _prevTripSegments, std::shared_ptr<TripSegment> _newTripSegment)
    : rpI(_rpI), tripI(_tripI), firstStopSeq(_firstStopSeq), lastStopSeq(_lastStopSeq), prevTripSegments(_prevTripSegments)
    {
      prevTripSegments.push_back(_newTripSegment);
    }
    
    // constructor including journey previous trip segments and new journey trip segment as a TripSegment object:
    TripSegment(int _rpI, int _tripI, int _firstStopSeq, int _lastStopSeq, std::vector<std::shared_ptr<TripSegment>> _prevTripSegments, TripSegment _newTripSegment)
    : rpI(_rpI), tripI(_tripI), firstStopSeq(_firstStopSeq), lastStopSeq(_lastStopSeq), prevTripSegments(_prevTripSegments)
    {
      prevTripSegments.push_back(std::make_shared<TripSegment>(_newTripSegment));
    }

  };

}

#endif // TR_TRIP_SEGMENT
