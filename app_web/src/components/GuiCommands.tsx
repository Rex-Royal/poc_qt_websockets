import { useWebSocket } from "app/libs/websocket/WebSocketContext";
import { WebSocketTopic } from "app/libs/websocket/WebSocketTopic";
import { useState } from "react";
import { Button } from "./Button";
import { Dropdown } from "./Dropdown";

type MessageCommandType = "CMD_START_CLEANING" | "CMD_CONFIRM_USERMSG" | "CMD_TBD_BY_USER";
type MessageData = "cancel" | "ok";

export const GuiCommands = () => {
  const ws = useWebSocket();

  const [pid, setPid] = useState<number>(42);
  const [commandType, setCommandType] = useState<MessageCommandType>();
  const [messageData, setMessageData] = useState<MessageData>();

  /**
   * GUI_READ_STATUS - empty
   * GUI_PRODUCT_START - pid
   * GUI_PRODUCT_STOP - empty
   * GUI_MSG_COMMAND - { type: CMD_START_CLEANING | CMD_CONFIRM_USERMSG | CMD_TBD_BY_USER, data: "cancel" | "ok" }
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
      WebSocketTopic.GUI_MSG_COMMAND,
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

      <div className="flex flex-col items-center">
        <label htmlFor="pid" className="block mb-2 text-sm font-semibold">
          Enter the PID of the One Process:
        </label>
        <input
          id="pid"
          type="text"
          value={pid}
          onChange={handlePidChange}
          className="p-2 border rounded leading-1 w-full"
          placeholder="e.g., 4242"
        />
      </div>

      <Button label="Start selected product" onClick={productStart} />
      <Button label="Stop product" onClick={productStop} />

      <Dropdown<MessageCommandType>
        id="commandType"
        label="Select Command Type"
        value={commandType}
        options={["CMD_START_CLEANING", "CMD_CONFIRM_USERMSG", "CMD_TBD_BY_USER"]}
        onChange={setCommandType}
      />
      <Dropdown<MessageData>
        id="messageData"
        label="Select Message Data"
        value={messageData}
        options={["cancel", "ok"]}
        onChange={setMessageData}
      />
      <Button label="Message comand" onClick={msgCommand} />
    </>
  );
};
