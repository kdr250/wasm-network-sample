#pragma once

#include <drogon/PubSubService.h>
#include <drogon/WebSocketController.h>

using namespace drogon;

class EchoWebSocket : public drogon::WebSocketController<EchoWebSocket>
{
public:
    void handleNewMessage(const WebSocketConnectionPtr& connectionPtr,
                          std::string&& message,
                          const WebSocketMessageType& messageType) override;

    void handleNewConnection(const HttpRequestPtr& requestPtr,
                             const WebSocketConnectionPtr& connectionPtr) override;

    void handleConnectionClosed(const WebSocketConnectionPtr& connectionPtr) override;

    WS_PATH_LIST_BEGIN
    // list path definitions here
    WS_PATH_ADD("/echo", Get);
    WS_PATH_LIST_END

private:
    PubSubService<std::string> channels;
};

struct Subscriber
{
    drogon::SubscriberID id;
};
