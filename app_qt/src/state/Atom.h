#ifndef ATOM_H
#define ATOM_H

#include <QObject>
#include <QVariant>
#include <QDebug>

class Atom : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit Atom(QObject *parent = nullptr)
        : QObject(parent), m_value(QVariant()) {}

    QVariant value() const
    {
        return m_value;
    }

    void setValue(const QVariant &newValue)
    {
        if (m_value != newValue)
        {
            m_value = newValue;
            emit valueChanged();
        }
    }

    Q_SIGNAL void valueChanged();

private:
    QVariant m_value;
};

#endif // ATOM_H
