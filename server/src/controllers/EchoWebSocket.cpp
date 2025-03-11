#include "EchoWebSocket.h"
#include <string>

void EchoWebSocket::handleNewMessage(const WebSocketConnectionPtr& connectionPtr,
                                     std::string&& message,
                                     const WebSocketMessageType& messageType)
{
    // write your application logic here
    LOG_DEBUG << "new websocket message:" << message;

    switch (messageType)
    {
        case WebSocketMessageType::Ping:
            LOG_DEBUG << "receive a ping!";
            break;

        case WebSocketMessageType::Text:
        {
            auto& subscriber = connectionPtr->getContextRef<Subscriber>();
            channels.publish("testTopicName", message);
            break;
        }

        default:
            break;
    }
}

void EchoWebSocket::handleNewConnection(const HttpRequestPtr& requestPtr,
                                        const WebSocketConnectionPtr& connectionPtr)
{
    // write your application logic here
    LOG_DEBUG << "New WebSocket connection!";

    Subscriber subscriber;
    subscriber.id =
        channels.subscribe("testTopicName",
                           [connectionPtr](const std::string& topic, const std::string& message)
                           {
                               connectionPtr->send(message);
                           });

    connectionPtr->setContext(std::make_shared<Subscriber>(std::move(subscriber)));

    connectionPtr->send("RegisterEvent " + std::to_string(subscriber.id));

    channels.publish("testTopicName", "SyncEvent");
}

void EchoWebSocket::handleConnectionClosed(const WebSocketConnectionPtr& connectionPtr)
{
    // write your application logic here
    LOG_DEBUG << "WebSocket closed!";
    auto& subscriber = connectionPtr->getContextRef<Subscriber>();
    channels.unsubscribe("testTopicName", subscriber.id);

    channels.publish("testTopicName", "CloseEvent " + std::to_string(subscriber.id));
}
