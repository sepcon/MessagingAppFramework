#pragma once
#include <maf/messaging/client-server/internal/CSShared.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/messaging/client-server/ServiceProviderInterface.h>
#include <maf/threading/Lockable.h>
#include <map>
#include <set>
#include <list>

namespace maf {
namespace messaging {

class ServerInterface;
class ServiceProvider;
class Request;

struct ServiceProviderImpl
{
    template <typename ValueType>
    using OpIDMap                = threading::Lockable<std::map<OpID, ValueType>>;

    using RequestPtrType         = std::shared_ptr<Request>;
    using PropertyPtr            = CSMsgContentBasePtr;
    using RequestMap             = OpIDMap<std::list<RequestPtrType>>;
    using PropertyMap            = OpIDMap<PropertyPtr>;
    using RequestHandlerMap      = OpIDMap<RequestHandlerFunction>;
    using Address2OpIDsMap       = threading::Lockable<std::map<Address, std::set<OpID>>>;

    Address2OpIDsMap                _regEntriesMap;
    RequestMap                      _requestsMap;
    std::weak_ptr<ServerInterface>  _server;
    ServiceProvider*                _delegator;
    PropertyMap                     _propertyMap;
    RequestHandlerMap               _requestHandlerMap;
    std::atomic<Availability>       _availability = Availability::Unavailable;

    ServiceProviderImpl(
        ServiceProvider* holder,
        std::weak_ptr<ServerInterface> server
        );

    ~ServiceProviderImpl();

    ActionCallStatus respondToRequest(const CSMessagePtr &csMsg);

    ActionCallStatus setStatus(
        const OpID& propertyID,
        const CSMsgContentBasePtr& property
        );

    ActionCallStatus broadcastSignal(
        const OpID&  signalID,
        const CSMsgContentBasePtr& signal
        );

    ActionCallStatus broadcast(
        const OpID&  propertyID,
        OpCode opCode,
        const CSMsgContentBasePtr& content
        );

    CSMsgContentBasePtr getStatus(const OpID&  propertyID);

    Availability availability() const;
    void startServing();
    void stopServing();

    bool onIncomingMessage(const CSMessagePtr& msg);

    ActionCallStatus sendMessage(
        const CSMessagePtr &csMsg,
        const Address &toAddr
        );
    ActionCallStatus sendBackMessageToClient(const CSMessagePtr &csMsg);
    void onStatusChangeRegister(const CSMessagePtr& msg);
    void onStatusChangeUnregister(const CSMessagePtr& msg);


    RequestPtrType saveRequestInfo(const CSMessagePtr& msg);
    RequestPtrType pickOutRequestInfo(const CSMessagePtr &msgContent);

    void invalidateAndRemoveAllRequests();


    void saveRegisterInfo(const CSMessagePtr& msg);
    void removeRegisterInfo(const CSMessagePtr& msg);
    void removeAllRegisterInfo();
    void removeRegistersOfAddress(const Address& addr);

    void onAbortActionRequest(const CSMessagePtr& msg);
    void onClientGoesOff(const CSMessagePtr& msg);

    void onActionRequest(const CSMessagePtr& msg);
    void updateLatestStatus(const CSMessagePtr& registerMsg);
    void onStatusGetRequest(const CSMessagePtr &getMsg);
    bool invokeRequestHandlerCallback(const RequestPtrType& request);

    bool registerRequestHandler(
        const OpID&  opID,
        RequestHandlerFunction handlerFunction
        );
    bool unregisterRequestHandler( const OpID&  opID );

};

}
}