#pragma once

#include <maf/messaging/client-server/ipc/IPCSender.h>


namespace maf {
namespace messaging {
namespace ipc {

class NamedPipeSender;

class LocalIPCSender : public IPCSender
{
public:
    LocalIPCSender();
    ~LocalIPCSender() override;
    bool initConnection(const Address &addr) override;
    ActionCallStatus send(const maf::srz::ByteArray &ba, const Address& destination = {}) override;
    const Address &receiverAddress() const override;
    Availability checkReceiverStatus() const override;

private:
    std::unique_ptr<IPCSender> _pImpl;
};

} // ipc
} // messaging
} // maf
