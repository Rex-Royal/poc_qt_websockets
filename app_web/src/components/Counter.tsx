"use client";
import { useEffect, useState } from "react";
import { useWebSocket } from "app/libs/websocket/useWebSocket";

export default function Counter() {
  const [count, setCount] = useState(0);

  const ws = useWebSocket();

  useEffect(() => {
    ws.connect();

    ws.subscribe("counter", (msg) => {
      const parsed = parseInt(msg, 10);
      if (!isNaN(parsed)) setCount(parsed);
    });
  }, []);

  const updateCount = (newCount: number) => {
    setCount(newCount);
    ws.publish("counter", newCount.toString());
  };

  return (
    <div className="p-4 border rounded w-fit bg-white shadow-md space-y-4">
      <h2 className="text-xl font-semibold text-gray-800">🔢 Live Counter</h2>
      <div className="text-3xl font-bold text-blue-600">{count}</div>
      <div className="space-x-2">
        <button
          className="px-3 py-1 bg-blue-500 text-white rounded hover:bg-blue-600"
          onClick={() => updateCount(count + 1)}
        >
          Increment
        </button>
        <button
          className="px-3 py-1 bg-red-500 text-white rounded hover:bg-red-600"
          onClick={() => updateCount(count - 1)}
        >
          Decrement
        </button>
        <button
          className="px-3 py-1 bg-gray-300 rounded hover:bg-gray-400"
          onClick={() => updateCount(0)}
        >
          Reset
        </button>
      </div>
    </div>
  );
}
