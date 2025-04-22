import type { Metadata } from "next";
import { Geist, Geist_Mono } from "next/font/google";
import "./globals.css";
import NavBar from "app/components/NavBar";
import { WebSocketProvider } from "app/libs/websocket/WebSocketContext";

const geistSans = Geist({
  variable: "--font-geist-sans",
  subsets: ["latin"],
});

const geistMono = Geist_Mono({
  variable: "--font-geist-mono",
  subsets: ["latin"],
});

export const metadata: Metadata = {
  title: "WebSocket Demo",
  description: "Next.js + WebSocket + TypeScript",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <body
        className={`${geistSans.variable} ${geistMono.variable} antialiased min-h-screen bg-gray-100 text-gray-900 prose-sm flex flex-col`}
      >
        <WebSocketProvider>
          <NavBar />
          <main className="p-6 flex flex-1 justify-center">{children}</main>
        </WebSocketProvider>
      </body>
    </html>
  );
}
