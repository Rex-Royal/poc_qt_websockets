"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";
import clsx from "clsx";

const links = [
  { href: "/", label: "Home" },
  { href: "/ws", label: "WebSocket" },
  { href: "/counter", label: "Counter" },
];

export default function NavBar() {
  const pathname = usePathname();

  return (
    <nav className="bg-white border-b px-6 py-4 shadow-sm flex space-x-4">
      {links.map(({ href, label }) => (
        <Link
          key={href}
          href={href}
          className={clsx(
            "hover:underline",
            pathname === href ? "text-blue-600 font-semibold" : "text-gray-700"
          )}
        >
          {label}
        </Link>
      ))}
    </nav>
  );
}
