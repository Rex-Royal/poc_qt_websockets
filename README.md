# PoC QT WebSockets

Proof of Concept between a QT5.13.2 project and WebSockets

## Project structure

### PROJECT: QT Hello World

The QT Proof of Concept to showcase how to communicate publish/subscribe to WS.

```bash
app_qt/
├── src/                  # QT code base
│   ├── qml/              # QML code
│   │   ├── components/   # QML components
│   │   │   └── ...
│   │   └── main.qml      # Main QML source file
│   ├── state/            # App State atomic utilities
│   │   └── ...
│   ├── websockets/       # WebSockets communication files
│   │   └── ...
│   ├── main.cpp          # Main C++ source file
│   └── resources.qrc     # Qt resource collection file
├── CMakeLists.txt        # CMake build configuration file
├── compile.sh            # Compile script
├── conanfile.txt         # Conan dependency configuration
├── install.sh            # Install script
└── ...
```

#### QT Installation & Running

1. `./app_qt/install.sh`
2. `./app_qt/compile.sh`
3. `./app_qt/build/app_qt`

### PROJECT: WEB Server

The Web Server to showcase how to communicate publish/subscribe to WS.

```bash
app_web/
├── public/
│   └── public files
├── src/
│   └── web codebase
├── package.json
├── tsconfig.json
└── ...
```

#### WEB Installation & Running

Keep in mind that due to self-signed certification issues with the browser, the GUI only works with insecure websockets. To work with secure websockets we'd need to assing the certificate to a domain and execute the WebSocket server from there, pointing the `WebSocketConf.ts` URL to the proper domain address.

1. Install NVM (Nove Version Manager); see [freeCodeCamp tutorial](https://www.freecodecamp.org/news/node-version-manager-nvm-install-guide/).
2. Install Node (using NVM)
3. `cd ./app_web/`
4. `npm install`
5. `npm run dev`

## SSL and non SSL

1. `cd ws_server_cpp`
2. `./start.sh`
3. `./stop.sh`

### ws_server_cpp

To test SSL/NonSSL.

1. Run `rr_ws_server`

    ```bash
    cd ws_server_cpp
    ./compile.sh && ./build/rr_ws_server -p 3002 --secure --cert ./ssl/server.crt --key ./ssl/server.key
    # insecure
    ./compile.sh && ./build/rr_ws_server -p 3002
    ```

2. Run `app_qt`

    ```bash
    cd app_qt
    ./compile.sh && ./build/app_qt -w wss://localhost:3002
    # insecure
    ./compile.sh && ./build/app_qt -w ws://localhost:3002
    ```

3. If you have a true "let's encrypt" certificate on a proper domain, you can use the web server. If not, the web GUI will not work anymore. Here is how you test it:

    ```bash
    git clone https://github.com/vi/websocat.git
    cd websocat
    cargo build --release
    ./target/release/websocat wss://localhost:3002 --insecure
    ## you should have seen this message:
    {"action":"publish","payload":"Welcome! Your client ID is 29c89319","topic":"system"}

    ## while running websocat, you can also publish messages, just copy past the following:
    {"action":"SUBSCRIBE","topic":"CM_STATUS"}

    ## It should output:
    {"action":"subscribe","status":"success","topic":"CM_STATUS"}

    ## Now, if you change the "STATUS" counter or hit the "CM_STATUS" button, you should receive the following:
    {"action":"publish","payload":"3","timestamp":"2025-05-07T11:40:11","topic":"CM_STATUS"}
    {"action":"publish","payload":"4","timestamp":"2025-05-07T11:40:12","topic":"CM_STATUS"}
    {"action":"publish","payload":"5","timestamp":"2025-05-07T11:40:13","topic":"CM_STATUS"}

    ## You can also send messages the GUI can send like:
    {"action":"publish","topic":"GUI_READ_STATUS","payload":""}
    {"action":"publish","topic":"GUI_READ_STATUS"}

    ## Which will output
    {"action":"publish","payload":"4","timestamp":"2025-05-07T11:50:20","topic":"CM_STATUS"}
    ```
