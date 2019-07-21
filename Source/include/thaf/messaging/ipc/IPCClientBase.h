#pragma once

#include "thaf/messaging/client-server/ClientBase.h"
#include "BytesCommunicator.h"
#include "IPCTypes.h"
#include <thread>

namespace thaf {
namespace messaging {
namespace ipc {

class IPCClientBase : public ClientBase, public BytesCommunicator
{
public:
    IPCClientBase();
    void init(IPCType type, const Address& serverAddress);
    ~IPCClientBase() override;
    DataTransmissionErrorCode sendMessageToServer(const CSMessagePtr& msg)  override;
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;

protected:
    void monitorServerStatus();
    std::thread _serverMonitorThread;
    std::atomic_bool _stop;
};


} // ipc
} // messaging
} // thaf