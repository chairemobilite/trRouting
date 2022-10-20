#ifndef TR_CONNECTION
#define TR_CONNECTION

namespace TrRouting
{
  class Node;
  
  // tuple representing a connection: departureNode, arrivalNode, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequence in trip, canTransferSameLine, minWaitingTimeSeconds (-1 to inherit from parameters)
  using ConnectionTuple = std::tuple<std::reference_wrapper<const Node>,std::reference_wrapper<const Node>,int,int,int,short,short,int,short,short>;

  enum connectionIndexes : short { NODE_DEP = 0, NODE_ARR = 1, TIME_DEP = 2, TIME_ARR = 3, TRIP = 4, CAN_BOARD = 5, CAN_UNBOARD = 6, SEQUENCE = 7, CAN_TRANSFER_SAME_LINE = 8, MIN_WAITING_TIME_SECONDS = 9 };


}

#endif
