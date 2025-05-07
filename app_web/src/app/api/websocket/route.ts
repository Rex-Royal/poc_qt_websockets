import { NextRequest, NextResponse } from "next/server";
import { WebSocketBroker } from "app/libs/websocket/WebSocketBroker";
import {
  WS_PROTOCOL,
  WS_PORT,
  WS_DOMAIN,
} from "app/libs/websocket/WebSocketConf";

export async function GET(req: NextRequest): Promise<NextResponse> {
  // To publish a message from server:
  WebSocketBroker.getInstance(WS_PORT);
  WebSocketBroker.getInstance().publish("chat", "Hello from the server!");

  console.log("GET REQUEST TO /api/websocket");

  return new NextResponse(
    JSON.stringify({
      message: "WebSocket server running",
      url: `${WS_PROTOCOL}://${WS_DOMAIN}:${WS_PORT}`,
    }),
    { status: 200 }
  );
}
