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

    const Node & getDepartureNode() const {return departureNode;}
    const Node & getArrivalNode() const {return arrivalNode;}
    int getDepartureTime() const {return departureTimeSeconds;}
    int getArrivalTime() const {return arrivalTimeSeconds;}
    const Trip & getTrip() const {return trip;}
    //TODO Revisit how we create Trip and Connection to not require a mutable trip here (issue #201
    Trip & getTripMutable() const {return trip;}
    bool canBoard() const {return canBoardValue;}
    bool canUnboard() const {return canUnboardValue;}
    int getSequenceInTrip() const {return sequenceInTrip;}
    bool canTransferSameLine() const {return canTransferSameLineValue;}
    short getMinWaitingTime() const {return minWaitingTimeSeconds;}
    short getMinWaitingTimeOrDefault(short defaultMinWaitingTime) const {
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
