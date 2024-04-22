#ifndef PAGE_LODERS_WORKER_H
#define PAGE_LODERS_WORKER_H

#include <QObject>

class PageLoadersWorker : public QObject
{
    Q_OBJECT

public:
    PageLoadersWorker();

public slots:
    void    sortTilings(bool worklist, bool backgrounds, bool sort);
    void    sortMosaics(bool worklist, bool backgrounds, bool sort);

signals:
    void    tilingsSorted();
    void    mosaicsSorted();

protected:
    QStringList sortTilingsByDate(const QStringList &names);
    QStringList sortMosaicsByDate(const QStringList &names);
    QStringList findMosaicsWithBkgds(const QStringList & names);
    QStringList findTilingsWithBkgds(const QStringList & names);
private:
};

#endif // TILINGMONITOR_H
