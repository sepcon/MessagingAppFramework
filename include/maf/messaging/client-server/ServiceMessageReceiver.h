#pragma once

#include "CSMessage.h"
#include "CSMessageReceiverIF.h"
#include <functional>

namespace maf {
namespace messaging {

using CSMessageHandlerCallback = std::function<void(const CSMessagePtr &)>;

class ServiceMessageReceiver : public CSMessageReceiverIF {
public:
  const ServiceID &serviceID() const { return _serviceID; }
  void setServiceID(ServiceID serviceID) { _serviceID = std::move(serviceID); }

protected:
  ServiceID _serviceID;
};

} // namespace messaging
} // namespace maf
