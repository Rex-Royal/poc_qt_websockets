#!/usr/bin/env bash

cd ..
rm -rf build -y

sudo apt update
sudo apt install qt5-qmake qtbase5-dev qtbase5-dev-tools qtchooser qt5-qmake-bin qtquickcontrols2-5-dev qml-module-qtquick-controls2 qml-module-qtquick2 qml-module-qtquick-window2 qml-module-qtwebsockets libqt5websockets5-dev -y

conan install . --output-folder=build --build=missing
