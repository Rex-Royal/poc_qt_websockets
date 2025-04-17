import { useState } from "react";
import { WebSocketClient } from "./WebSocketClient";

export const useWebSocket = (url?: string) => {
  const [ws, _] = useState<WebSocketClient>(WebSocketClient.getInstance(url));
  return ws;
};
