#ifndef TR_CONNECTION_SET
#define TR_CONNECTION_SET

#include <vector>


namespace TrRouting {

class Connection;
class Trip;

/**
 * @brief Encapsulates a subset of the whole connection set, ie the connections
 * that are used by the trips
 */
class ConnectionSet {

  public:
    
    ConnectionSet(
        const std::vector<std::reference_wrapper<const Trip>> _trips,
        const std::vector<std::reference_wrapper<Connection>> _forwardConnections,
        const std::vector<std::reference_wrapper<Connection>> _reverseConnections
    );
    const std::vector<std::reference_wrapper<const Trip>> & getTrips() const {return trips;}
    const std::vector<std::reference_wrapper<Connection>> & getForwardConnections() const {return forwardConnections;}
    const std::vector<std::reference_wrapper<Connection>> & getReverseConnections() const {return reverseConnections;}

    std::vector<std::reference_wrapper<Connection>>::const_iterator getForwardConnectionsBeginAtDepartureHour(int hour) const;
    std::vector<std::reference_wrapper<Connection>>::const_iterator getReverseConnectionsBeginAtArrivalHour(int hour) const;

  private:
    std::vector<std::reference_wrapper<const Trip>> trips;
    std::vector<std::reference_wrapper<Connection>> forwardConnections; // Forward connections, sorted by departure time ascending
    std::vector<std::reference_wrapper<Connection>> reverseConnections; // Reverse connections, sorted by arrival time descending

    // Contains iterator matching each hour of the day from the corresponding connections container.
    // Used to speed up iterating the connections by skipping the connections that are too early or too late
    std::vector<std::vector<std::reference_wrapper<Connection>>::const_iterator> forwardConnectionsBeginIteratorCache;
    std::vector<std::vector<std::reference_wrapper<Connection>>::const_iterator> reverseConnectionsBeginIteratorCache;

    void generateConnectionsIteratorCache();
};

}

#endif // TR_CONNECTION_SET
