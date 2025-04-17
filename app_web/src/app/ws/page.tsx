"use client";

import { WebSocketClient } from "app/libs/websocket/WebSocketClient";
import { WebSocketTopics } from "app/libs/websocket/WebSocketTopics";
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
      ws.subscribe(WebSocketTopics.CHAT, displayMessage);
      setTimeout(() => {
        // Send a message (optional test)
        ws.publish(WebSocketTopics.CHAT, "Yo from client!");
        setTimeout(() => {
          ws.unsubscribe(WebSocketTopics.CHAT);

          // ws.subscribe(WebSocketTopics.CHAT, displayMessage);
          // Send a message (optional test)
          ws.publish(WebSocketTopics.CHAT, "Yo from client 2!");
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
