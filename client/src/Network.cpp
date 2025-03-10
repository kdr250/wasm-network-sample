#include "Network.h"
#include <SDL2/SDL.h>
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
    EM_BOOL OnOpen(int eventType,
                   const EmscriptenWebSocketOpenEvent* websocketEvent,
                   void* userData)
    {
        SDL_Log("On Open");

        EMSCRIPTEN_RESULT result =
            emscripten_websocket_send_utf8_text(websocketEvent->socket, "Hello");
        if (result)
        {
            SDL_Log("Failed to execute emscripten_websocket_send_utf8_text(): %i", result);
        }
        return EM_TRUE;
    }

    EM_BOOL OnError(int eventType,
                    const EmscriptenWebSocketErrorEvent* websocketEvent,
                    void* userData)
    {
        SDL_Log("On Error");
        return EM_TRUE;
    }

    EM_BOOL OnClose(int eventType,
                    const EmscriptenWebSocketCloseEvent* websocketEvent,
                    void* userData)
    {
        SDL_Log("On Close");
        return EM_TRUE;
    }

    EM_BOOL OnMessage(int eventType,
                      const EmscriptenWebSocketMessageEvent* websocketEvent,
                      void* userData)
    {
        SDL_Log("On Message");

        if (websocketEvent->isText)
        {
            SDL_Log("message: %s", websocketEvent->data);  // for only ascii chars
        }

        return EM_TRUE;
    }

    void Run(bool* isRunning)
    {
        EmscriptenWebSocketCreateAttributes websocketAttributes = {"ws://127.0.0.1:8080/echo",
                                                                   NULL,
                                                                   EM_TRUE};

        EMSCRIPTEN_WEBSOCKET_T websocket = emscripten_websocket_new(&websocketAttributes);

        emscripten_websocket_set_onopen_callback(websocket, NULL, EMWebSocket::OnOpen);
        emscripten_websocket_set_onerror_callback(websocket, NULL, EMWebSocket::OnError);
        emscripten_websocket_set_onclose_callback(websocket, NULL, EMWebSocket::OnClose);
        emscripten_websocket_set_onmessage_callback(websocket, NULL, EMWebSocket::OnMessage);

        // FIXME
        while (*isRunning)
            ;

        EMSCRIPTEN_RESULT result;
        result = emscripten_websocket_close(websocket, 1000, "clinet shutdown!");
        if (result)
        {
            SDL_Log("Failed to emscripten_websocket_close(): %d", result);
        }
    }
}  // namespace EMWebSocket
#else
namespace WebSocket
{
    void Run(bool* isRunning)
    {
        // Required on Windows
        ix::initNetSystem();

        // websockt object
        ix::WebSocket webSocket;

        // Connecto to a server
        std::string url("ws://127.0.0.1:8080/echo");
        webSocket.setUrl(url);

        // Setup a callback fired when a message or an event (open, close, error) is received
        webSocket.setOnMessageCallback(
            [](const ix::WebSocketMessagePtr& message)
            {
                switch (message->type)
                {
                    case ix::WebSocketMessageType::Message:
                        SDL_Log("received message: %s", message->str.c_str());
                        break;

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

        webSocket.start();

        // FIXME
        while (*isRunning)
            ;
    }
}  // namespace WebSocket
#endif

Network::Network() {}

bool Network::Initialize(bool* isRunning)
{
#ifdef __EMSCRIPTEN__
    if (!emscripten_websocket_is_supported())
    {
        SDL_Log("websocket is not supported...");
        return false;
    }
#endif

    mNetworkThread = std::make_unique<std::thread>(
        [isRunning]()
        {
#ifdef __EMSCRIPTEN__
            EMWebSocket::Run(isRunning);
#else
            WebSocket::Run(isRunning);
#endif
        });

    mNetworkThread->detach();

    return true;
}
