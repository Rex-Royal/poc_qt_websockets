#include "websockets/WebSocketCmComs.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "state/Atom.h"
#include "state/DerivedAtom.h"
#include "state/AtomObserverWrapper.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    /**
     * TEST ATOM in pure cpp
     */
    // Instantiate Atom and DerivedAtom
    Atom atom;
    atom.setValue("Hello from C++!");

    DerivedAtom derivedAtom(&atom);
    derivedAtom.setCppTransformer([](const QVariant &v)
                                  {
                                      return v.toString().toUpper(); // or your custom logic
                                  });

    // Output values
    qDebug() << "Atom Value: " << atom.value().toString();                       // Should print: "Hello from C++!"
    qDebug() << "Derived Atom Value: " << derivedAtom.derivedValue().toString(); // Should print: "HELLO FROM C++!"

    // Change the value of Atom and observe the effect on DerivedAtom
    atom.setValue("New Value from C++");

    qDebug() << "Atom Value after change: " << atom.value().toString();                       // Should print: "New Value from C++"
    qDebug() << "Derived Atom Value after change: " << derivedAtom.derivedValue().toString(); // Should print: "NEW VALUE FROM C++"

    Atom counter;
    counter.setValue(1);
    qDebug() << "Atom Value: " << counter.value().toString();
    counter.setValue(counter.value().toInt() + 1);
    qDebug() << "Atom Value: " << counter.value().toString();

    /**
     * END TEST
     */

    // Register external classes with QML
    qmlRegisterType<WebSocketCmComs>("MyApp", 1, 0, "WebSocketCmComs");

    // Register STATE utils with QML
    qmlRegisterType<Atom>("MyApp", 1, 0, "Atom");
    qmlRegisterType<DerivedAtom>("MyApp", 1, 0, "DerivedAtom");
    qmlRegisterType<AtomObserverWrapper>("MyApp", 1, 0, "AtomObserverWrapper");

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));

    QQmlApplicationEngine engine;
    engine.load(url);

    return app.exec();
}
