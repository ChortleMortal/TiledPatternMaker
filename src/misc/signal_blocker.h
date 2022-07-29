#ifndef SIGNAL_BLOCKER_H
#define SIGNAL_BLOCKER_H

#include <QObject>
#include <QPointer>

/**
 * Small helper class that blocks all signals from an object for the lifetime of this object.
 * it is safe against deletion of the object before deletion of this.
 *
 * This class was written by Bo Thorsen of Viking Software <bo@vikingsoft.eu>.
 * The code is in the public domain.
 */
class SignalBlocker
{
public:
    explicit SignalBlocker(QObject* object) : mObject(object)
    {
        mWasBlocked = object->signalsBlocked();
        object->blockSignals(true);
    }

    ~SignalBlocker()
    {
        if (mObject && !mWasBlocked)
            mObject->blockSignals(false);
    }

private:
    // Disabled
    SignalBlocker(const SignalBlocker&);
    SignalBlocker& operator=(const SignalBlocker&);

    QPointer<QObject> mObject;
    bool mWasBlocked;
};

#endif
