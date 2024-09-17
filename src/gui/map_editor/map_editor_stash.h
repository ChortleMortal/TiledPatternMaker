#pragma once
#ifndef MAPEDITORSTASH_H
#define MAPEDITORSTASH_H

#include <QObject>
#include <QTimer>
#include <QLineF>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

#include "sys/geometry/circle.h"

typedef std::shared_ptr<class Circle>           CirclePtr;

class MapEditorDb;

#define MAX_STASH 9

class MapEditorStash
{
    friend class MapEditor;

public:

    MapEditorStash();

protected:
    void    init() { first = -1; last = -1; current = -1; }

    bool    saveTemplate(VersionedName vname);

    bool    stash(MapEditorDb *db);
    bool    destash();
    bool    undoStash();
    bool    redoStash();

    bool    initStash(VersionedName mosaicname, MapEditorDb *db);

    bool    readStash(VersionedFile & xfile, MapEditorDb *db);
    bool    animateReadStash(VersionedFile & xfile);
    void    nextAnimationStep(MapEditorDb *db, QTimer *timer);
    bool    writeStash(QString name, MapEditorDb *db);

    int     getNext();
    int     getPrev();

    void    add(int index);

    QString getStashName(int index);

    bool    readStashTo(VersionedFile &xfile, QVector<QLineF>  & lines, QVector<CirclePtr> &circs);

private:
    int     first;
    int     last;
    int     current;

    QVector<QLineF>    localLines;
    QVector<CirclePtr> localCircs;
};

#endif // MAPEDITORSTASH_H
