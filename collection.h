#pragma once

#include "attribute.h"
#include "holder.h"

#include <tuple>
#include <utility>


namespace porter::attr {
namespace traits {


/// Template that checks whether a type is contained within provided variadic pack.
template<typename T, typename... Hs> struct contains : std::false_type {};

/// Template that checks whether a type is contained within provided variadic pack.
/// Defines the check for a unit variadic pack.
template<typename T, typename H> struct contains<T, H> : std::is_same<T, H> {};

/// Template that checks whether a type is contained within provided variadic pack.
/// Defines the check for non-unit variadic packs.
template<typename T, typename H1, typename... Hs> struct contains<T, H1, Hs...>
{
    static constexpr bool value = contains<T, H1>::value || contains<T, Hs...>::value;
};

/// Helper variable for checking whether a type is contained within a variadic pack.
template<typename T, typename... Hs> inline constexpr bool contains_v = contains<T, Hs...>::value;


/// Template that checks whether all holder types in a pack are valid.
template<typename... Hs> struct are_holders_valid : std::true_type {};

/// Template that checks whether all holder types in a pack are valid.
/// Defines the check for non-empty packs.
template<typename H, typename... Hs> struct are_holders_valid<H, Hs...>
{
    static constexpr bool value = is_valid_v<H> && !contains_v<H, Hs...> && are_holders_valid<Hs...>::value;
};

/// Helper variable for checking whether all holder types in a pacj are valid.
template<typename... Hs> inline constexpr bool are_holders_valid_v = are_holders_valid<Hs...>::value;


/// Helper template that looks for a holder in a pack by a tag.
template<typename Tag, typename... Hs> struct by_tag_impl { using type = void; };

/// Helper template that looks for a holder in a pack by a tag.
/// Defines lookup for non-empty packs.
template<typename Tag, typename H, typename... Hs> struct by_tag_impl<Tag, H, Hs...>
{
    using type = std::conditional_t<
        std::is_same_v<Tag, traits::tag_of_t<H>>,
        H,
        typename by_tag_impl<Tag, Hs...>::type>;
};

/// Template that looks for a holder in a pack by a tag.
/// Adds a static assertion for failed lookups.
template<typename Tag, typename... Hs> struct by_tag
{
    using type = typename by_tag_impl<Tag, Hs...>::type;

    static_assert(!std::is_same_v<void, type>, "Tagged type was not found");
};

/// Helper variable for looking for a holder in a pack by a tag.
template<typename Tag, typename... Hs> using by_tag_t = typename by_tag<Tag, Hs...>::type;


/// Template that extends a type with a pack of holders, ignoring duplicates.
template<typename T, typename... New> struct extend;

/// Template that extends a type with a pack of holders, ignoring duplicates.
/// Defines the operation for empty pack of holders.
template<template<typename... > typename T, typename... Hs> struct extend<T<Hs...>> { using type = T<Hs...>; };

/// Template that extends a type with a pack of holders, ignoring duplicates.
/// Defines the operation for a unit pack of holders.
template<template<typename...> typename T, typename... Hs, typename New> struct extend<T<Hs...>, New>
{
    using type = std::conditional_t<contains_v<New, Hs...>, T<Hs...>, T<New, Hs...>>;
};

/// Template that extends a type with a pack of holders, ignoring duplicates.
/// Defines the operation for a non-unit pack of holders.
template<template<typename...> typename T, typename... Hs, typename N, typename... R> struct extend<T<Hs...>, N, R...>
{
    using type = typename extend<typename extend<T<Hs...>, N>::type, R...>::type;
};

/// Helper variable for extending a type with a pack of holders, ignoring duplicates.
template<typename T, typename... New> using extend_t = typename extend<T, New...>::type;


} // namespace traits


/// Defines a collection of unique attribute values.
/// Provides strictly typed interfaces for value access and collection manipulation.
/// @tparam Holders pack of attribute holder types.
template<typename... Holders>
class Collection
{
    /// Allows access to collections of different sets of attributes.
    template<typename... Hs> friend class Collection;

    static_assert(traits::are_holders_valid_v<Holders...>, "One or more holder types are not valid");

    /// Stores attribute value holders.
    std::tuple<Holders...> d_holders;

private:
    /// Update (copy) stored attribute value from another collection, if present.
    /// @tparam Current attribute holder type to update.
    /// @tparam Others attribute holder types of source collection.
    /// @param other const reference to source collection.
    template<typename Current, typename... Others>
    void assign(const Collection<Others...> &other);

    /// Update (move) stored attribute value from another collection, if present.
    /// @tparam Current attribute holder type to update.
    /// @tparam Others attribute holder types of source collection.
    /// @param other rvalue reference to source collection.
    template<typename Current, typename... Others>
    void assign(Collection<Others...> &&other);

    /// Check whether all contained attributes are in valid state.
    /// @tparam Idx sequence of pack indices, auto-deduced.
    /// @return true if all attributes are properly set.
    template<size_t... Idx>
    bool ready(std::index_sequence<Idx...>) const;

public:
    /// Default ctor.
    Collection() = default;

    /// Default copy ctor.
    Collection(const Collection &) = default;

    /// Default move ctor.
    Collection(Collection &&) = default;

    /// Default copy assignment.
    Collection<Holders...> &operator=(const Collection &) = default;

    /// Default move assignment.
    Collection<Holders...> &operator=(Collection &&) = default;

    /// Construct a collection from another by copying common attribute values.
    /// @tparam Others attribute holder types of source collection.
    /// @param other source collection.
    template<typename... Others>
    explicit Collection(const Collection<Others...> &other);

    /// Construct a collection from another by moving common attribute values.
    /// @tparam Others attribute holder types of source collection.
    /// @param other source collection.
    template<typename... Others>
    explicit Collection(Collection<Others...> &&other);

    /// Copy-assign common attribute values from another collection.
    /// @tparam Others attribute holder types of source collection.
    /// @param other source collection.
    /// @return reference to self.
    template<typename... Others>
    Collection<Holders...> &operator=(const Collection<Others...> &other);

    /// Move-assign common attribute values from another collection.
    /// @tparam Others attribute holder types of source collection.
    /// @param other source collection.
    /// @return reference to self.
    template<typename... Others>
    Collection<Holders...> &operator=(Collection<Others...> &&other);

    /// Update (copy) attribute value from Value container.
    /// @tparam Tag attribute tag type.
    /// @tparam V attribute value type.
    /// @param value attribute value container.
    /// @return reference to self.
    template<typename Tag, typename V>
    Collection<Holders...> &operator<<(const Value<Tag, V> &value);

    /// Update (move) attribute value from Value container.
    /// @tparam Tag attribute tag type.
    /// @tparam V attribute value type.
    /// @param value attribute value container.
    /// @return reference to self.
    template<typename Tag, typename V>
    Collection<Holders...> &operator<<(Value<Tag, V> &&value);

    /// Update (copy) attribute value from KeyValue container.
    /// @tparam Tag attribute tag type.
    /// @tparam V attribute value type.
    /// @param kv attribute value container.
    /// @return reference to self.
    template<typename Tag, typename V>
    Collection<Holders...> &operator<<(const KeyValue<Tag, V> &kv);

    /// Update (move) attribute value from KeyValue container.
    /// @tparam Tag attribute tag type.
    /// @tparam V attribute value type.
    /// @param kv attribute value container.
    /// @return reference to self.
    template<typename Tag, typename V>
    Collection<Holders...> &operator<<(KeyValue<Tag, V> &&kv);

    /// Construct a new collection by merging this one and a pack of attribute value containers.
    /// For duplicates, new attribute values take priority.
    /// @tparam New pack of attribute value containers.
    /// @param attributes pack of attribute value containers.
    /// @return new extended attribute value collection.
    template<typename... New>
    traits::extend_t<Collection<Holders...>, traits::from_t<New>...> extend(New &&...attributes) &&;

    /// Checks whether all attribute values are properly set.
    /// @return true if all attribute values are properly set.
    operator bool() const;

    /// Get attribute value or value storage.
    /// @tparam Tag attribute tag type.
    /// @tparam H attribute holder type, auto-deduced.
    /// @return attribute value or value storage reference.
    template<typename Tag, typename H = typename traits::by_tag_t<Tag, Holders...>>
    const typename H::type &operator()(Tag) const;

    /// Get named attribute value for associative attributes (i.e. Multiple holder).
    /// @tparam Tag attribute tag type.
    /// @tparam H attribute holder type, auto-deduced.
    /// @param key attribute value name.
    /// @return attribute value reference or std::nullopt, if not found.
    template<
        typename Tag,
        typename H = traits::by_tag_t<Tag, Holders...>,
        typename = std::void_t<typename H::key_type>>
    typename H::mapped_type operator()(Tag, const typename H::key_type &key) const;
};


/// Define addition for collections and attribute containers.
template<typename... Holders, typename Tag, typename V>
auto operator+(Collection<Holders...> &&collection, Value<Tag, V> &&attribute)
{
    return std::move(collection).extend(std::move(attribute));
}

/// Define addition for collections and attribute containers.
template<typename Tag, typename V, typename... Holders>
auto operator+(Value<Tag, V> &&attribute, Collection<Holders...> &&collection)
{
    return std::move(collection) + std::move(attribute);
}

/// Define addition for attribute containers.
template<typename Tagl, typename Vl, typename Tagr, typename Vr>
auto operator+(Value<Tagl, Vl> &&lhs, Value<Tagr, Vr> &&rhs)
{
    return (Collection<> {} + std::move(lhs)) + std::move(rhs);
}

/// Define addition for collections and attribute containers.
template<typename... Holders, typename Tag, typename V>
auto operator+(Collection<Holders...> &&collection, KeyValue<Tag, V> &&attribute)
{
    return std::move(collection).extend(std::move(attribute));
}

/// Define addition for collections and attribute containers.
template<typename Tag, typename V, typename... Holders>
auto operator+(KeyValue<Tag, V> &&attribute, Collection<Holders...> &&collection)
{
    return std::move(collection) + std::move(attribute);
}

/// Define addition for attribute containers.
template<typename Tagl, typename Vl, typename Tagr, typename Vr>
auto operator+(KeyValue<Tagl, Vl> &&lhs, KeyValue<Tagr, Vr> &&rhs)
{
    return (Collection<> {} + std::move(lhs)) + std::move(rhs);
}

/// Define addition for attribute containers.
template<typename Tagl, typename Vl, typename Tagr, typename Vr>
auto operator+(Value<Tagl, Vl> &&lhs, KeyValue<Tagr, Vr> &&rhs)
{
    return (Collection<> {} + std::move(lhs)) + std::move(rhs);
}

/// Define addition for attribute containers.
template<typename Tagl, typename Vl, typename Tagr, typename Vr>
auto operator+(KeyValue<Tagl, Vl> &&lhs, Value<Tagr, Vr> &&rhs)
{
    return std::move(rhs) + std::move(lhs);
}



template<typename... Holders>
template<typename Current, typename... Others>
void Collection<Holders...>::assign(const Collection<Others...> &other)
{
    if constexpr (traits::contains_v<Current, Others...>)
    {
        std::get<Current>(d_holders) = std::get<Current>(other.d_holders);
    }
}

template<typename... Holders>
template<typename Current, typename... Others>
void Collection<Holders...>::assign(Collection<Others...> &&other)
{
    if constexpr (traits::contains_v<Current, Others...>)
    {
        std::get<Current>(d_holders) = std::get<Current>(std::move(other.d_holders));
    }
}

template<typename... Holders>
template<size_t... Idx> bool Collection<Holders...>::ready(std::index_sequence<Idx...>) const
{
    return (true && ... && bool(std::get<Idx>(d_holders)));
}


template<typename... Holders>
template<typename... Others>
Collection<Holders...>::Collection(const Collection<Others...> &other)
{
    (assign<Holders>(other), ...);
}

template<typename... Holders>
template<typename... Others>
Collection<Holders...>::Collection(Collection<Others...> &&other)
{
    (assign<Holders>(std::move(other)), ...);
}

template<typename... Holders>
template<typename... Others>
Collection<Holders...> &Collection<Holders...>::operator=(const Collection<Others...> &other)
{
    (assign<Holders>(other), ...);
}

template<typename... Holders>
template<typename... Others>
Collection<Holders...> &Collection<Holders...>::operator=(Collection<Others...> &&other)
{
    (assign<Holders>(std::move(other)), ...);
}

template<typename... Holders>
template<typename Tag, typename V>
Collection<Holders...> &Collection<Holders...>::operator<<(const Value<Tag, V> &value)
{
    std::get<traits::by_tag_t<Tag, Holders...>>(d_holders) = value;
    return *this;
}

template<typename... Holders>
template<typename Tag, typename V>
Collection<Holders...> &Collection<Holders...>::operator<<(Value<Tag, V> &&value)
{
    std::get<traits::by_tag_t<Tag, Holders...>>(d_holders) = std::move(value);
    return *this;
}

template<typename... Holders>
template<typename Tag, typename V>
Collection<Holders...> &Collection<Holders...>::operator<<(const KeyValue<Tag, V> &kv)
{
    std::get<traits::by_tag_t<Tag, Holders...>>(d_holders) = kv;
    return *this;
}

template<typename... Holders>
template<typename Tag, typename V>
Collection<Holders...> &Collection<Holders...>::operator<<(KeyValue<Tag, V> &&kv)
{
    std::get<traits::by_tag_t<Tag, Holders...>>(d_holders) = std::move(kv);
    return *this;
}

template<typename... Holders>
template<typename... New>
traits::extend_t<Collection<Holders...>, traits::from_t<New>...> Collection<Holders...>::extend(New &&...attributes) &&
{
    traits::extend_t<Collection<Holders...>, traits::from_t<New>...> extended {std::move(*this)};
    (extended << ... << std::forward<New>(attributes));

    return extended;
}

template<typename... Holders>
Collection<Holders...>::operator bool() const
{
    return ready(std::index_sequence_for<Holders...> {});
}

template<typename... Holders>
template<typename Tag, typename H>
const typename H::type &Collection<Holders...>::operator()(Tag) const
{
    return *std::get<traits::by_tag_t<Tag, Holders...>>(d_holders);
}

template<typename... Holders>
template<typename Tag, typename H, typename>
typename H::mapped_type Collection<Holders...>::operator()(Tag, const typename H::key_type &key) const
{
    return std::get<traits::by_tag_t<Tag, Holders...>>(d_holders)(key);
}


} // namespace porter::attr
