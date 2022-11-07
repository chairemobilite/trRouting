#ifndef TR_NODE
#define TR_NODE

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "point.hpp"
#include <boost/functional/hash.hpp>

namespace TrRouting
{
  class NodeTimeDistance;
  
  class Node {
  public:

    typedef int uid_t; //Type for a local temporary ID

    Node(const boost::uuids::uuid &_uuid,
         unsigned long long _id,
         const std::string &_code,
         const std::string &_name,
         const std::string &_internalId,
         std::unique_ptr<Point> _point
         ) : uuid(_uuid),
             id(_id),
             code(_code),
             name(_name),
             internalId(_internalId),
             uid(++global_uid)
             {
               point = std::move(_point);
             }
   
    boost::uuids::uuid uuid;
    unsigned long long id;
    std::string code;
    std::string name;
    std::string internalId;
    uid_t uid; //Local, temporary unique id, used to speed up lookups
    std::unique_ptr<Point> point; //TODO Does this need to be a ptr or could be part of the object?
    std::vector<NodeTimeDistance> transferableNodes;
    std::vector<NodeTimeDistance> reverseTransferableNodes; //TODO Add comment on what this is

    const std::string toString() {
      return "Node " + boost::uuids::to_string(uuid) + " (id " + std::to_string(id) + ")\n  code " + code + "\n  name " + name + "\n  latitude " + std::to_string(point.get()->latitude)  + "\n  longitude " + std::to_string(point.get()->longitude);
    }
    // Equal operator. We only compare the local uid, since they should be unique.
    inline bool operator==(const Node& other ) const { return uid == other.uid; }
    inline bool operator<(const Node& other ) const { return uid < other.uid; }

  private:
    //TODO, this could probably be an unsigned long, but current MAX_INT is good enough for our needs
    inline static uid_t global_uid = 0;
  };

  inline bool operator==(const std::reference_wrapper<const TrRouting::Node>& lhs, const std::reference_wrapper<const Node>& rhs)
  {
    return lhs.get() == rhs.get();
  }
  // For sorting and std::map usage
  inline bool operator<(const std::reference_wrapper<const TrRouting::Node>& lhs, const std::reference_wrapper<const Node>& rhs)
  {
    return lhs.get() < rhs.get();
  }

  // Store information about access time and distance to a specific Node
  class NodeTimeDistance {

  public:
    NodeTimeDistance(const Node& _node, int _time, int _distance) :
      node(_node),
      time(_time),
      distance(_distance)
    {

    }
    NodeTimeDistance(const NodeTimeDistance &_node):
      node(_node.node),
      time(_node.time),
      distance(_node.distance)
    {

    }

    const Node & node;
    const int time;
    const int distance;
  };

}

#endif // TR_NODE
