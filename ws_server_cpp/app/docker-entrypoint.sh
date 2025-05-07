#!/bin/bash

set -e

# Default APP_PORT to 3002 if not set
APP_PORT=${APP_PORT:-3002}
SSL=${SSL:-false}

# Compose secure options if SSL is enabled
if [ "$SSL" = "true" ]; then

    # Generate SSL certificates for WebSocket server if they don't exist
    if [ ! -f /app/server.key ] || [ ! -f /app/server.crt ]; then
        echo "Generating self-signed SSL certificates..."
        openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
            -keyout /app/server.key -out /app/server.crt \
            -subj "/C=US/ST=State/L=City/O=Organization/CN=localhost"
        chmod 600 /app/server.key
    fi

    SECURE_OPTS="--secure --cert /app/server.crt --key /app/server.key"
else
    SECURE_OPTS=""
fi

# Build the application if it doesn't exist
if [ ! -f /app/build/rr_ws_server ]; then
    echo "Building application..."
    conan install . --output-folder=build --build=missing
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    cd ..
fi

# Run the application with automatic restart
while true; do
    echo "Starting WebSocket server on port $APP_PORT with SSL=$SSL..."
    cd /app/build && ./rr_ws_server -p "$APP_PORT" $SECURE_OPTS

    # If the application crashes, wait a bit before restarting
    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 0 ]; then
        echo "WebSocket server exited normally with code $EXIT_CODE"
        break
    else
        echo "WebSocket server crashed with code $EXIT_CODE. Restarting in 5 seconds..."
        sleep 5
    fi
done
