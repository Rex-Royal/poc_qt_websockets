"use client";

import { useWebSocket } from "app/libs/websocket/WebSocketContext";
import { WebSocketTopic } from "app/libs/websocket/WebSocketTopic";
import { useEffect, useState } from "react";

export default function Home() {
  const [message, setMessage] = useState<string>("");
  const ws = useWebSocket();

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

  return (
    <div>
      <h1>WebSocket Test</h1>
      <p>Message from server: {message}</p>
    </div>
  );
}
