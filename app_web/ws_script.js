const WebSocket = require("ws");

const socket = new WebSocket("wss://localhost:3002");

socket.on("open", function open() {
  console.log("WebSocket connected");
  socket.send("Hello Server");
});

socket.on("message", function incoming(data) {
  console.log("Received:", data);
});

socket.on("error", function (error) {
  console.error("WebSocket error:", error);
});
