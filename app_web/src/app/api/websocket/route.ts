import { NextRequest, NextResponse } from "next/server";
import { WebSocketBroker } from "app/libs/websocket/WebSocketBroker";

const WS_PORT = 3002;

export async function GET(req: NextRequest): Promise<NextResponse> {
  // To publish a message from server:
  WebSocketBroker.getInstance(WS_PORT);
  WebSocketBroker.getInstance().publish("chat", "Hello from the server!");

  return new NextResponse(
    JSON.stringify({
      message: "WebSocket server running",
      url: `ws://localhost:${WS_PORT}`,
    }),
    { status: 200 }
  );
}
