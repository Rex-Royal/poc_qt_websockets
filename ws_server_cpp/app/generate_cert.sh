#!/bin/bash
# Script to generate a self-signed certificate for testing WebSocket SSL
# Usage: ./generate_cert.sh [hostname]

HOSTNAME=${1:-localhost}
DAYS=365
OUT_DIR="./ssl"

# Create output directory if it doesn't exist
mkdir -p $OUT_DIR

echo "Generating self-signed certificate for $HOSTNAME (valid for $DAYS days)"

# Generate private key
openssl genrsa -out $OUT_DIR/server.key 2048

# Generate certificate signing request (CSR)
openssl req -new -key $OUT_DIR/server.key -out $OUT_DIR/server.csr -subj "/CN=$HOSTNAME/O=WebSocketBroker/C=US"

# Generate self-signed certificate
openssl x509 -req -days $DAYS -in $OUT_DIR/server.csr -signkey $OUT_DIR/server.key -out $OUT_DIR/server.crt

# Clean up CSR file
rm $OUT_DIR/server.csr

echo "Done! Certificate files created in $OUT_DIR directory:"
echo " - $OUT_DIR/server.key (Private Key)"
echo " - $OUT_DIR/server.crt (Certificate)"
echo ""
echo "To use with WebSocketBroker:"
echo "./websocket_broker --secure --cert $OUT_DIR/server.crt --key $OUT_DIR/server.key"
echo ""
