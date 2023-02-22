#ifndef TR_JOURNEY_STEP
#define TR_JOURNEY_STEP

namespace TrRouting {

  //a journey step can be walking from origin to first stop,
  //riding between boarding and unboarding stops in vehicle,
  //transfering by foot or walking from last stop to destination
  //TODO We could change this to be a base class and have derived one for
  //each of the step types. See issue #209
  class JourneyStep {
  public:
    JourneyStep(std::optional<std::reference_wrapper<const Connection>> _finalEnterConnection,
                std::optional<std::reference_wrapper<const Connection>> _finalExitConnection,
                std::optional<std::reference_wrapper<const Trip>> _finalTrip,
                int _transferTravelTime,
                bool _sameNodeTransfer,
                int _transferDistance) : finalEnterConnection(_finalEnterConnection),
                                         finalExitConnection(_finalExitConnection),
                                         finalTrip(_finalTrip),
                                         transferTravelTime(_transferTravelTime),
                                         sameNodeTransfer(_sameNodeTransfer),
                                         transferDistance(_transferDistance) { }

    //TODO Why is there Final in the function name. Is that appropriate?
    std::optional<std::reference_wrapper<const Connection>> getFinalEnterConnection() const {return finalEnterConnection;}
    std::optional<std::reference_wrapper<const Connection>> getFinalExitConnection() const {return finalExitConnection;}
    std::optional<std::reference_wrapper<const Trip>> getFinalTrip() const {return finalTrip;}
    //TODO: Comment from @tahini.
    // So walking is tranferring in a transfer step, but walking is just walking in an access/egress step.
    // So for the function name, getWalkingTravelTime or getWalkingTravelDistance would be more appropriate
    //(since transfer is a concept that means something that isn't this). Or anything that means what it
    //should and does not imply transferring from one line to the other.
    // Again, this is something that might be more meaningful after a refactoring #209
    int getTransferTravelTime() const {return transferTravelTime;}
    bool isSameNodeTransfer() const {return sameNodeTransfer;} //TODO Is is used??
    int getTransferDistance() const {return transferDistance;}

    //TODO Setting connections midway should also affected other parameters automatically
    // Their usage in the optimise function should be reviewed (Maybe in conjunction with #209)
    void setFinalEnterConnection(const Connection&  _connection) {
      finalEnterConnection = _connection;
    }
    void setFinalExitConnection(const Connection& _connection) {
      finalExitConnection = _connection;
    }

    void setTransferTimeDistance(int _transferTime, int _transferDistance) {
      transferTravelTime = _transferTime;
      transferDistance = _transferDistance;
    }
    // Copy time and distance from another JourneyStep object
    void copyTransferTimeDistance(const JourneyStep &_other) {
      transferTravelTime = _other.transferTravelTime;
      transferDistance = _other.transferDistance;
    }

    // Return true if both final and exit connections are valid
    // TODO This could have a more meaningful name, if we find out what this represent in real life
    // In one instance, the comment associated with calling it says "check if it is an in-vehicle journey"
    // If we have type of each step type, maybe this would be needed. #209
    bool hasConnections() const {return finalEnterConnection.has_value() && finalExitConnection.has_value();}
  private:
    std::optional<std::reference_wrapper<const Connection>> finalEnterConnection;
    std::optional<std::reference_wrapper<const Connection>> finalExitConnection;
    std::optional<std::reference_wrapper<const Trip>> finalTrip;
    int transferTravelTime;
    bool sameNodeTransfer;
    int transferDistance;
  };

}

#endif
