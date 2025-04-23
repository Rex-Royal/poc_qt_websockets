# PoC QT WebSockets

Proof of Concept between a QT5.13.2 project and Websotkets

## Project structure

### PROJECT: QT Hello World

The QT Proof of Concept to showcase how to communicate publish/subscribe to WS.

```bash
app_qt/
├── CMakeLists.txt        # CMake build configuration file
├── compile.sh            # Compile script
├── conanfile.txt         # Conan dependency configuration
├── install.sh            # Install script
├── main.cpp              # Main C++ source file
├── main.qml              # QML user interface file
└── resources.qrc         # Qt resource collection file
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

1. Install NVM (Nove Version Manager); see [freeCodeCamp tutorial](https://www.freecodecamp.org/news/node-version-manager-nvm-install-guide/).
2. Install Node (using NVM)
3. `cd ./app_web/`
4. `npm run dev`
