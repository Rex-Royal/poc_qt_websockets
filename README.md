# PoC QT WebSockets

Proof of Concept between a QT5.13.2 project and Websotkets

## Project structure

### PROJECT: QT Hello World

The QT Proof of Concept to showcase how to communicate publish/subscribe to WS.

```bash
app_qt/
├── CMakeLists.txt
├── main.cpp
└── main.qml
```

#### QT Installation & Running

1. `./app_qt/install.sh`
2. `./app_qt/compile.sh`
3. `./app_qt/build/app_qt`

### PROJECT: WEB Server

The Web Server to showcase how to communicate publish/subscribe to WS.

```bash
app_web/
├── TBD
├── TBD
└── TBD
```

#### WEB Installation & Running

1. Install NVM (Nove Version Manager); see [freeCodeCamp tutorial](https://www.freecodecamp.org/news/node-version-manager-nvm-install-guide/).
2. Install Node (using NVM)
3. `cd ./app_web/`
4. `npm run dev`
