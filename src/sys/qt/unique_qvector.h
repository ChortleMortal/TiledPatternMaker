#pragma once
#ifndef UNIQUE_QVECTOR_H
#define UNIQUE_QVECTOR_H

#include <QVector>

template <class T> class UniqueQVector : public QVector<T>
{
public:
    UniqueQVector();
    UniqueQVector(const QVector<T> & other);

    void push_back(const T &value);
    void push_front(const T &value);

};

template <class T> UniqueQVector<T>::UniqueQVector() : QVector<T>()
{}

template <class T> UniqueQVector<T>::UniqueQVector(const QVector<T> & other) : QVector<T>()
{
    for (const auto & t : other)
    {
        push_back(t);
    }
}

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
#endif
