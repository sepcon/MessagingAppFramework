#pragma once

#include "thaf/messaging/client-server/CSStatus.h"
#include "thaf/messaging/client-server/Address.h"

namespace thaf {
namespace srz { struct ByteArray; }
namespace messaging {
namespace ipc {

class IPCSender
{
public:
    virtual ~IPCSender() = default;
    virtual void initConnection(const Address&) = 0;
    virtual DataTransmissionErrorCode send(const srz::ByteArray& ba, const Address& destination = Address::INVALID_ADDRESS) = 0;
    virtual Availability checkReceiverStatus() const = 0;
    virtual const Address& receiverAddress() const = 0;
//    virtual void stop() = 0;
};

}// ipc
}// messaging
}// thaf
