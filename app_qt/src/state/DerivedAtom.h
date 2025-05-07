#ifndef DERIVEDATOM_H
#define DERIVEDATOM_H

#include "Atom.h"

#include <QPointer>
#include <QJSValue>
#include <QQmlEngine>
#include <functional>

class DerivedAtom : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Atom *baseAtom READ baseAtom WRITE setBaseAtom NOTIFY baseAtomChanged)
    Q_PROPERTY(QVariant derivedValue READ derivedValue NOTIFY derivedValueChanged)
    Q_PROPERTY(QJSValue transformer READ transformer WRITE setTransformer NOTIFY transformerChanged)

public:
    using Transformer = std::function<QVariant(const QVariant &)>;

    explicit DerivedAtom(Atom *baseAtom = nullptr, QObject *parent = nullptr)
        : QObject(parent),
          m_baseAtom(baseAtom ? baseAtom : new Atom(parent)),
          m_cppTransformer([](const QVariant &v)
                           { return v; }) // default identity
    {
        connect(m_baseAtom, &Atom::valueChanged, this, &DerivedAtom::onBaseValueChanged);
    }

    QVariant derivedValue()
    {
        if (m_qmlTransformer.isCallable())
        {
            QJSValueList args;
            QQmlEngine *engine = qmlEngine(this);
            if (engine)
                args << engine->toScriptValue(m_baseAtom->value());
            QJSValue result = m_qmlTransformer.call(args);
            return result.toVariant();
        }
        return m_cppTransformer(m_baseAtom->value());
    }

    Atom *baseAtom() const { return m_baseAtom; }

    void setBaseAtom(Atom *newBaseAtom)
    {
        if (m_baseAtom != newBaseAtom)
        {
            if (m_baseAtom)
            {
                disconnect(m_baseAtom, nullptr, this, nullptr);
            }
            m_baseAtom = newBaseAtom;
            if (m_baseAtom)
            {
                connect(m_baseAtom, &Atom::valueChanged, this, &DerivedAtom::onBaseValueChanged);
            }
            emit baseAtomChanged();
            emit derivedValueChanged();
        }
    }

    QJSValue transformer() const { return m_qmlTransformer; }

    void setTransformer(const QJSValue &jsTransformer)
    {
        m_qmlTransformer = jsTransformer;
        emit transformerChanged();
        emit derivedValueChanged();
    }

    void setCppTransformer(const Transformer &transformer)
    {
        m_cppTransformer = transformer;
        emit derivedValueChanged();
    }

    Q_INVOKABLE void refresh()
    {
        emit derivedValueChanged();
    }

signals:
    void derivedValueChanged();
    void baseAtomChanged();
    void transformerChanged();

private slots:
    void onBaseValueChanged()
    {
        emit derivedValueChanged();
    }

private:
    QPointer<Atom> m_baseAtom;
    Transformer m_cppTransformer;
    QJSValue m_qmlTransformer;
};

#endif // DERIVEDATOM_H
