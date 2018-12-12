// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: servicesCollection.capnp

#pragma once

#include <capnp/generated-header-support.h>
#include <kj/windows-sanity.h>

#if CAPNP_VERSION != 7000
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif


namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(cbc91832a29e844e);
CAPNP_DECLARE_SCHEMA(b43b93e1c1ad0616);

}  // namespace schemas
}  // namespace capnp

namespace servicesCollection {

struct ServicesCollection {
  ServicesCollection() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(cbc91832a29e844e, 0, 1)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct Service {
  Service() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(b43b93e1c1ad0616, 2, 6)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

class ServicesCollection::Reader {
public:
  typedef ServicesCollection Reads;

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

  inline bool hasServices() const;
  inline  ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>::Reader getServices() const;

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

class ServicesCollection::Builder {
public:
  typedef ServicesCollection Builds;

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

  inline bool hasServices();
  inline  ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>::Builder getServices();
  inline void setServices( ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>::Reader value);
  inline  ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>::Builder initServices(unsigned int size);
  inline void adoptServices(::capnp::Orphan< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>> disownServices();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class ServicesCollection::Pipeline {
public:
  typedef ServicesCollection Pipelines;

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

class Service::Reader {
public:
  typedef Service Reads;

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

  inline bool hasUuid() const;
  inline  ::capnp::Text::Reader getUuid() const;

  inline  ::int64_t getId() const;

  inline bool hasName() const;
  inline  ::capnp::Text::Reader getName() const;

  inline  ::int8_t getMonday() const;

  inline  ::int8_t getTuesday() const;

  inline  ::int8_t getWednesday() const;

  inline  ::int8_t getThursday() const;

  inline  ::int8_t getFriday() const;

  inline  ::int8_t getSaturday() const;

  inline  ::int8_t getSunday() const;

  inline bool hasStartDate() const;
  inline  ::capnp::Text::Reader getStartDate() const;

  inline bool hasEndDate() const;
  inline  ::capnp::Text::Reader getEndDate() const;

  inline bool hasOnlyDates() const;
  inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Reader getOnlyDates() const;

  inline bool hasExceptDates() const;
  inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Reader getExceptDates() const;

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

class Service::Builder {
public:
  typedef Service Builds;

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

  inline bool hasUuid();
  inline  ::capnp::Text::Builder getUuid();
  inline void setUuid( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initUuid(unsigned int size);
  inline void adoptUuid(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownUuid();

  inline  ::int64_t getId();
  inline void setId( ::int64_t value);

  inline bool hasName();
  inline  ::capnp::Text::Builder getName();
  inline void setName( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initName(unsigned int size);
  inline void adoptName(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownName();

  inline  ::int8_t getMonday();
  inline void setMonday( ::int8_t value);

  inline  ::int8_t getTuesday();
  inline void setTuesday( ::int8_t value);

  inline  ::int8_t getWednesday();
  inline void setWednesday( ::int8_t value);

  inline  ::int8_t getThursday();
  inline void setThursday( ::int8_t value);

  inline  ::int8_t getFriday();
  inline void setFriday( ::int8_t value);

  inline  ::int8_t getSaturday();
  inline void setSaturday( ::int8_t value);

  inline  ::int8_t getSunday();
  inline void setSunday( ::int8_t value);

  inline bool hasStartDate();
  inline  ::capnp::Text::Builder getStartDate();
  inline void setStartDate( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initStartDate(unsigned int size);
  inline void adoptStartDate(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownStartDate();

  inline bool hasEndDate();
  inline  ::capnp::Text::Builder getEndDate();
  inline void setEndDate( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initEndDate(unsigned int size);
  inline void adoptEndDate(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownEndDate();

  inline bool hasOnlyDates();
  inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Builder getOnlyDates();
  inline void setOnlyDates( ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Reader value);
  inline void setOnlyDates(::kj::ArrayPtr<const  ::capnp::Text::Reader> value);
  inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Builder initOnlyDates(unsigned int size);
  inline void adoptOnlyDates(::capnp::Orphan< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>> disownOnlyDates();

  inline bool hasExceptDates();
  inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Builder getExceptDates();
  inline void setExceptDates( ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Reader value);
  inline void setExceptDates(::kj::ArrayPtr<const  ::capnp::Text::Reader> value);
  inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Builder initExceptDates(unsigned int size);
  inline void adoptExceptDates(::capnp::Orphan< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>> disownExceptDates();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Service::Pipeline {
public:
  typedef Service Pipelines;

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

inline bool ServicesCollection::Reader::hasServices() const {
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool ServicesCollection::Builder::hasServices() {
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>::Reader ServicesCollection::Reader::getServices() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>>::get(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline  ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>::Builder ServicesCollection::Builder::getServices() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>>::get(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline void ServicesCollection::Builder::setServices( ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>::Builder ServicesCollection::Builder::initServices(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>>::init(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), size);
}
inline void ServicesCollection::Builder::adoptServices(
    ::capnp::Orphan< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>>::adopt(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>> ServicesCollection::Builder::disownServices() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::servicesCollection::Service,  ::capnp::Kind::STRUCT>>::disown(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}

inline bool Service::Reader::hasUuid() const {
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool Service::Builder::hasUuid() {
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Service::Reader::getUuid() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Service::Builder::getUuid() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline void Service::Builder::setUuid( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Service::Builder::initUuid(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), size);
}
inline void Service::Builder::adoptUuid(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Service::Builder::disownUuid() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}

inline  ::int64_t Service::Reader::getId() const {
  return _reader.getDataField< ::int64_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline  ::int64_t Service::Builder::getId() {
  return _builder.getDataField< ::int64_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline void Service::Builder::setId( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value);
}

inline bool Service::Reader::hasName() const {
  return !_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline bool Service::Builder::hasName() {
  return !_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Service::Reader::getName() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Service::Builder::getName() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline void Service::Builder::setName( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Service::Builder::initName(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), size);
}
inline void Service::Builder::adoptName(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Service::Builder::disownName() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}

inline  ::int8_t Service::Reader::getMonday() const {
  return _reader.getDataField< ::int8_t>(
      ::capnp::bounded<8>() * ::capnp::ELEMENTS);
}

inline  ::int8_t Service::Builder::getMonday() {
  return _builder.getDataField< ::int8_t>(
      ::capnp::bounded<8>() * ::capnp::ELEMENTS);
}
inline void Service::Builder::setMonday( ::int8_t value) {
  _builder.setDataField< ::int8_t>(
      ::capnp::bounded<8>() * ::capnp::ELEMENTS, value);
}

inline  ::int8_t Service::Reader::getTuesday() const {
  return _reader.getDataField< ::int8_t>(
      ::capnp::bounded<9>() * ::capnp::ELEMENTS);
}

inline  ::int8_t Service::Builder::getTuesday() {
  return _builder.getDataField< ::int8_t>(
      ::capnp::bounded<9>() * ::capnp::ELEMENTS);
}
inline void Service::Builder::setTuesday( ::int8_t value) {
  _builder.setDataField< ::int8_t>(
      ::capnp::bounded<9>() * ::capnp::ELEMENTS, value);
}

inline  ::int8_t Service::Reader::getWednesday() const {
  return _reader.getDataField< ::int8_t>(
      ::capnp::bounded<10>() * ::capnp::ELEMENTS);
}

inline  ::int8_t Service::Builder::getWednesday() {
  return _builder.getDataField< ::int8_t>(
      ::capnp::bounded<10>() * ::capnp::ELEMENTS);
}
inline void Service::Builder::setWednesday( ::int8_t value) {
  _builder.setDataField< ::int8_t>(
      ::capnp::bounded<10>() * ::capnp::ELEMENTS, value);
}

inline  ::int8_t Service::Reader::getThursday() const {
  return _reader.getDataField< ::int8_t>(
      ::capnp::bounded<11>() * ::capnp::ELEMENTS);
}

inline  ::int8_t Service::Builder::getThursday() {
  return _builder.getDataField< ::int8_t>(
      ::capnp::bounded<11>() * ::capnp::ELEMENTS);
}
inline void Service::Builder::setThursday( ::int8_t value) {
  _builder.setDataField< ::int8_t>(
      ::capnp::bounded<11>() * ::capnp::ELEMENTS, value);
}

inline  ::int8_t Service::Reader::getFriday() const {
  return _reader.getDataField< ::int8_t>(
      ::capnp::bounded<12>() * ::capnp::ELEMENTS);
}

inline  ::int8_t Service::Builder::getFriday() {
  return _builder.getDataField< ::int8_t>(
      ::capnp::bounded<12>() * ::capnp::ELEMENTS);
}
inline void Service::Builder::setFriday( ::int8_t value) {
  _builder.setDataField< ::int8_t>(
      ::capnp::bounded<12>() * ::capnp::ELEMENTS, value);
}

inline  ::int8_t Service::Reader::getSaturday() const {
  return _reader.getDataField< ::int8_t>(
      ::capnp::bounded<13>() * ::capnp::ELEMENTS);
}

inline  ::int8_t Service::Builder::getSaturday() {
  return _builder.getDataField< ::int8_t>(
      ::capnp::bounded<13>() * ::capnp::ELEMENTS);
}
inline void Service::Builder::setSaturday( ::int8_t value) {
  _builder.setDataField< ::int8_t>(
      ::capnp::bounded<13>() * ::capnp::ELEMENTS, value);
}

inline  ::int8_t Service::Reader::getSunday() const {
  return _reader.getDataField< ::int8_t>(
      ::capnp::bounded<14>() * ::capnp::ELEMENTS);
}

inline  ::int8_t Service::Builder::getSunday() {
  return _builder.getDataField< ::int8_t>(
      ::capnp::bounded<14>() * ::capnp::ELEMENTS);
}
inline void Service::Builder::setSunday( ::int8_t value) {
  _builder.setDataField< ::int8_t>(
      ::capnp::bounded<14>() * ::capnp::ELEMENTS, value);
}

inline bool Service::Reader::hasStartDate() const {
  return !_reader.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS).isNull();
}
inline bool Service::Builder::hasStartDate() {
  return !_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Service::Reader::getStartDate() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Service::Builder::getStartDate() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS));
}
inline void Service::Builder::setStartDate( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Service::Builder::initStartDate(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS), size);
}
inline void Service::Builder::adoptStartDate(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Service::Builder::disownStartDate() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS));
}

inline bool Service::Reader::hasEndDate() const {
  return !_reader.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS).isNull();
}
inline bool Service::Builder::hasEndDate() {
  return !_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Service::Reader::getEndDate() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Service::Builder::getEndDate() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS));
}
inline void Service::Builder::setEndDate( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Service::Builder::initEndDate(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS), size);
}
inline void Service::Builder::adoptEndDate(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Service::Builder::disownEndDate() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS));
}

inline bool Service::Reader::hasOnlyDates() const {
  return !_reader.getPointerField(
      ::capnp::bounded<4>() * ::capnp::POINTERS).isNull();
}
inline bool Service::Builder::hasOnlyDates() {
  return !_builder.getPointerField(
      ::capnp::bounded<4>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Reader Service::Reader::getOnlyDates() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::get(_reader.getPointerField(
      ::capnp::bounded<4>() * ::capnp::POINTERS));
}
inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Builder Service::Builder::getOnlyDates() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::get(_builder.getPointerField(
      ::capnp::bounded<4>() * ::capnp::POINTERS));
}
inline void Service::Builder::setOnlyDates( ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::set(_builder.getPointerField(
      ::capnp::bounded<4>() * ::capnp::POINTERS), value);
}
inline void Service::Builder::setOnlyDates(::kj::ArrayPtr<const  ::capnp::Text::Reader> value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::set(_builder.getPointerField(
      ::capnp::bounded<4>() * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Builder Service::Builder::initOnlyDates(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::init(_builder.getPointerField(
      ::capnp::bounded<4>() * ::capnp::POINTERS), size);
}
inline void Service::Builder::adoptOnlyDates(
    ::capnp::Orphan< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::adopt(_builder.getPointerField(
      ::capnp::bounded<4>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>> Service::Builder::disownOnlyDates() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::disown(_builder.getPointerField(
      ::capnp::bounded<4>() * ::capnp::POINTERS));
}

inline bool Service::Reader::hasExceptDates() const {
  return !_reader.getPointerField(
      ::capnp::bounded<5>() * ::capnp::POINTERS).isNull();
}
inline bool Service::Builder::hasExceptDates() {
  return !_builder.getPointerField(
      ::capnp::bounded<5>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Reader Service::Reader::getExceptDates() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::get(_reader.getPointerField(
      ::capnp::bounded<5>() * ::capnp::POINTERS));
}
inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Builder Service::Builder::getExceptDates() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::get(_builder.getPointerField(
      ::capnp::bounded<5>() * ::capnp::POINTERS));
}
inline void Service::Builder::setExceptDates( ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::set(_builder.getPointerField(
      ::capnp::bounded<5>() * ::capnp::POINTERS), value);
}
inline void Service::Builder::setExceptDates(::kj::ArrayPtr<const  ::capnp::Text::Reader> value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::set(_builder.getPointerField(
      ::capnp::bounded<5>() * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>::Builder Service::Builder::initExceptDates(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::init(_builder.getPointerField(
      ::capnp::bounded<5>() * ::capnp::POINTERS), size);
}
inline void Service::Builder::adoptExceptDates(
    ::capnp::Orphan< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::adopt(_builder.getPointerField(
      ::capnp::bounded<5>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>> Service::Builder::disownExceptDates() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Text,  ::capnp::Kind::BLOB>>::disown(_builder.getPointerField(
      ::capnp::bounded<5>() * ::capnp::POINTERS));
}

}  // namespace

