// AtomObserverWrapper.h
#ifndef ATOMOBSERVERWRAPPER_H
#define ATOMOBSERVERWRAPPER_H

#include "Atom.h"
#include "IAtomObserver.h"

#include <QObject>
#include <QJSValue>
#include <functional>

class AtomObserverWrapper : public QObject, public IAtomObserver
{
    Q_OBJECT
    Q_PROPERTY(Atom *atom READ atom WRITE setAtom NOTIFY atomChanged)

public:
    explicit AtomObserverWrapper(QObject *parent = nullptr)
        : QObject(parent), m_atom(nullptr) {}

    Q_INVOKABLE QVariant getValue() const override
    {
        return m_atom ? m_atom->value() : QVariant();
    }

    void setValue(const QVariant &value) override
    {
        if (m_atom)
        {
            m_atom->setValue(value);
        }
    }

    void observe(std::function<void()> callback) override
    {
        m_callback = callback;
        if (m_atom)
        {
            connect(m_atom, &Atom::valueChanged, this, [this]()
                    {
                if (m_callback) m_callback(); });
        }
    }

    Q_INVOKABLE void observe(QJSValue callback)
    {
        if (!m_atom || !callback.isCallable())
            return;

        m_js_callback = callback;

        connect(m_atom, &Atom::valueChanged, this, [this]()
                {
            if (m_js_callback.isCallable()) {
                QJSValue result = m_js_callback.call();
                if (result.isError()) {
                    qWarning() << "observe callback error:" << result.toString();
                }
            } });
    }

    Atom *atom() const
    {
        return m_atom;
    }

    void setAtom(Atom *newAtom)
    {
        if (m_atom != newAtom)
        {
            if (m_atom)
            {
                disconnect(m_atom, nullptr, this, nullptr);
            }

            m_atom = newAtom;

            if (m_atom && m_callback)
            {
                connect(m_atom, &Atom::valueChanged, this, [this]()
                        {
                            if (m_callback )
                                m_callback();
                            if (m_js_callback.isCallable())
                            {
                                QJSValue result = m_js_callback.call();
                                if (result.isError()) {
                                    qWarning() << "observe callback error:" << result.toString();
                                }
                            } });
            }

            emit atomChanged();
        }
    }

signals:
    void atomChanged();

private:
    Atom *m_atom;
    std::function<void()> m_callback;
    QJSValue m_js_callback;
};

#endif // ATOMOBSERVERWRAPPER_H
