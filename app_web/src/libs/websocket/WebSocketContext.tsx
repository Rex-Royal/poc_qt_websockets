"use client";

import {
  createContext,
  useEffect,
  useRef,
  ReactNode,
  useContext,
  useState,
} from "react";

import { WebSocketClient } from "./WebSocketClient";
import { RequestBuilder } from "ts-request-builder";

type MessageHandler = (message: string) => void;

interface WebSocketContextType {
  publish: (topic: string, message: string) => void;
  subscribe: (topic: string, handler: MessageHandler) => void;
  unsubscribe: (topic: string) => void;
}
const WebSocketContext = createContext<WebSocketContextType | null>(null);

export function useWebSocket() {
  const context = useContext(WebSocketContext);
  if (!context)
    throw new Error("useWebSocket must be used inside WebSocketProvider");
  return context;
}

export function WebSocketProvider({ children }: { children: ReactNode }) {
  const ws = useRef<WebSocketClient | null>(null);
  const handlers = useRef<Record<string, MessageHandler>>({});

  const [wsApi, setWsApi] = useState("");

  const init = async () => {
    const wsApi = await new RequestBuilder("/api/websocket")
      .withErrorHandling((err, status, statusText) =>
        console.log("ERR: ", err, status, statusText)
      )
      .buildAsJson<{ url: string }>();
    setWsApi(wsApi.url);
  };

  useEffect(() => {
    if (wsApi) {
      ws.current = WebSocketClient.getInstance(`${wsApi}`);
      console.log("WS: ", wsApi, ws.current);
      WebSocketClient.getInstance().connect();
    }
  }, [wsApi]);

  useEffect(() => {
    init();
  }, []);

  const publish = (topic: string, payload: string) => {
    if (
      ws.current &&
      ws.current.isConnected() &&
      ws.current.readyState() === WebSocket.OPEN
    ) {
      ws.current.publish(topic, payload);
    }
  };

  const subscribe = (topic: string, handler: MessageHandler) => {
    handlers.current[topic] = handler;
    ws.current?.subscribe(topic, handler);
  };

  const unsubscribe = (topic: string) => {
    delete handlers.current[topic];
    ws.current?.unsubscribe(topic);
  };

  return (
    <WebSocketContext.Provider value={{ publish, subscribe, unsubscribe }}>
      {children}
    </WebSocketContext.Provider>
  );
}
