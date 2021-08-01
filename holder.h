#pragma once

#include "attribute.h"

#include <optional>
#include <functional>
#include <unordered_map>


namespace porter::attr {
namespace traits {


/// Template for checking whether an attribute holder type is valid.
template<typename H, typename = void> struct is_valid : std::false_type {};

/// Helper variable for checking whether an attribute holder type is valid.
template<typename H> inline constexpr bool is_valid_v = is_valid<H>::value;


} // namespace traits;


/// Defines a single value storage for an attribute (tag).
/// Can be set or updated from Value container.
/// @tparam Tag tag type that defines attribute properties.
/// @tparam Required if set to true, empty values will be treated as invalid state.
/// @tparam V attribute value type. Normally auto-deduced.
template<typename Tag, bool Required, typename V = typename Tag::type>
class Single
{
    static_assert(traits::is_tag_valid_v<Tag>, "Tag type is invalid");

    /// Stores attribute value.
    std::optional<V> d_value;

public:
    /// Defines attribute storage type.
    using type = std::optional<V>;

    /// Assign from a Value container with different tag or value type.
    /// Not allowed; causes static assertion.
    template<typename Tagt, typename Vt>
    Single<Tag, Required, V> &operator=(const Value<Tagt, Vt> &);

    /// Copy-assign from a compatible Value container.
    /// @param value value container to copy attribute value from.
    /// @return reference to self.
    Single<Tag, Required, V> &operator=(const Value<Tag, V> &value);

    /// Move-assign from a compatible Value container.
    /// @param value value container to move attribute value from.
    /// @return reference to self.
    Single<Tag, Required, V> &operator=(Value<Tag, V> &&value);

    /// Assign from a KeyValue container.
    /// Not allowed; causes static assertion.
    template<typename Vt>
    Single<Tag, Required, V> &operator=(const KeyValue<Tag, Vt> &);

    /// Checks whether attribute storage state is valid.
    /// @return true if the value is present or not required.
    operator bool() const;

    /// Get stored value.
    /// @return const reference to stored value.
    const type &operator*() const &;

    /// Take stored value.
    /// @return rvalue reference to stored value.
    type &&operator*() &&;
};


/// Defines an associative multiple value storage for an attribute (tag).
/// Can be extended or updated from KeyValue container.
/// @tparam Tag tag type that defines attribute properties.
/// @tparam V attribute value type. Normally auto-deduced.
template<typename Tag, typename V = typename Tag::type>
class Multiple
{
    static_assert(traits::is_tag_valid_v<Tag>, "Tag type is invalid");

    /// Stores attribute values.
    std::unordered_map<std::string_view, V> d_values;

public:
    /// Defines attribute name type.
    using key_type = std::string_view;

    /// Defines stored attribute value.
    using mapped_type = std::optional<std::reference_wrapper<const V>>;

    /// Defines attribute value storage type.
    using type = std::unordered_map<key_type, V>;

    /// Assign from KeyValue container with different tag or value type.
    /// Not allowed; causes a static assertion.
    template<typename Tagt, typename Vt>
    Multiple<Tag, V> &operator=(const KeyValue<Tagt, Vt> &);

    /// Copy-assign from a compatible KeyValue container.
    /// @param kv key-value pair to copy.
    /// @return reference to self.
    Multiple<Tag, V> &operator=(const KeyValue<Tag> &kv);

    /// Move-assign from a compatible KeyValue container.
    /// @param kv key-value pair to move.
    /// @return reference to self.
    Multiple<Tag, V> &operator=(KeyValue<Tag> &&kv);

    /// Assign from Value container.
    /// Not allowed; causes a static assertion.
    template<typename Vt>
    Multiple<Tag, V> &operator=(const Value<Tag, Vt> &);

    /// Checks whether attribute storage state is valid.
    /// Always returns true.
    operator bool() const;

    /// Get value storage.
    /// @return const reference to value storage.
    const type &operator*() const &;

    /// Take value storage.
    /// @return rvalue reference to value storage.
    type &&operator*() &&;

    /// Get storeg value by key.
    /// @param item value key.
    /// @return stored value reference or std::nullopt, if key was not found.
    mapped_type operator()(const key_type &item) const;
};


namespace traits {


/// Defines validity check for single attribute value holder.
template<typename Tag, bool Required, typename V>
struct is_valid<Single<Tag, Required, V>, std::enable_if_t<traits::is_tag_valid_v<Tag>>> : std::true_type {};

/// Defines validity check for multiple attribute values holder.
template<typename Tag, typename V>
struct is_valid<Multiple<Tag, V>, std::enable_if_t<traits::is_tag_valid_v<Tag>>> : std::true_type {};


/// Defines tag type for single attribute value holder.
template<typename Tag, bool Required, typename V>
struct tag_of<Single<Tag, Required, V>> { using type = Tag; };

/// Defines tag type for multiple attribute values holder.
template<typename Tag, typename V>
struct tag_of<Multiple<Tag, V>> { using type = Tag; };


/// Defines holder type for provided value container type.
template<typename Attr> struct from;

/// Defines holder type for Value container types.
template<typename Tag, typename V> struct from<Value<Tag, V>> { using type = Single<Tag, true, V>; };

/// Defines holder type for KeyValue container types.
template<typename Tag, typename V> struct from<KeyValue<Tag, V>> { using type = Multiple<Tag, V>; };

// Helper variable for inferring holder types from attribute value container types.
template<typename Attr> using from_t = typename from<Attr>::type;


} // namespace traits



template<typename Tag, bool Required, typename V>
template<typename Tagt, typename Vt>
Single<Tag, Required, V> &Single<Tag, Required, V>::operator=(const Value<Tagt, Vt> &)
{
    static_assert(
        std::is_same_v<Tag, Tagt> && std::is_same_v<V, Vt>,
        "Attribute can't be initialized from value of mismatching tag/type");

    return *this;
}

template<typename Tag, bool Required, typename V>
Single<Tag, Required, V> &Single<Tag, Required, V>::operator=(const Value<Tag, V> &value)
{
    d_value = *value;
    return *this;
}

template<typename Tag, bool Required, typename V>
Single<Tag, Required, V> &Single<Tag, Required, V>::operator=(Value<Tag, V> &&value)
{
    d_value = *std::move(value);
    return *this;
}

template<typename Tag, bool Required, typename V>
template<typename Vt>
Single<Tag, Required, V> &Single<Tag, Required, V>::operator=(const KeyValue<Tag, Vt> &)
{
    static_assert(
        std::is_same_v<Tag, Tag *>,
        "Key value pair cannot be assigned to a single value holder");

    return *this;
};

template<typename Tag, bool Required, typename V>
Single<Tag, Required, V>::operator bool() const
{
    return !Required || d_value.has_value();
}

template<typename Tag, bool Required, typename V>
const typename Single<Tag, Required, V>::type &Single<Tag, Required, V>::operator*() const &
{
    return d_value;
}

template<typename Tag, bool Required, typename V>
typename Single<Tag, Required, V>::type &&Single<Tag, Required, V>::operator*() &&
{
    return std::move(d_value);
}


template<typename Tag, typename V>
template<typename Tagt, typename Vt>
Multiple<Tag, V> &Multiple<Tag, V>::operator=(const KeyValue<Tagt, Vt> &)
{
    static_assert(
        std::is_same_v<Tag, Tagt> && std::is_same_v<std::pair<std::string_view, V>, Vt>,
        "Values can't be updated from key-value pair of mismatching tag/key type/value type");

    return *this;
}

template<typename Tag, typename V>
Multiple<Tag, V> &Multiple<Tag, V>::operator=(const KeyValue<Tag> &kv)
{
    if (*kv)
    {
        const auto &[key, value] = **kv;
        d_values[key] = value;
    }

    return *this;
}

template<typename Tag, typename V>
Multiple<Tag, V> &Multiple<Tag, V>::operator=(KeyValue<Tag> &&kv)
{
    if (*kv)
    {
        auto &&[key, value] = **std::move(kv);
        d_values[std::move(key)] = std::move(value);
    }

    return *this;
}

template<typename Tag, typename V>
template<typename Vt>
Multiple<Tag, V> &Multiple<Tag, V>::operator=(const Value<Tag, Vt> &)
{
    static_assert(
        std::is_same_v<Tag, Tag *>,
        "Single value can't be used to update a key-value collection");

    return *this;
}

template<typename Tag, typename V>
Multiple<Tag, V>::operator bool() const
{
    return true;
}

template<typename Tag, typename V>
const typename Multiple<Tag, V>::type &Multiple<Tag, V>::operator*() const &
{
    return d_values;
}

template<typename Tag, typename V>
typename Multiple<Tag, V>::type &&Multiple<Tag, V>::operator*() &&
{
    return std::move(d_values);
}

template<typename Tag, typename V>
typename Multiple<Tag, V>::mapped_type Multiple<Tag, V>::operator()(const key_type &item) const
{
    auto it = d_values.find(item);
    return it == d_values.end() ? mapped_type {} : std::make_optional(std::cref(it->second));
}


} // namespace porter::attr
