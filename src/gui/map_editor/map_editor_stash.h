#pragma once

#include <QString>

#include "sys/sys/versioning.h"

class MapEditorDb;

// construction lines/circles: a working set rewritten on every change and restored at startup,
// plus named templates saved/loaded on request
class MapEditorStash
{
public:
    static bool stash(MapEditorDb * db);
    static bool restore(MapEditorDb * db);

    static bool saveTemplate(VersionedName vname, MapEditorDb * db);
    static bool loadTemplate(VersionedFile xfile, MapEditorDb * db);

private:
    static QString workingFile();
    static bool    write(QString name, MapEditorDb * db);
    static bool    read(QString name, MapEditorDb * db);
};
