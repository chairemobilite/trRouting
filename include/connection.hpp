#ifndef TR_CONNECTION
#define TR_CONNECTION

namespace TrRouting
{
  class Node;
  class Trip;
  
  class Connection {

  public:
    Connection(const Node & _departureNode,
               const Node & _arrivalNode,
               int _departureTimeSeconds,
               int _arrivalTimeSeconds,
               Trip & _trip,
               bool _canBoard,
               bool _canUnboard,
               int _sequenceInTrip,
               bool _canTransferSameLine,
               short _minWaitingTimeSeconds) :
      departureNode(_departureNode),
      arrivalNode(_arrivalNode),
      departureTimeSeconds(_departureTimeSeconds),
      arrivalTimeSeconds(_arrivalTimeSeconds),
      trip(_trip),
      canBoardValue(_canBoard),
      canUnboardValue(_canUnboard),
      sequenceInTrip(_sequenceInTrip),
      canTransferSameLineValue(_canTransferSameLine),
      minWaitingTimeSeconds(_minWaitingTimeSeconds) {}

    const Node & getDepartureNode() {return departureNode;}
    const Node & getArrivalNode() {return arrivalNode;}
    int getDepartureTime() {return departureTimeSeconds;}
    int getArrivalTime() {return arrivalTimeSeconds;}
    const Trip & getTrip() {return trip;}
    //TODO Revisit how we create Trip and Connection to not require a mutable trip here (issue #201
    Trip & getTripMutable() {return trip;}
    bool canBoard() {return canBoardValue;}
    bool canUnboard() {return canUnboardValue;}
    int getSequenceInTrip() {return sequenceInTrip;}
    bool canTransferSameLine() {return canTransferSameLineValue;}
    short getMinWaitingTime() {return minWaitingTimeSeconds;}
    short getMinWaitingTimeOrDefault(short defaultMinWaitingTime) {
      if (minWaitingTimeSeconds >= 0) {
        return minWaitingTimeSeconds;
      } else {
        return defaultMinWaitingTime;
      }
    }

  private:
    const Node & departureNode;
    const Node & arrivalNode;
    int departureTimeSeconds;
    int arrivalTimeSeconds;
    Trip & trip;
    bool canBoardValue;
    bool canUnboardValue;
    int sequenceInTrip;
    bool canTransferSameLineValue;
    short minWaitingTimeSeconds; // (-1 to inherit from parameters)
  };

}

#endif
