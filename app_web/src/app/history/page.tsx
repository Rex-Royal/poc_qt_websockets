"use client";

import { Button } from "app/components/Button";
import { useWebSocket } from "app/libs/websocket/WebSocketContext";
import { WebSocketTopic } from "app/libs/websocket/WebSocketTopic";
import { useCallback, useEffect, useState } from "react";
import { v4 } from "uuid";

type Message = {
  id: string;
  topic: string;
  payload: string;
};

const WS = WebSocketTopic;

const MAX_QUEUE = 20;

export default function Home() {
  const [cmMessageList, setCmMessageList] = useState<Message[]>([]);
  const [guiMessageList, setGuiMessageList] = useState<Message[]>([]);
  const [message, setMessage] = useState<Message>();
  const ws = useWebSocket();

  const display = useCallback(
    (topic: WebSocketTopic) => (payload: string) => {
      console.log("DISPLAY(): ", topic, payload);
      const newMsg = { id: v4(), topic, payload };
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

  useEffect(() => {
    if (ws) {
      setTimeout(() => {
        // CM
        ws.subscribe(WS.CM_PRODUCTS_DISABLED, display(WS.CM_PRODUCTS_DISABLED));
        ws.subscribe(WS.CM_PRODUCT_ACTIVE, display(WS.CM_PRODUCT_ACTIVE));
        ws.subscribe(WS.CM_STATUS, display(WS.CM_STATUS));
        ws.subscribe(WS.CM_USER_MSG, display(WS.CM_USER_MSG));
        // GUI
        ws.subscribe(WS.GUI_MSG_COMMAND, display(WS.GUI_MSG_COMMAND));
        ws.subscribe(WS.GUI_PRODUCT_START, display(WS.GUI_PRODUCT_START));
        ws.subscribe(WS.GUI_PRODUCT_STOP, display(WS.GUI_PRODUCT_STOP));
        ws.subscribe(WS.GUI_READ_STATUS, display(WS.GUI_READ_STATUS));
      });
    }
    return () => {
      if (ws) {
        ws.unsubscribe(WS.CM_PRODUCTS_DISABLED);
        ws.unsubscribe(WS.CM_PRODUCT_ACTIVE);
        ws.unsubscribe(WS.CM_STATUS);
        ws.unsubscribe(WS.CM_USER_MSG);
        // GUI
        ws.unsubscribe(WS.GUI_MSG_COMMAND);
        ws.unsubscribe(WS.GUI_PRODUCT_START);
        ws.unsubscribe(WS.GUI_PRODUCT_STOP);
        ws.unsubscribe(WS.GUI_READ_STATUS);
      }
    };
  }, [ws]);

  // useEffect(() => {
  //   if (ws) {
  //     // CM
  //     ws.subscribe(WS.CM_PRODUCTS_DISABLED, display(WS.CM_PRODUCTS_DISABLED));
  //     ws.subscribe(WS.CM_PRODUCT_ACTIVE, display(WS.CM_PRODUCT_ACTIVE));
  //     ws.subscribe(WS.CM_STATUS, display(WS.CM_STATUS));
  //     ws.subscribe(WS.CM_USER_MSG, display(WS.CM_USER_MSG));
  //     // GUI
  //     ws.subscribe(WS.GUI_MSG_COMMAND, display(WS.GUI_MSG_COMMAND));
  //     ws.subscribe(WS.GUI_PRODUCT_START, display(WS.GUI_PRODUCT_START));
  //     ws.subscribe(WS.GUI_PRODUCT_STOP, display(WS.GUI_PRODUCT_STOP));
  //     ws.subscribe(WS.GUI_READ_STATUS, display(WS.GUI_READ_STATUS));
  //   }

  //   return () => {
  //     if (ws) {
  //       ws.unsubscribe(WS.CM_PRODUCTS_DISABLED);
  //       ws.unsubscribe(WS.CM_PRODUCT_ACTIVE);
  //       ws.unsubscribe(WS.CM_STATUS);
  //       ws.unsubscribe(WS.CM_USER_MSG);
  //       // GUI
  //       ws.unsubscribe(WS.GUI_MSG_COMMAND);
  //       ws.unsubscribe(WS.GUI_PRODUCT_START);
  //       ws.unsubscribe(WS.GUI_PRODUCT_STOP);
  //       ws.unsubscribe(WS.GUI_READ_STATUS);
  //     }
  //   };
  // }, []);

  return (
    <div className="flex flex-1 flex-col border border-blue-500 w-full">
      <div className="flex flex-row items-center justify-between border border-red-500">
        <h1 className="p-0! m-0!">WebSocket Test</h1>
        <div className="flex flex-row gap-2 items-center">
          <GuiCommands />
        </div>
      </div>

      <div className="border border-blue-500 flex h-30 p-1">
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
  );
}

const Message = ({ message }: { message: Message }) => (
  <div className="flex">
    <b className="min-w-48">{message.topic}</b>

    <div>📩 payload: [{message.payload ||" - "}]</div>
  </div>
);

type MessageCommandType = "StartCleaning" | "TresterEmptied" | "BeansFilled";
type MessageData = "cancel" | "ok";

const GuiCommands = () => {
  const ws = useWebSocket();

  const [pid, setPid] = useState<number>(0);
  const [commandType, setCommandType] = useState<MessageCommandType>();
  const [messageData, setMessageData] = useState<MessageData>();

  /**
   * GUI_READ_STATUS - empty
   * GUI_PRODUCT_START - pid
   * GUI_PRODUCT_STOP - empty
   * GUI_MSG_COMMAND - { type: StartCleaning | TresterEmptied | BeansFilled, data: "cancel" | "ok" }
   */
  const getStatus = () => ws && ws.publish(WebSocketTopic.GUI_READ_STATUS, "");

  const productStart = () =>
    ws && pid && ws.publish(WebSocketTopic.GUI_PRODUCT_START, String(pid));

  const productStop = () =>
    ws && ws.publish(WebSocketTopic.GUI_PRODUCT_STOP, "");

  const msgCommand = () =>
    ws &&
    commandType &&
    messageData &&
    ws.publish(
      WebSocketTopic.GUI_PRODUCT_START,
      JSON.stringify({ type: commandType, data: messageData })
    );

  const handlePidChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = e.target.value;
    if (/^\d*$/.test(value)) {
      setPid(parseInt(value));
    }
  };

  return (
    <>
      <Button label="Get Status" onClick={getStatus} />

      <label htmlFor="pid" className="m-0! p-0! block mb-2 text-sm">
        Enter the PID of the One Process:
      </label>
      <input
        id="pid"
        type="text"
        value={pid}
        onChange={handlePidChange}
        className="p-2 m-2 text-black rounded"
        placeholder="e.g., 4242"
      />

      <Button label="Start selected product" onClick={productStart} />
      <Button label="Stop product" onClick={productStop} />
      <Button label="Message comand" onClick={msgCommand} />
    </>
  );
};
