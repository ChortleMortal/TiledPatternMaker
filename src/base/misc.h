/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MISC_H
#define MISC_H

#include <QtWidgets>
#include <QPainter>

#include "base/layer.h"


// MarkX
class MarkX : public Layer
{
public:
    MarkX(QPointF a, QPen  pen, int index=0);
    MarkX(QPointF a, QPen  pen, QString txt);

    void setHuge() {huge = true;}

    void paint(QPainter *painter) override;

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

private:
    bool    huge;
    QPointF _a;
    QPen    _pen;
    int     _index;
    QString _txt;
};


// UniqueQVector
template <class T> class UniqueQVector : public QVector<T>
{
public:
    UniqueQVector();

    void push_back(const T &value);
    void push_front(const T &value);

};

template <class T> UniqueQVector<T>::UniqueQVector() : QVector<T>()
{}

template <class T> void UniqueQVector<T>::push_back(const T & value)
{
    if (!QVector<T>::contains(value))
    {
        QVector<T>::push_back(value);
    }
}

template <class T> void UniqueQVector<T>::push_front(const T & value)
{
    if (!QVector<T>::contains(value))
    {
        QVector<T>::push_front(value);
    }
}

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
#endif // MISC_H
