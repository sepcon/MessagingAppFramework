#pragma once

#include <maf/utils/cppextension/TupleManip.h>
#include <maf/utils/cppextension/TypeTraits.h>

#include <cassert>
#include <string>
#include <tuple>

#include "Tuplizable.h"

/// Serialization

#define mc_enable_if_is_tuplelike_(TypeName) \
  template <typename TypeName,               \
            std::enable_if_t<is_tuplizable_type_v<TypeName>, bool> = true>
#define mc_enable_if_is_number_or_enum_(NumberType)                     \
  template <typename NumberType,                                        \
            std::enable_if_t<nstl::is_number_type<NumberType>::value || \
                                 std::is_enum_v<NumberType>,            \
                             bool> = true>
#define mc_enable_if_is_smartptr_(SmartPtrType) \
  template <typename SmartPtrType,              \
            std::enable_if_t<nstl::is_smart_ptr_v<SmartPtrType>, bool> = true>
#define mc_enable_if_is_ptr_(PointerType) \
  template <typename PointerType,         \
            std::enable_if_t<std::is_pointer_v<PointerType>, bool> = true>
#define mc_enable_if_is_a_char_string(CharString)                          \
  template <typename CharString,                                           \
            std::enable_if_t<std::is_base_of_v<std::string, CharString> && \
                                 !std::is_same_v<std::string, CharString>, \
                             bool> = true>
#define mc_must_default_constructible(PointerType)                         \
  static_assert(                                                           \
      std::is_default_constructible_v<std::remove_pointer_t<PointerType>>, \
      "");

#define mc_enable_if_is_iterable_(Container) \
  template <typename Container,              \
            std::enable_if_t<nstl::is_iterable_v<Container>, bool> = true>

namespace maf {
namespace srz {

using namespace nstl;

using SizeType = uint32_t;
inline constexpr SizeType SIZETYPE_WIDE = sizeof(SizeType);

template <class OStream, typename T>
void serialize(OStream &, const T &);

template <class IStream, typename T>
T deserialize(IStream &, bool &);

namespace internal {

template <class Container, typename = void>
struct ContainerReserver {
  static void reserve(Container & /*c*/, SizeType /*size*/) {}
};

template <typename Container>
struct ContainerReserver<
    Container, std::void_t<decltype(std::declval<Container>().reserve(0))>> {
  static void reserve(Container &c, SizeType size) {
    c.reserve(static_cast<size_t>(size));
  }
};

template <typename T>
char *to_cstr(T *value) {
  return reinterpret_cast<char *>(value);
}

template <typename T>
const char *to_cstr(const T *value) {
  return reinterpret_cast<const char *>(value);
}

template <class StreamType, typename = void>
struct StreamHelper {
  static void prepareNextWrite(StreamType &, SizeType) {}
};

template <class T>
struct DeserializableType {
  using Type = std::remove_const_t<T>;
};

template <class First, class Second>
struct DeserializableType<std::pair<First, Second>> {
  using Type =
      std::pair<std::remove_const_t<First>, std::remove_const_t<Second>>;
};

template <class... Args>
struct DeserializableType<std::tuple<Args...>> {
  using Type = std::tuple<std::remove_const_t<Args>...>;
};

}  // namespace internal

template <class OStream, class IStream, typename T, typename = void>
struct Serializer {
  template <typename TT>
  using SerializerT = Serializer<OStream, IStream, TT>;

  void serialize(OStream &os, const T &value) {
    Impl::template serialize<T>(os, value);
  }
  bool deserialize(IStream &is, T &val) {
    return Impl::template deserialize<T>(is, val);
  }
  SizeType serializedSize(const T &value) noexcept {
    return Impl::template serializedSize<T>(value);
  }

  struct Impl {
    mc_enable_if_is_tuplelike_(TupleLike) inline static SizeType
        serializedSize(const TupleLike &value) noexcept {
      return SerializerT<decltype(value.as_tuple())>{}.serializedSize(
          value.as_tuple());
    }

    mc_enable_if_is_tuplelike_(TupleLike) static void serialize(
        OStream &os, const TupleLike &value) {
      SerializerT<decltype(value.as_tuple())>{}.serialize(os, value.as_tuple());
    }

    mc_enable_if_is_tuplelike_(TupleLike) static bool deserialize(
        IStream &is, TupleLike &tpl) {
      auto tp = tpl.as_tuple();
      return Serializer<OStream, IStream, decltype(tp)>{}.deserialize(is, tp);
    }

    mc_enable_if_is_number_or_enum_(NumberOrEnum) static SizeType
        serializedSize(const NumberOrEnum & /*value*/) noexcept {
      return sizeof(NumberOrEnum);
    }

    mc_enable_if_is_number_or_enum_(NumberOrEnum) static void serialize(
        OStream &os, const NumberOrEnum &value) {
      os.write(internal::to_cstr(&value), sizeof(NumberOrEnum));
    }

    mc_enable_if_is_number_or_enum_(NumberOrEnum) static bool deserialize(
        IStream &is, NumberOrEnum &value) {
      is.read(internal::to_cstr(&value), sizeof(NumberOrEnum));
      return !is.fail();
    }

    mc_enable_if_is_ptr_(PointerType) inline static SizeType
        serializedSize(PointerType value) noexcept {
      SizeType size = 1;
      if (value) {
        using NormalTypeOfPointerType =
            std::remove_const_t<std::remove_pointer_t<PointerType>>;
        size += SerializerT<NormalTypeOfPointerType>{}.serializedSize(*value);
      }
      return size;
    }

    mc_enable_if_is_ptr_(PointerType) static void serialize(
        OStream &os, const PointerType &p) {
      // Won't serialize type that is not default_constructible because later
      // won't be able to create object of that types to deserialize
      mc_must_default_constructible(PointerType);
      char c = 1;
      if (p) {
        using NormalTypeOfPointerType =
            std::remove_const_t<std::remove_pointer_t<PointerType>>;
        os.write(&c, 1);
        SerializerT<NormalTypeOfPointerType>{}.serialize(os, *p);
      } else {
        c = 0;
        os.write(&c, 1);
      }
    }

    mc_enable_if_is_ptr_(PointerType) static bool deserialize(
        IStream &is, pure_type_t<PointerType> &p) {
      mc_must_default_constructible(PointerType);

      using NormalTypeOfPointerType =
          std::remove_const_t<std::remove_pointer_t<PointerType>>;
      auto success = true;
      p = nullptr;
      uint8_t isNotNull = 0;

      is.read(internal::to_cstr(&isNotNull), 1);
      if (isNotNull) {
        p = new NormalTypeOfPointerType{};
        success = SerializerT<NormalTypeOfPointerType>{}.deserialize(is, *p);
        if (!success) {
          delete p;
          p = nullptr;
        }
      }

      return success;
    }

    mc_enable_if_is_smartptr_(SmartPtrType) static SizeType
        serializedSize(const SmartPtrType &value) noexcept {
      using PtrType = typename SmartPtrType::element_type *;
      return SerializerT<PtrType>{}.serializedSize(value.get());
    }

    mc_enable_if_is_smartptr_(SmartPtrType) static void serialize(
        OStream &os, const SmartPtrType &p) {
      using PtrType = typename SmartPtrType::element_type *;
      SerializerT<PtrType>{}.serialize(os, p.get());
    }

    mc_enable_if_is_smartptr_(SmartPtrType) static bool deserialize(
        IStream &is, pure_type_t<SmartPtrType> &sptr) {
      using PtrType = typename SmartPtrType::element_type *;
      auto success = false;
      PtrType ptr = nullptr;
      if (success = SerializerT<PtrType>{}.deserialize(is, ptr); success) {
        sptr.reset(ptr);
      }
      return success;
    }

    mc_enable_if_is_iterable_(Container) inline static SizeType
        serializedSize(const Container &c) noexcept {
      SizeType contentSize = 0;
      auto sr = SerializerT<typename Container::value_type>{};
      for (const auto &e : c) {
        contentSize += sr.serializedSize(e);
      }
      return SIZETYPE_WIDE + contentSize;
    }

    mc_enable_if_is_iterable_(Container) static void serialize(
        OStream &os, const Container &c) {
      auto numberOfElems = static_cast<SizeType>(c.size());
      auto elemSrz =
          Serializer<OStream, IStream, typename Container::value_type>{};

      SerializerT<SizeType>{}.serialize(os, numberOfElems);

      for (const auto &elem : c) {
        elemSrz.serialize(os, elem);
      }
    }

    template <typename Container,
              std::enable_if_t<nstl::is_iterable_v<Container>, bool> = true>
    static bool deserialize(IStream &is, Container &c) {
      using ElemType = typename Container::value_type;
      using DSBElemType = typename internal::DeserializableType<ElemType>::Type;
      using SizeTypeSerializer = SerializerT<SizeType>;

      constexpr bool pushBackable = nstl::is_back_insertible_v<Container>;
      auto success = false;
      auto esr = SerializerT<DSBElemType>{};
      SizeType size = 0;

      if (success |= SizeTypeSerializer{}.deserialize(is, size);
          success && size > 0) {
        internal::ContainerReserver<Container>::reserve(c, size);
        for (SizeType i = 0; i < size; ++i) {
          DSBElemType elem;
          if (success |= esr.deserialize(is, elem); success) {
            if constexpr (pushBackable) {
              c.push_back(std::move(elem));
            } else {
              c.insert(std::move(elem));
            }
          } else {
            break;
          }
        }
      }
      return success;
    }
  };
};

template <class OStream, class IStream, typename First, typename Second>
struct Serializer<OStream, IStream, std::pair<First, Second>, void> {
  template <typename T>
  using SerializerT = Serializer<OStream, IStream, T>;

  using DType = std::pair<First, Second>;
  using FirstPure = pure_type_t<First>;
  using SecondPure = pure_type_t<Second>;
  using NCDType = std::pair<FirstPure, SecondPure>;

  SizeType serializedSize(const DType &p) noexcept {
    return firstSr_.serializedSize(p.first) +
           secondSr_.serializedSize(p.second);
  }

  void serialize(OStream &os, const DType &p) {
    firstSr_.serialize(os, p.first);
    secondSr_.serialize(os, p.second);
  }

  bool deserialize(IStream &is, NCDType &value) {
    bool success = false;
    if (success |= firstSr_.deserialize(is, value.first); success) {
      success |= secondSr_.deserialize(is, value.second);
    }
    return success;
  }

  SerializerT<FirstPure> firstSr_;
  SerializerT<SecondPure> secondSr_;
};

template <class OStream, class IStream, typename Tuple>
struct Serializer<OStream, IStream, Tuple,
                  std::enable_if_t<nstl::is_tuple_v<Tuple>, void>> {
  template <typename T>
  using SerializerT = Serializer<OStream, IStream, T>;
  using SrType = Tuple;

  SizeType serializedSize(const Tuple &tp) noexcept {
    SizeType contentSize = 0;
    nstl::tuple_for_each(tp, [&contentSize](const auto &elem) {
      contentSize +=
          SerializerT<pure_type_t<decltype(elem)>>{}.serializedSize(elem);
    });
    return contentSize;
  }

  void serialize(OStream &os, const SrType &tp) {
    nstl::tuple_for_each(tp, [&os](const auto &elem) {
      SerializerT<pure_type_t<decltype(elem)>>{}.serialize(os, elem);
    });
  }

  bool deserialize(IStream &is, SrType &tp) {
    bool success = false;
    nstl::tuple_for_each(tp, [&is, &success](auto &elem) {
      success |=
          SerializerT<pure_type_t<decltype(elem)>>{}.deserialize(is, elem);
    });
    return success;
  }
};

template <class OStream, class IStream, typename CharT, class Trait,
          class Allocator>
struct Serializer<OStream, IStream, std::basic_string<CharT, Trait, Allocator>,
                  void> {
  template <typename T>
  using SerializerT = Serializer<OStream, IStream, T>;
  using SrType = std::basic_string<CharT>;

  SizeType serializedSize(const SrType &value) noexcept {
    return SIZETYPE_WIDE + static_cast<SizeType>(value.size() * sizeof(CharT));
  }

  void serialize(OStream &os, const SrType &value) {
    auto size = value.size();
    os.write(internal::to_cstr(&size), sizeof(SizeType));
    if (!value.empty()) {
      os.write(reinterpret_cast<const char *>(value.c_str()),
               value.size() * sizeof(CharT));
    }
  }

  bool deserialize(IStream &is, SrType &value) {
    SizeType size = 0;
    if (SerializerT<SizeType>{}.deserialize(is, size)) {
      if (size > 0) {
        value.resize(size);
        is.read(internal::to_cstr(&value[0]), size * sizeof(CharT));
      }
    }
    return !is.fail();
  }
};

template <class OStream, class IStream, typename StringDerived>
struct Serializer<
    OStream, IStream, StringDerived,
    std::enable_if_t<std::is_base_of_v<std::string, StringDerived> &&
                         !std::is_same_v<std::string, StringDerived>,
                     void>> {
  template <typename T>
  using SerializerT = Serializer<OStream, IStream, T>;
  using SrType = StringDerived;

  SizeType serializedSize(const SrType &value) noexcept {
    return ssr_.serializedSize(static_cast<const std::string &>(value));
  }

  void serialize(OStream &os, const SrType &value) {
    ssr_.serialize(os, static_cast<const std::string &>(value));
  }

  bool deserialize(IStream &is, SrType &value) {
    return ssr_.deserialize(is, static_cast<std::string &>(value));
  }

  SerializerT<std::string> ssr_;
};

template <class OStream, typename T>
void serialize(OStream &os, const T &v) {
  using namespace internal;
  auto sr = Serializer<OStream, std::nullptr_t, T>{};
  StreamHelper<OStream>::prepareNextWrite(os, sr.serializedSize(v));
  sr.serialize(os, v);
}

template <class IStream, typename T>
bool deserialize(IStream &is, T &value) {
  return Serializer<std::nullptr_t, IStream, T>{}.deserialize(is, value);
}

template <class OStream>
class SR {
  OStream &os_;

 public:
  SR(OStream &os) : os_{os} {}
  template <typename T>
  SR &operator<<(const T &value) {
    serialize(os_, value);
    return *this;
  }

  template <typename... Ts>
  void serializeBatch(const Ts &... ts) {
    serialize(os_, std::tie(ts...));
  }
};

template <class IStream>
class DSR {
  IStream &is_;

 public:
  DSR(IStream &is) : is_{is} {}
  template <typename T>
  DSR &operator>>(T &value) {
    if (!deserialize<IStream, T>(is_, value)) {
      throw std::runtime_error{"Could not deserialize"};
    }
    return *this;
  }

  template <typename... Ts>
  DSR &deserializeBatch(Ts &... ts) {
    auto tp = std::tie(ts...);
    if (!deserialize<IStream, std::tuple<Ts...>>(is_, tp)) {
      throw std::runtime_error{"Could not deserialize"};
    }
    return *this;
  }
};

}  // namespace srz
}  // namespace maf

#undef mc_enable_if_is_tuplelike_
#undef mc_enable_if_is_number_or_enum_
#undef mc_enable_if_is_smartptr_
#undef mc_enable_if_is_ptr_
#undef mc_enable_if_is_a_char_string
#undef mc_must_default_constructible
#undef mc_enable_if_is_iterable_
