// lib/WebSocketBroker.ts
import { createServer, IncomingMessage, ServerResponse } from "http";
import { WebSocketServer, WebSocket, RawData } from "ws";
import { Socket } from "net";
import { WebSocketTopic } from "./WebSocketTopic";
import { WebsocketActions } from "./WebSocketActions";

type Topic = string;
type Callback = (message: string) => void;

interface ClientSubscription {
  socket: WebSocket;
  topics: Set<Topic>;
}

export class WebSocketBroker {
  private static instance: WebSocketBroker;
  private wss: WebSocketServer;
  private clients: Set<ClientSubscription> = new Set();
  private server = createServer((_: IncomingMessage, res: ServerResponse) => {
    res.writeHead(200, { "Content-Type": "application/json" });
    res.end(JSON.stringify({ message: "WebSocketBroker running" }));
  });

  private port: number;

  private constructor(port: number) {
    this.port = port;
    this.wss = new WebSocketServer({ noServer: true });

    this.wss.on("connection", (ws: WebSocket) => {
      const subscription: ClientSubscription = {
        socket: ws,
        topics: new Set(),
      };
      this.clients.add(subscription);

      ws.on("message", (data: RawData) => {
        try {
          const { action, topic, message } = JSON.parse(data.toString());
          // console.log("received", action, topic, message);
          if (action === WebsocketActions.SUBSCRIBE) {
            subscription.topics.add(topic);
          } else if (action === WebsocketActions.UNSUBSCRIBE) {
            subscription.topics.delete(topic);
          } else if (action === WebsocketActions.PUBLISH) {
            this.publish(topic, message);
          }
        } catch (err) {
          console.error("Invalid message", err);
        }
      });

      ws.on("close", () => {
        this.clients.delete(subscription);
      });

      ws.send(
        JSON.stringify({
          action: WebsocketActions.PUBLISH,
          topic: WebSocketTopic.CHAT,
          message: "Welcome to the WebSocket server!",
        })
      );
    });

    this.server.on(
      "upgrade",
      (req: IncomingMessage, socket: Socket, head: Buffer) => {
        this.wss.handleUpgrade(req, socket, head, (ws) => {
          this.wss.emit("connection", ws, req);
        });
      }
    );

    this.server.listen(this.port, () => {
      console.log(
        `✅ WebSocketBroker listening on ws://localhost:${this.port}`
      );
    });
  }

  static getInstance(port: number = 3001): WebSocketBroker {
    if (!WebSocketBroker.instance) {
      WebSocketBroker.instance = new WebSocketBroker(port);
    }
    return WebSocketBroker.instance;
  }

  publish(topic: Topic, message: string) {
    for (const client of this.clients) {
      if (client.topics.has(topic)) {
        client.socket.send(JSON.stringify({ topic, message }));
      }
    }
  }

  subscribe(topic: Topic, callback: Callback) {
    // Optional: could be extended to support internal handlers (not just WS)
    console.warn(
      "Internal subscribe not implemented. Use WS clients to subscribe."
    );
  }
}
