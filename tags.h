#pragma once

#include <string_view>
#include <string>

#include "attribute.h"


namespace porter {


#define TagAndAttr(Tag, TagTy, Ty, Kind, Attr) \
namespace tag { \
struct TagTy \
{ \
    using type = Ty; \
    static constexpr std::string_view value = #Tag; \
}; \
inline constexpr TagTy Tag; \
} \
using Attr = Kind<tag::TagTy>;


TagAndAttr(id       , id_t       , std::string_view, attr::Value   , Id       );
TagAndAttr(service  , service_t  , std::string_view, attr::Value   , Service  );
TagAndAttr(subsystem, subsystem_t, std::string_view, attr::Value   , Subsystem);
TagAndAttr(context  , context_t  , std::string     , attr::KeyValue, Context  );
TagAndAttr(pwho     , pwho_t     , uint32_t        , attr::Value   , Pwho     );
TagAndAttr(label    , label_t    , uint32_t        , attr::Value   , Label    );

#undef TagAndAttr


} // namespace porter
