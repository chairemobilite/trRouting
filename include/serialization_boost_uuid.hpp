#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/serialization/split_free.hpp>

// Add serialization support for boost::uuids::uuid
namespace boost {
namespace serialization {

template<class Archive>
inline void save(Archive & ar, const boost::uuids::uuid & uuid, const unsigned int /* version */)
{
    // UUID is just 16 bytes, so we can serialize it as a binary blob
    ar & make_array(uuid.data, uuid.size());
}

template<class Archive>
inline void load(Archive & ar, boost::uuids::uuid & uuid, const unsigned int /* version */)
{
    // Load the UUID data
    ar & make_array(uuid.data, uuid.size());
}

template<class Archive>
inline void serialize(Archive & ar, boost::uuids::uuid & uuid, const unsigned int version)
{
    split_free(ar, uuid, version);
}

} // namespace serialization
} // namespace boost
