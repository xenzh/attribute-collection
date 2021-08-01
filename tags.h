#pragma once

#include <string_view>
#include <string>

#include "attribute.h"


namespace porter {


#define TagAndAttr(Tag, Ty, Kind, Attr) \
namespace tag { \
struct Tag \
{ \
    using type = Ty; \
    static constexpr std::string_view value = #Tag; \
}; \
} \
using Attr = Kind<tag::Tag>;


TagAndAttr(id       , std::string_view, attr::Value   , Id       );
TagAndAttr(service  , std::string_view, attr::Value   , Service  );
TagAndAttr(subsystem, std::string_view, attr::Value   , Subsystem);
TagAndAttr(context  , std::string     , attr::KeyValue, Context  );
TagAndAttr(pwho     , uint32_t        , attr::Value   , Pwho     );
TagAndAttr(label    , uint32_t        , attr::Value   , Label    );

#undef TagType


} // namespace porter
