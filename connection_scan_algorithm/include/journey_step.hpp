#ifndef TR_JOURNEY_STEP
#define TR_JOURNEY_STEP

namespace TrRouting {

  class JourneyStep {
  public:
    JourneyStep(std::optional<std::shared_ptr<Connection>> _finalEnterConnection,
                std::optional<std::shared_ptr<Connection>> _finalExitConnection,
                std::optional<std::reference_wrapper<const Trip>> _finalTrip,
                int _transferTravelTime,
                bool _sameNodeTransfer,
                int _transferDistance) : finalEnterConnection(_finalEnterConnection),
                                         finalExitConnection(_finalExitConnection),
                                         finalTrip(_finalTrip),
                                         transferTravelTime(_transferTravelTime),
                                         sameNodeTransfer(_sameNodeTransfer),
                                         transferDistance(_transferDistance) { }

    std::optional<std::shared_ptr<Connection>> getFinalEnterConnection() const {return finalEnterConnection;}
    std::optional<std::shared_ptr<Connection>> getFinalExitConnection() const {return finalExitConnection;}
    std::optional<std::reference_wrapper<const Trip>> getFinalTrip() const {return finalTrip;}
    int getTransferTravelTime() const {return transferTravelTime;}
    bool isSameNodeTransfer() const {return sameNodeTransfer;} //TODO Is is used??
    int getTransferDistance() const {return transferDistance;}

    void setFinalEnterConnection(std::shared_ptr<Connection> _connection) {
      finalEnterConnection = _connection;
    }
    void setFinalExitConnection(std::shared_ptr<Connection> _connection) {
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
    bool hasConnections() const {return finalEnterConnection.has_value() && finalExitConnection.has_value();}
  private:
    std::optional<std::shared_ptr<Connection>> finalEnterConnection;
    std::optional<std::shared_ptr<Connection>> finalExitConnection;
    std::optional<std::reference_wrapper<const Trip>> finalTrip;
    int transferTravelTime;
    bool sameNodeTransfer;
    int transferDistance;
  };

}

#endif
