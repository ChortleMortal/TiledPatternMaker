#ifndef PAGE_LODERS_WORKER_H
#define PAGE_LODERS_WORKER_H

#include "sys/sys/versioning.h"
#include <QObject>

class PanelWorker : public QObject
{
    Q_OBJECT

public:
    PanelWorker();

    static VersionFileList findTilingsWithBkgds(VersionFileList &files);

public slots:
    void    sortTilings(bool worklist, bool backgrounds, bool sortByDate);
    void    sortMosaics(bool worklist, bool backgrounds, bool sortByDate);

signals:
    void    tilingsSorted();
    void    mosaicsSorted();

protected:
    VersionFileList sortTilingsByDate(VersionFileList &names);
    VersionFileList sortMosaicsByDate(VersionFileList &vlist);
    VersionFileList findMosaicsWithBkgds(VersionFileList &files);
private:
};

#endif
