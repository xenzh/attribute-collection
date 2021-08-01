#pragma once

#include <type_traits>
#include <optional>
#include <utility>
#include <string_view>


namespace porter::attr {
namespace traits {


/// Template that checks whether a tag type is valid.
template<typename Tag, typename = void> struct is_tag_valid : std::false_type {};

/// Template that checks whether a tag type is valid.
/// Valid tags are expected to contain static std::string_view value member.
template<typename Tag>
struct is_tag_valid<Tag, std::enable_if_t<
    std::is_same_v<const std::string_view, decltype(Tag::value)>,
    std::void_t<typename Tag::type>>>
    : std::true_type {};

/// Helper variable template for checking whether a tag type is valid.
template<typename Tag> inline constexpr bool is_tag_valid_v = is_tag_valid<Tag>::value;


/// Template that defines tag type for any valid attrbute type.
template<typename T> struct tag_of
{
    static_assert(std::is_same_v<T, T *>, "Not a valid attribute type");
};

/// Helper alias for getting tag type from any valid attribute type.
template<typename T> using tag_of_t = typename tag_of<T>::type;


} // namespace traits


/// Describes a typed optional value for a given attribute (tag).
/// @tparam Tag tag type that defines attribute properties.
/// @tparam V attribute value type. Normally auto-deduced.
template<typename Tag, typename V = typename Tag::type>
class Value
{
    static_assert(traits::is_tag_valid_v<Tag>, "Tag type is invalid");

    /// Stores attribute value.
    std::optional<V> d_value;

public:
    /// Construct instance from attribute value.
    /// @tparam Vt type, convertible to attribute value type.
    /// @param value initial attribute value.
    template<typename Vt>
    explicit Value(Vt &&value);

    /// Default copy ctor.
    Value(const Value<Tag, V> &) = default;

    /// Default move ctor.
    Value(Value<Tag, V> &&) = default;

    /// Default copy assignment.
    Value<Tag, V> &operator=(const Value<Tag, V> &) = default;

    /// Default move assignment.
    Value<Tag, V> &operator=(Value<Tag, V> &&) = default;

    /// Update stored value.
    /// @tparam Vt type, convertible to attribute value type.
    /// @param value new attribute value (overwrites the old one, if any).
    /// @return reference to self.
    template<typename Vt>
    Value<Tag, V> &operator=(Vt &&value);

    /// Get stored value.
    /// @return const reference to stored value.
    const std::optional<V> &operator*() const &;

    /// Get stored value.
    /// @return rvalue reference to stored value.
    std::optional<V> &&operator*() &&;
};


/// Describes a named typed optional value for a given attribute (tag).
/// @tparam Tag tag type that defines attribute properties.
/// @tparam V attribute value type. Normally auto-deduced.
template<typename Tag, typename V = std::pair<std::string_view, typename Tag::type>>
class KeyValue : public Value<Tag, V>
{
public:
    /// Construct instance from name and value.
    /// @tparam Kt attribute name type.
    /// @tparam Vt value type, convertible to the attribute value type.
    /// @param key attribute name.
    /// @param value attribute value.
    template<typename Kt, typename Vt>
    explicit KeyValue(Kt &&key, Vt &&value);
};


namespace traits {


/// Defines tag type for Value container.
template<typename Tag, typename V> struct tag_of<Value<Tag, V>> { using type = Tag; };

/// Defines tag type for KeyValue container.
template<typename Tag, typename V> struct tag_of<KeyValue<Tag, V>> { using type = Tag; };


} // namespace traits



template<typename Tag, typename V>
template<typename Vt>
Value<Tag, V>::Value(Vt &&value)
    : d_value(std::forward<Vt>(value))
{
}

template<typename Tag, typename V>
template<typename Vt>
Value<Tag, V> &Value<Tag, V>::operator=(Vt &&value)
{
    d_value.reset(std::forward<Vt>(value));
    return *this;
}

template<typename Tag, typename V>
const std::optional<V> &Value<Tag, V>::operator*() const &
{
    return d_value;
}

template<typename Tag, typename V>
std::optional<V> &&Value<Tag, V>::operator*() &&
{
    return std::move(d_value);
}


template<typename Tag, typename V>
template<typename Kt, typename Vt>
KeyValue<Tag, V>::KeyValue(Kt &&key, Vt &&value)
    : Value<Tag, V> {std::make_pair(std::forward<Kt>(key), std::forward<Vt>(value))}
{
}


} // namespace porter::attr
