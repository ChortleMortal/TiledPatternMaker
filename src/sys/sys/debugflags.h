#pragma once
#ifndef DEBUGFLAGS_H
#define DEBUGFLAGS_H

#include <QObject>
#include <QMap>

class page_debug;

enum eDbgType
{
    FLAG_REPAINT,
    FLAG_CREATE_STYLE,
    FLAG_CREATE_MOTIF,
    NUM_DFLAG_TYPES,
};

extern QString sDbgType[NUM_DFLAG_TYPES];

enum eDbgFlag
{
    ILACE_DBG,
    OUTLINE_DBG,
    SOLO_EDGE,
    HIGHLIGHT_SELECTED,
    NO_SHADOW,
    NO_OUTLINE,
    NO_FILL,
    NO_FILL_STROKE,
    OVER_UNDER,
    SIDE_1_PTS,
    SIDE_2_PTS,
    MARK_1_PTS,
    MARK_2_PTS,
    INDEX_LINES,
    INDEX_CURVES,
    DRAW_MID_LINE,
    UNUSED,
    EXCLUDE_ODDS,
    EXCLUDE_EVENS,
    DRAW_INNER_OUTER,
    DRAW_LINE_EDGES,
    DRAW_CURVE_EDGES,
    DRAW_CURVES_ONLY,
    DRAW_RADII,

    CURVES_AS_LINES,
    USE_TRIGGER_1,
    USE_TRIGGER_2,
    USE_TRIGGER_3A,
    USE_TRIGGER_3B,
    NO_ALIGN_CURVES,
    NO_UNDER,
    NO_OVER,
    NO_2ND_CORRECTION,
    VALIDATE,
    NULL_MARKS_1,
    NULL_MARKS_2,
    MARK_JOIN,
    USE_OLDER_CODE,

    SIDE_1_LINE,
    SIDE_2_LINE,

    USE_OUTLINE_INIT,
    USE_OUTLINE_ALIGN,
    APPROACH_6,
    DUMP_CASINGS,
    USE_NEWER_CODE,
    NO_GAP,
    USE_MID_FIX,

    USE_NEWER_PAINT,
    NUM_DEBUG_FLAGS
};

struct eDbgFlagDefinition
{
    eDbgFlag    flag;
    QString     name;
    eDbgType    type;
};

class DbgFlag
{
public:
    DbgFlag(eDbgFlag flag, QString name, bool enb, eDbgType type);

    eDbgFlag flag;
    QString  name;
    eDbgType type;
    bool     enb;
};

class DebugFlags : public QObject
{
    Q_OBJECT

public:
    DebugFlags();

    void enable(bool set)           { _enabled = set; }
    bool enabled()                  { return _enabled; }
    void clearFlags();

    bool flagged(eDbgFlag flag);
    void setFlag(eDbgFlag flag, bool set);
    void toggleFlag(eDbgFlag flag);

    void setDbgIndex(int val);
    int  getDbgIndex()              { return _dbgIndex; }

    DbgFlag * find(QString name);

    void setIndexEnable(bool enb);
    bool getIndexEnable()           { return _indexEnabled; }

    void setXval(qreal val, bool repaint);
    void setYval(qreal val, bool repaint);
    qreal getXval()                 { return _xval;}
    qreal getYval()                 { return _yval;}

    void trigger();
    void setTriggerIndex(int val);
    uint getTriggerIndex()          { return _triggerIndex; }

    uint getRef()                   { return _ref; }
    void bumpRef()                  { _ref++; }
    void persist();
    void eraseHistory();

    void  dumpSet();

    const QVector<DbgFlag*> getFlags() { return flags.values(); }

signals:
    void sig_dbgChanged(eDbgType type);
    void sig_dbgTrigger(int val);

protected:
    void retrieveOld();

private:
    DbgFlag * find(eDbgFlag flag);
    DbgFlag * findOld(eDbgFlag flag);
    QStringList names();

    QMap<eDbgFlag, DbgFlag*> flags;
    QMap<eDbgFlag, DbgFlag*> oldFlags;

    uint _ref;

    int  _dbgIndex;
    uint _triggerIndex;
    bool _indexEnabled;
    qreal _xval;
    qreal _yval;


    bool _enabled;
    uint _version;
};

#endif
