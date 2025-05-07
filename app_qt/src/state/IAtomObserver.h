// IAtomObserver.h
#ifndef IATOMOBSERVER_H
#define IATOMOBSERVER_H

#include <QVariant>
#include <functional>

class IAtomObserver
{

public:
    virtual ~IAtomObserver() = default;

    virtual QVariant getValue() const = 0;
    virtual void setValue(const QVariant &value) = 0;
    virtual void observe(std::function<void()> callback) = 0;
};

#endif // IATOMOBSERVER_H
