import { WebsocketActions } from "./WebSocketActions";
import { OnSocketMessageHander } from "./WebSocketContext";

interface Subscriptions {
  [topic: string]: OnSocketMessageHander[];
}

export class WebSocketClient {
  private static instance: WebSocketClient;
  private socket: WebSocket | null = null;
  private url: string;
  private subscriptions: Subscriptions = {};

  private connected: boolean = false;

  private reconnectionLogic = () => {};

  private constructor(url: string) {
    this.url = url;
  }

  static getInstance(url?: string): WebSocketClient {
    if (!WebSocketClient.instance && url) {
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
      this.connected = true;
      // Resubscribe to all topics
      Object.keys(this.subscriptions).forEach((topic) => {
        this.send({ action: WebsocketActions.SUBSCRIBE, topic });
      });
    };

    this.socket.onmessage = (event: MessageEvent) => {
      const { action, topic, payload } = JSON.parse(event.data);
      const handlers = this.subscriptions[topic] || [];
      handlers.forEach((cb) => cb(action, topic, payload));
    };

    this.socket.onclose = () => {
      console.warn("❌ WebSocket closed. Reconnecting in 1s...");
      this.connected = false;
      this.reconnectionLogic();
      setTimeout(() => this.connect(), 1000); // Optional auto-reconnect
    };

    this.socket.onerror = (err) => {
      console.error("WebSocket error:", err);
    };
  }

  subscribe(topic: string, callback: OnSocketMessageHander) {
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

  publish(topic: string, payload: string) {
    this.send({ action: WebsocketActions.PUBLISH, topic, payload });
  }

  private send(payload: object) {
    if (this.socket?.readyState === WebSocket.OPEN) {
      this.socket.send(JSON.stringify(payload));
    } else {
      console.warn("WebSocket not connected. Dropping message:", payload);
    }
  }

  clear() {
    this.subscriptions = {};
  }

  readyState() {
    return this.socket?.readyState;
  }

  isConnected() {
    return this.connected;
  }

  close() {
    this.connected = false;
    this.close();
  }
}
