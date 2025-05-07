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
import {
  WebSocket_API,
  WS_DOMAIN,
  WS_PORT,
  WS_PROTOCOL,
} from "./WebSocketConf";
import { WebSocketTopic } from "./WebSocketTopic";
import { WebsocketActions } from "./WebSocketActions";

export type OnSocketMessageHander = (
  action: WebsocketActions,
  topic: WebSocketTopic,
  payload?: string
) => void;

interface WebSocketContextType {
  publish: (topic: WebSocketTopic, message: string) => void;
  subscribe: (topic: WebSocketTopic, handler: OnSocketMessageHander) => void;
  unsubscribe: (topic: WebSocketTopic) => void;
  clear: () => void;
}
const WebSocketContext = createContext<WebSocketContextType | null>(null);

export function useWebSocket() {
  const context = useContext(WebSocketContext);
  if (!context)
    throw new Error("useWebSocket must be used inside WebSocketProvider");
  return context;
}

export function WebSocketProvider({
  children,
  display,
}: {
  children: ReactNode;
  display?: OnSocketMessageHander;
}) {
  const ws = useRef<WebSocketClient | null>(null);
  const handlers = useRef<Record<string, OnSocketMessageHander>>({});

  const [wsApi, setWsApi] = useState("");

  const init = async () => {
    // const wsApi = await new RequestBuilder(WebSocket_API)
    //   .withErrorHandling((err, status, statusText) =>
    //     console.log("ERR: ", err, status, statusText)
    //   )
    //   .buildAsJson<{ url: string }>();
    // setWsApi(wsApi.url);
    setWsApi(`${WS_PROTOCOL}://${WS_DOMAIN}:${WS_PORT}`);
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

  const publish = (topic: WebSocketTopic, payload: string) => {
    if (
      ws.current &&
      ws.current.isConnected() &&
      ws.current.readyState() === WebSocket.OPEN
    ) {
      ws.current.publish(topic, payload);
      display?.(WebsocketActions.PUBLISH, topic, payload);
    }
  };

  const subscribe = (topic: WebSocketTopic, handler: OnSocketMessageHander) => {
    handlers.current[topic] = handler;
    ws.current?.subscribe(topic, handler);
    display?.(WebsocketActions.SUBSCRIBE, topic);
  };

  const unsubscribe = (topic: WebSocketTopic) => {
    delete handlers.current[topic];
    ws.current?.unsubscribe(topic);
    display?.(WebsocketActions.UNSUBSCRIBE, topic);
  };

  const clear = () => ws.current?.clear();

  return (
    <WebSocketContext.Provider
      value={{ publish, subscribe, unsubscribe, clear }}
    >
      {children}
    </WebSocketContext.Provider>
  );
}
