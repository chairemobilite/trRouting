// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: odTripFootpathsCollection.capnp

#ifndef CAPNP_INCLUDED_964450da4385f299_
#define CAPNP_INCLUDED_964450da4385f299_

#include <capnp/generated-header-support.h>

#if CAPNP_VERSION != 6001
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif


namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(db9d7702f2aac194);
CAPNP_DECLARE_SCHEMA(d095107ffc3abb1d);
CAPNP_DECLARE_SCHEMA(c0e27f126aae2d75);

}  // namespace schemas
}  // namespace capnp

namespace odTripFootpathsCollection {

struct OdTripFootpathsCollection {
  OdTripFootpathsCollection() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(db9d7702f2aac194, 0, 2)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct OdTripFootpath {
  OdTripFootpath() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(d095107ffc3abb1d, 1, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct OdTripFootpathRange {
  OdTripFootpathRange() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(c0e27f126aae2d75, 1, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

class OdTripFootpathsCollection::Reader {
public:
  typedef OdTripFootpathsCollection Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline bool hasOdTripFootpaths() const;
  inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>::Reader getOdTripFootpaths() const;

  inline bool hasOdTripFootpathRanges() const;
  inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>::Reader getOdTripFootpathRanges() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class OdTripFootpathsCollection::Builder {
public:
  typedef OdTripFootpathsCollection Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline bool hasOdTripFootpaths();
  inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>::Builder getOdTripFootpaths();
  inline void setOdTripFootpaths( ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>::Reader value);
  inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>::Builder initOdTripFootpaths(unsigned int size);
  inline void adoptOdTripFootpaths(::capnp::Orphan< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>> disownOdTripFootpaths();

  inline bool hasOdTripFootpathRanges();
  inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>::Builder getOdTripFootpathRanges();
  inline void setOdTripFootpathRanges( ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>::Reader value);
  inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>::Builder initOdTripFootpathRanges(unsigned int size);
  inline void adoptOdTripFootpathRanges(::capnp::Orphan< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>> disownOdTripFootpathRanges();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class OdTripFootpathsCollection::Pipeline {
public:
  typedef OdTripFootpathsCollection Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class OdTripFootpath::Reader {
public:
  typedef OdTripFootpath Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline  ::int32_t getStopIdx() const;

  inline  ::int32_t getTravelTime() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class OdTripFootpath::Builder {
public:
  typedef OdTripFootpath Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::int32_t getStopIdx();
  inline void setStopIdx( ::int32_t value);

  inline  ::int32_t getTravelTime();
  inline void setTravelTime( ::int32_t value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class OdTripFootpath::Pipeline {
public:
  typedef OdTripFootpath Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class OdTripFootpathRange::Reader {
public:
  typedef OdTripFootpathRange Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline  ::int32_t getFootpathsStartIdx() const;

  inline  ::int32_t getFootpathsEndIdx() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class OdTripFootpathRange::Builder {
public:
  typedef OdTripFootpathRange Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::int32_t getFootpathsStartIdx();
  inline void setFootpathsStartIdx( ::int32_t value);

  inline  ::int32_t getFootpathsEndIdx();
  inline void setFootpathsEndIdx( ::int32_t value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class OdTripFootpathRange::Pipeline {
public:
  typedef OdTripFootpathRange Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

// =======================================================================================

inline bool OdTripFootpathsCollection::Reader::hasOdTripFootpaths() const {
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool OdTripFootpathsCollection::Builder::hasOdTripFootpaths() {
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>::Reader OdTripFootpathsCollection::Reader::getOdTripFootpaths() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>>::get(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>::Builder OdTripFootpathsCollection::Builder::getOdTripFootpaths() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>>::get(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline void OdTripFootpathsCollection::Builder::setOdTripFootpaths( ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>::Builder OdTripFootpathsCollection::Builder::initOdTripFootpaths(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>>::init(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), size);
}
inline void OdTripFootpathsCollection::Builder::adoptOdTripFootpaths(
    ::capnp::Orphan< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>>::adopt(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>> OdTripFootpathsCollection::Builder::disownOdTripFootpaths() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpath>>::disown(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}

inline bool OdTripFootpathsCollection::Reader::hasOdTripFootpathRanges() const {
  return !_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline bool OdTripFootpathsCollection::Builder::hasOdTripFootpathRanges() {
  return !_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>::Reader OdTripFootpathsCollection::Reader::getOdTripFootpathRanges() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>>::get(_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>::Builder OdTripFootpathsCollection::Builder::getOdTripFootpathRanges() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>>::get(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline void OdTripFootpathsCollection::Builder::setOdTripFootpathRanges( ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>>::set(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>::Builder OdTripFootpathsCollection::Builder::initOdTripFootpathRanges(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>>::init(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), size);
}
inline void OdTripFootpathsCollection::Builder::adoptOdTripFootpathRanges(
    ::capnp::Orphan< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>>::adopt(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>> OdTripFootpathsCollection::Builder::disownOdTripFootpathRanges() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::odTripFootpathsCollection::OdTripFootpathRange>>::disown(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}

inline  ::int32_t OdTripFootpath::Reader::getStopIdx() const {
  return _reader.getDataField< ::int32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline  ::int32_t OdTripFootpath::Builder::getStopIdx() {
  return _builder.getDataField< ::int32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline void OdTripFootpath::Builder::setStopIdx( ::int32_t value) {
  _builder.setDataField< ::int32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value);
}

inline  ::int32_t OdTripFootpath::Reader::getTravelTime() const {
  return _reader.getDataField< ::int32_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS);
}

inline  ::int32_t OdTripFootpath::Builder::getTravelTime() {
  return _builder.getDataField< ::int32_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS);
}
inline void OdTripFootpath::Builder::setTravelTime( ::int32_t value) {
  _builder.setDataField< ::int32_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS, value);
}

inline  ::int32_t OdTripFootpathRange::Reader::getFootpathsStartIdx() const {
  return _reader.getDataField< ::int32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline  ::int32_t OdTripFootpathRange::Builder::getFootpathsStartIdx() {
  return _builder.getDataField< ::int32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline void OdTripFootpathRange::Builder::setFootpathsStartIdx( ::int32_t value) {
  _builder.setDataField< ::int32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value);
}

inline  ::int32_t OdTripFootpathRange::Reader::getFootpathsEndIdx() const {
  return _reader.getDataField< ::int32_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS);
}

inline  ::int32_t OdTripFootpathRange::Builder::getFootpathsEndIdx() {
  return _builder.getDataField< ::int32_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS);
}
inline void OdTripFootpathRange::Builder::setFootpathsEndIdx( ::int32_t value) {
  _builder.setDataField< ::int32_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS, value);
}

}  // namespace

#endif  // CAPNP_INCLUDED_964450da4385f299_
