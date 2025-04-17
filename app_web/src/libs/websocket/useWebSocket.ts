import { useState } from "react";
import { WebSocketClient } from "./WebSocketClient";

export const useWebSocket = () => {
  const [ws, _] = useState<WebSocketClient>(WebSocketClient.getInstance());
  return ws;
};
