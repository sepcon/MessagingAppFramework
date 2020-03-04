#pragma once

#include <vector>
#include <string>
#include <map>

#define MAF_ENABLE_DUMP
#include "maf/messaging/client-server/CSContractDefinesBegin.mc.h"

SERVICE(weather)
    REQUEST(today_weather)
        ALIAS(using StringList = std::vector<std::string>);
        OUTPUT
            (
                (std::string, the_status, "It is going to rain now!"),
                (StringList, list_of_places),
                (StringList, shared_list_of_places)
                )
        INPUT
            (
                (std::string, client_name, "This s client"),
                (uint32_t, command, 0)
            )

    ENDREQUEST(today_weather)

    PROPERTY(simple_status)
        using CustomHeader = std::map<std::string, std::string>;
        using String = std::string;
        STATUS
            (
                (bool,              compliant,           true),
                (bool,              critical,            false),
                (CustomHeader,      headers                   ),
                (String,            cloud_message,       "don't care")
            )
    ENDPROPERTY(simple_status)

    PROPERTY(compliance)
        STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (std::string, cloud_message, "This device is compliant!")
        )
    ENDPROPERTY(compliance)

ENDSERVICE(weather);

#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>