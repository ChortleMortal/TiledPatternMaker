#ifndef MAPEDITORSTASH_H
#define MAPEDITORSTASH_H

#include <QtCore>

typedef std::shared_ptr<class Circle>  CirclePtr;

class MapEditor;

#define MAX_STASH 9

class MapEditorStash : public QObject
{
    Q_OBJECT

public:

    MapEditorStash(MapEditor * me);

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
    void        slot_nextAnimationStep();

protected:
    bool         readStashTo(QString name, QVector<QLineF>  & lines, QVector<CirclePtr> & circs);

private:
    MapEditor * editor;
    int first;
    int last;
    int current;

    QTimer           * timer;
    QVector<QLineF>    localLines;
    QVector<CirclePtr> localCircs;

};

#endif // MAPEDITORSTASH_H
