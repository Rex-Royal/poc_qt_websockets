"use client";

import { v4 } from "uuid";
import { RequestBuilder } from "ts-request-builder";
import { useCallback, useEffect, useRef, useState } from "react";

import { GuiCommands } from "app/components/GuiCommands";
import {
  OnSocketMessageHander,
  useWebSocket,
  WebSocketProvider,
} from "app/libs/websocket/WebSocketContext";
import { WebSocketTopic } from "app/libs/websocket/WebSocketTopic";
import { WebSocket_API } from "app/libs/websocket/WebSocketConf";
import { WebsocketActions } from "app/libs/websocket/WebSocketActions";

type Message = {
  id: string;
  action: WebsocketActions;
  topic: WebSocketTopic;
  payload?: string;
};

const WS = WebSocketTopic;

const MAX_QUEUE = 20;

export default function Home() {
  const [cmMessageList, setCmMessageList] = useState<Message[]>([]);
  const [guiMessageList, setGuiMessageList] = useState<Message[]>([]);
  const [message, setMessage] = useState<Message>();

  const display: OnSocketMessageHander = useCallback(
    (action: WebsocketActions, topic: WebSocketTopic, payload?: string) => {
      console.log("DISPLAY(): ", topic, payload);
      const newMsg = { id: v4(), action, topic, payload };
      setMessage(newMsg);
      console.log("📩 Message from server:", payload);

      switch (topic) {
        case WebSocketTopic.CM_STATUS:
        case WebSocketTopic.CM_USER_MSG:
        case WebSocketTopic.CM_PRODUCT_ACTIVE:
        case WebSocketTopic.CM_PRODUCTS_DISABLED:
          // Prepend the new message and keep only the last MAX_QUEUE
          setCmMessageList((prev) => {
            const updatedList = [newMsg, ...prev];
            return updatedList.slice(0, MAX_QUEUE);
          });
          return;
        case WebSocketTopic.GUI_READ_STATUS:
        case WebSocketTopic.GUI_PRODUCT_START:
        case WebSocketTopic.GUI_PRODUCT_STOP:
        case WebSocketTopic.GUI_MSG_COMMAND:
          // Prepend the new message and keep only the last MAX_QUEUE
          setGuiMessageList((prev) => {
            const updatedList = [newMsg, ...prev];
            return updatedList.slice(0, MAX_QUEUE);
          });
          return;
        default:
        //
      }
    },
    []
  );

  const ws = useWebSocket();

  const hasRun = useRef(false);

  const init = async () => {
    // await new RequestBuilder(WebSocket_API).build();
    ws.clear();
    const subscriptions = [
      WS.CM_PRODUCTS_DISABLED,
      WS.CM_PRODUCT_ACTIVE,
      WS.CM_STATUS,
      WS.CM_USER_MSG,
      WS.GUI_MSG_COMMAND,
      WS.GUI_PRODUCT_START,
      WS.GUI_PRODUCT_STOP,
      WS.GUI_READ_STATUS,
    ];

    setTimeout(() => {
      subscriptions.forEach((topic) => ws.subscribe(topic, display));
    }, 1000);
  };

  useEffect(() => {
    if (!hasRun.current) {
      init();
      hasRun.current = true;
    }

    return () => ws.clear();
  }, []);

  const reset = () => {
    setCmMessageList([]);
    setGuiMessageList([]);
  };

  return (
    <WebSocketProvider display={display}>
      <div className="flex flex-1 flex-col border border-blue-500 w-full">
        <div className="flex flex-row items-center justify-between border border-red-500">
          <h1 className="p-0! m-0!">WebSocket Test</h1>
          <button onClick={reset}>Reset</button>
          <div className="flex flex-row gap-2 items-center">
            <GuiCommands />
          </div>
        </div>

        <div className="border border-blue-500 flex p-1 items-center gap-2">
          <h3 className="p-0! m-0!">Latest message: </h3>
          {message && <Message message={message} />}
        </div>
        <div className="flex flex-1 border border-red-500">
          <div className="flex flex-col gap-2 flex-1 p-1 border border-green-500">
            <h1>Grafical User Interface</h1>
            {guiMessageList.map((msg) => (
              <Message key={msg.id} message={msg} />
            ))}
          </div>
          <div className="flex flex-col gap-2 flex-1 p-1 border border-blue-500">
            <h1>Coffee Machine</h1>
            {cmMessageList.map((msg) => (
              <Message key={msg.id} message={msg} />
            ))}
          </div>
        </div>
      </div>
    </WebSocketProvider>
  );
}

const Message = ({ message }: { message: Message }) => (
  <div className="flex">
    <div className="w-32">
      <h3 className="m-0! p-0">{message.action.toUpperCase()}</h3>
    </div>

    <div className="flex w-lg gap-4">
      <div>📩</div>
      <div className="min-w-48">
        <b>{message.topic}</b>
      </div>
      <div>payload: {message.payload || "[ - ]"}</div>
    </div>
  </div>
);
