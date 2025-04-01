#include "Network.h"
#include <SDL2/SDL.h>
#include <sstream>
#include <string>
#include "Game.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/websocket.h>
#else
    #include <IXNetSystem.h>
    #include <IXUserAgent.h>
    #include <IXWebSocket.h>
#endif

#ifdef __EMSCRIPTEN__
namespace EMWebSocket
{
    void SendPosition(Game* game, EMSCRIPTEN_WEBSOCKET_T websocket)
    {
        unsigned int id = game->GetId();
        auto& position  = game->GetPosition();

        std::string data = "MoveEvent " + std::to_string(game->GetId()) + " "
                           + std::to_string(position.x) + " " + std::to_string(position.y);

        EMSCRIPTEN_RESULT result = emscripten_websocket_send_utf8_text(websocket, data.c_str());
        if (result)
        {
            SDL_Log("Failed to execute emscripten_websocket_send_utf8_text(): %i", result);
        }
    }

    EM_BOOL OnOpen(int eventType,
                   const EmscriptenWebSocketOpenEvent* websocketEvent,
                   void* userData)
    {
        SDL_Log("WebSocket On Open");
        return EM_TRUE;
    }

    EM_BOOL OnError(int eventType,
                    const EmscriptenWebSocketErrorEvent* websocketEvent,
                    void* userData)
    {
        SDL_Log("WebSocket On Error");
        return EM_TRUE;
    }

    EM_BOOL OnClose(int eventType,
                    const EmscriptenWebSocketCloseEvent* websocketEvent,
                    void* userData)
    {
        SDL_Log("WebSocket On Close");
        return EM_TRUE;
    }

    EM_BOOL OnMessage(int eventType,
                      const EmscriptenWebSocketMessageEvent* websocketEvent,
                      void* userData)
    {
        Game* pGame = reinterpret_cast<Game*>(userData);

        if (websocketEvent->isText)
        {
            const char* data = reinterpret_cast<const char*>(websocketEvent->data);
            std::string strData(data);
            std::stringstream stringStream(strData);

            std::string type;

            stringStream >> type;
            if (type == "RegisterEvent")
            {
                int id;
                stringStream >> id;
                pGame->SetId(id);
            }
            if (type == "SyncEvent")
            {
                SendPosition(pGame, websocketEvent->socket);
            }
            if (type == "MoveEvent")
            {
                int id;
                float x, y;
                stringStream >> id >> x >> y;
                pGame->Receive(id, x, y);
            }
            if (type == "CloseEvent")
            {
                int id;
                stringStream >> id;
                pGame->Remove(id);
            }
        }

        return EM_TRUE;
    }

    void Run(Game* game)
    {
        EmscriptenWebSocketCreateAttributes websocketAttributes = {"ws://127.0.0.1:8080/echo",
                                                                   NULL,
                                                                   EM_FALSE};

        EMSCRIPTEN_WEBSOCKET_T websocket = emscripten_websocket_new(&websocketAttributes);

        emscripten_websocket_set_onopen_callback(websocket, (void*)game, EMWebSocket::OnOpen);
        emscripten_websocket_set_onerror_callback(websocket, (void*)game, EMWebSocket::OnError);
        emscripten_websocket_set_onclose_callback(websocket, (void*)game, EMWebSocket::OnClose);
        emscripten_websocket_set_onmessage_callback(websocket, (void*)game, EMWebSocket::OnMessage);

        // FIXME
        while (game->IsRunning())
        {
            if (game->IsAnyAction()
                && SDL_TICKS_PASSED(SDL_GetTicks64(), game->GetTickCount() + 16))
            {
                SendPosition(game, websocket);
            }
        }

        EMSCRIPTEN_RESULT result = emscripten_websocket_close(websocket, 1000, nullptr);
        if (result)
        {
            SDL_Log("Failed to emscripten_websocket_close(): %d", result);
        }
    }
}  // namespace EMWebSocket
#else
namespace WebSocket
{
    void SendPosition(Game* game, std::shared_ptr<ix::WebSocket> webSocket)
    {
        unsigned int id = game->GetId();
        auto& position  = game->GetPosition();

        std::string data = "MoveEvent " + std::to_string(game->GetId()) + " "
                           + std::to_string(position.x) + " " + std::to_string(position.y);

        auto result = webSocket->send(data);
        if (!result.success)
        {
            SDL_Log("Failed to send data...");
        }
    }

    void Run(Game* game)
    {
        // Required on Windows
        ix::initNetSystem();

        // websockt object
        std::shared_ptr<ix::WebSocket> webSocket = std::make_shared<ix::WebSocket>();

        // Connecto to a server
        std::string url("ws://127.0.0.1:8080/echo");
        webSocket->setUrl(url);

        // Setup a callback fired when a message or an event (open, close, error) is received
        webSocket->setOnMessageCallback(
            [game, webSocket](const ix::WebSocketMessagePtr& message)
            {
                switch (message->type)
                {
                    case ix::WebSocketMessageType::Message:
                    {
                        std::stringstream stringStream(message->str);

                        std::string type;

                        stringStream >> type;
                        if (type == "RegisterEvent")
                        {
                            int id;
                            stringStream >> id;
                            game->SetId(id);
                        }
                        if (type == "SyncEvent")
                        {
                            SendPosition(game, webSocket);
                        }
                        if (type == "MoveEvent")
                        {
                            int id;
                            float x, y;
                            stringStream >> id >> x >> y;
                            game->Receive(id, x, y);
                        }
                        if (type == "CloseEvent")
                        {
                            int id;
                            stringStream >> id;
                            game->Remove(id);
                        }

                        break;
                    }

                    case ix::WebSocketMessageType::Open:
                        SDL_Log("Connection established");
                        break;

                    case ix::WebSocketMessageType::Error:
                        SDL_Log("Connection error: %s", message->str.c_str());
                        break;

                    default:
                        break;
                }
            });

        webSocket->start();

        // FIXME
        while (game->IsRunning())
        {
            if (game->IsAnyAction()
                && SDL_TICKS_PASSED(SDL_GetTicks64(), game->GetTickCount() + 16))
            {
                SendPosition(game, webSocket);
            }
        }

        webSocket->close();
    }
}  // namespace WebSocket
#endif

Network::Network() {}

bool Network::Initialize(Game* game)
{
#ifdef __EMSCRIPTEN__
    if (!emscripten_websocket_is_supported())
    {
        SDL_Log("websocket is not supported...");
        return false;
    }
#endif

    mNetworkThread = std::make_unique<std::thread>(
        [game]()
        {
#ifdef __EMSCRIPTEN__
            EMWebSocket::Run(game);
#else
            WebSocket::Run(game);
#endif
        });

    mNetworkThread->detach();

    return true;
}
