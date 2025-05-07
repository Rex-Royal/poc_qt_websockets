FROM ubuntu:22.04

# Install required dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3-pip \
    curl \
    qt5-qmake \
    qtbase5-dev \
    qtbase5-dev-tools \
    qtchooser \
    qt5-qmake-bin \
    qtquickcontrols2-5-dev \
    qml-module-qtquick-controls2 \
    qml-module-qtquick2 \
    qml-module-qtquick-window2 \
    qml-module-qtwebsockets \
    libqt5websockets5-dev \
    openssl \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install conan
RUN pip3 install conan==1.59.0

# Create app directory
WORKDIR /app

# Copy source files
COPY . /app/

# Copy entrypoint script
COPY ./app/docker-entrypoint.sh /app/
RUN chmod +x /app/docker-entrypoint.sh

# Expose WebSocket port
EXPOSE 3002

# Run the entrypoint script
ENTRYPOINT ["/app/docker-entrypoint.sh"]
