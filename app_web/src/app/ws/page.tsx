"use client";

import { WebSocketClient } from "app/libs/websocket/WebSocketClient";
import { WebSocketTopic } from "app/libs/websocket/WebSocketTopic";
import { useEffect, useState } from "react";
import { RequestBuilder } from "ts-request-builder";

export default function Home() {
  const [wsApi, setWsApi] = useState<string | null>(null);
  const [message, setMessage] = useState<string>("");
  const [ws, setWs] = useState<WebSocketClient | null>(null);

  const triggerServerWebsocket = () =>
    new RequestBuilder("/api/websocket")
      .withErrorHandling((err, status, statusText) =>
        console.log("ERR: ", err, status, statusText)
      )
      .buildAsJson<{ url: string }>();

  // Initialize Websocket
  const init = async () => setWsApi((await triggerServerWebsocket()).url);

  useEffect(() => {
    if (wsApi) startWebSocket();
  }, [wsApi]);

  const startWebSocket = () => {
    const ws = WebSocketClient.getInstance(`${wsApi}/api/websocket`);
    ws.connect();
    ws.setReconnectionLogic(triggerServerWebsocket);
    setTimeout(() => setWs(ws), 0);

    return ws.close;
  };

  const displayMessage = (msg: string) => {
    setMessage(msg);
    console.log("📩 Message from server:", msg);
  };

  useEffect(() => {
    if (ws) {
      ws.subscribe(WebSocketTopic.CHAT, displayMessage);
      setTimeout(() => {
        // Send a message (optional test)
        ws.publish(WebSocketTopic.CHAT, "Yo from client!");
        setTimeout(() => {
          ws.unsubscribe(WebSocketTopic.CHAT);

          ws.subscribe(WebSocketTopic.CHAT, displayMessage);
          // Send a message (optional test)
          ws.publish(WebSocketTopic.CHAT, "Yo from client 2!");
        }, 1000);
      }, 1000);
    }
  }, [ws]);

  useEffect(() => {
    init();
  }, []);

  return (
    <div>
      <h1>WebSocket Test</h1>
      <p>Message from server: {message}</p>
    </div>
  );
}
