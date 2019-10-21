#pragma once

#ifdef MAF_ENABLE_DUMP
#   pragma push_macro("MAF_ENABLE_DUMP")
#   define maf_restore_macro_ADDRESS_H_MAF_ENABLE_DUMP
#endif

#define MAF_ENABLE_DUMP
#include <maf/utils/serialization/MafObjectBegin.mc.h>

namespace maf {
namespace messaging {

OBJECT(Address)
    public:
        using Port = int32_t;
        using Name = std::string;
        static constexpr Port INVALID_PORT = -1;
        static const Name INVALID_NAME;
        static const Address INVALID_ADDRESS;
        bool valid() const { return (port() != INVALID_PORT) || (name() != INVALID_NAME);}

    PROPERTIES
        (
            (Name, name, INVALID_NAME),
            (Port, port, INVALID_PORT)
        )

ENDOBJECT(Address)

}// messaging
}// maf

#include <maf/utils/serialization/MafObjectEnd.mc.h>
#undef MAF_ENABLE_DUMP

#ifdef maf_restore_macro_ADDRESS_H_MAF_ENABLE_DUMP
#   undef maf_restore_macro_ADDRESS_H_MAF_ENABLE_DUMP
#   pragma pop_macro("MAF_ENABLE_DUMP")
#endif
