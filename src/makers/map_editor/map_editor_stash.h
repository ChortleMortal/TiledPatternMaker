#pragma once
#ifndef MAPEDITORSTASH_H
#define MAPEDITORSTASH_H

#include <QObject>
#include <QTimer>
#include <QLineF>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

#include "geometry/circle.h"

typedef std::shared_ptr<class Circle>           CirclePtr;

class MapEditorDb;

#define MAX_STASH 9

class MapEditorStash : public QObject
{
    Q_OBJECT

public:

    MapEditorStash(MapEditorDb * db);

    bool         stash();
    bool         destash();
    bool         undoStash();
    bool         edoStash();

    bool         initStash(QString stashname);
    bool         keepStash(QString stashname);
    bool         saveTemplate(QString name);

    bool         readStash(QString name);
    bool         animateReadStash(QString name);
    bool         writeStash(QString name);

    int          getNext();
    int          getPrev();

    void         add(int index);

    QString      getStashName(int index);

protected slots:
    void         slot_nextAnimationStep();

protected:
    bool         readStashTo(QString name, QVector<QLineF>  & lines, QVector<CirclePtr> &circs);

private:
    int first;
    int last;
    int current;

    MapEditorDb      * db;
    QTimer           * timer;
    QVector<QLineF>    localLines;
    QVector<CirclePtr> localCircs;

};

#endif // MAPEDITORSTASH_H
