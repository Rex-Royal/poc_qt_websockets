import { WebsocketActions } from "./WebSocketActions";

type MessageHandler = (message: string) => void;

interface Subscriptions {
  [topic: string]: MessageHandler[];
}

export class WebSocketClient {
  private static instance: WebSocketClient;
  private socket: WebSocket | null = null;
  private url: string;
  private subscriptions: Subscriptions = {};

  private reconnectionLogic = () => {};

  private constructor(url: string) {
    this.url = url;
  }

  static getInstance(url: string = "/api/websocket"): WebSocketClient {
    if (!WebSocketClient.instance) {
      WebSocketClient.instance = new WebSocketClient(url);
    }
    return WebSocketClient.instance;
  }

  setReconnectionLogic(logic: () => void) {
    this.reconnectionLogic = logic;
  }

  connect() {
    this.socket = new WebSocket(this.url);

    this.socket.onopen = () => {
      console.log("✅ WebSocket connected");
      // Resubscribe to all topics
      Object.keys(this.subscriptions).forEach((topic) => {
        this.send({ action: WebsocketActions.SUBSCRIBE, topic });
      });
    };

    this.socket.onmessage = (event: MessageEvent) => {
      const { topic, message } = JSON.parse(event.data);
      const handlers = this.subscriptions[topic] || [];
      handlers.forEach((cb) => cb(message));
    };

    this.socket.onclose = () => {
      console.warn("❌ WebSocket closed. Reconnecting in 1s...");
      this.reconnectionLogic();
      setTimeout(() => this.connect(), 1000); // Optional auto-reconnect
    };

    this.socket.onerror = (err) => {
      console.error("WebSocket error:", err);
    };
  }

  subscribe(topic: string, callback: MessageHandler) {
    if (!this.subscriptions[topic]) {
      this.subscriptions[topic] = [];
      this.send({ action: WebsocketActions.SUBSCRIBE, topic });
    }
    this.subscriptions[topic].push(callback);
  }

  unsubscribe(topic: string) {
    if (this.subscriptions[topic]) {
      delete this.subscriptions[topic];
      this.send({ action: WebsocketActions.UNSUBSCRIBE, topic });
    }
  }

  publish(topic: string, message: string) {
    this.send({ action: WebsocketActions.PUBLISH, topic, message });
  }

  private send(payload: object) {
    if (this.socket?.readyState === WebSocket.OPEN) {
      this.socket.send(JSON.stringify(payload));
    } else {
      console.warn("WebSocket not connected. Dropping message:", payload);
    }
  }

  close() {
    this.close();
  }
}
