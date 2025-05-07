#!/bin/bash

# Create logs directory if it doesn't exist
mkdir -p logs

# Build and run the Docker container
docker compose up --build -d

# Show logs
docker compose logs -f
