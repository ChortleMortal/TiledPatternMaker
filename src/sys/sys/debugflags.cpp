#include "gui/top/system_view_controller.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"
#include "sys/sys/debugflags.h"

#define E2STR(x) #x

QString sDbgType[NUM_DFLAG_TYPES] =
    {
        E2STR(FLAG_REPAINT),
        E2STR(FLAG_CREATE_STYLE),
        E2STR(FLAG_CREATE_MOTIF),
};

eDbgFlagDefinition definitions[NUM_DEBUG_FLAGS] = {

    {ILACE_DBG,             "debug ilace",      FLAG_REPAINT },
    {OUTLINE_DBG,           "debug outline",    FLAG_REPAINT },
    {SOLO_EDGE,             "solo edge",        FLAG_REPAINT },
    {HIGHLIGHT_SELECTED,    "hilight selected", FLAG_REPAINT },
    {NO_SHADOW,             "no shadow",        FLAG_REPAINT },
    {NO_OUTLINE,            "no outline",       FLAG_REPAINT },
    {NO_FILL,               "no fill",          FLAG_REPAINT },
    {NO_FILL_STROKE,        "no fill stroke",   FLAG_REPAINT },
    {OVER_UNDER,            "over-under",       FLAG_REPAINT },
    {SIDE_1_PTS,            "side1-pts",        FLAG_REPAINT },
    {SIDE_2_PTS,            "side2-pts",        FLAG_REPAINT },
    {MARK_1_PTS,            "mark1-pts",        FLAG_REPAINT },
    {MARK_2_PTS,            "mark2-pts",        FLAG_REPAINT },
    {INDEX_LINES,           "index lines",      FLAG_REPAINT },
    {INDEX_CURVES,          "index curves",     FLAG_REPAINT },
    {DRAW_MID_LINE,         "draw mid line",    FLAG_REPAINT },
    {UNUSED,                "unused",           FLAG_REPAINT },
    {EXCLUDE_ODDS,          "exclude odds",     FLAG_REPAINT },
    {EXCLUDE_EVENS,         "exclude evens",    FLAG_REPAINT },
    {DRAW_INNER_OUTER,      "draw in-b-out-r",  FLAG_REPAINT },
    {DRAW_LINE_EDGES,       "draw line edges",  FLAG_REPAINT },
    {DRAW_CURVE_EDGES,      "draw curve edges", FLAG_REPAINT },
    {DRAW_CURVES_ONLY,      "draw curves only", FLAG_REPAINT },
    {DRAW_RADII,            "radii",            FLAG_REPAINT },

    {CURVES_AS_LINES,       "curves-as-lines",  FLAG_CREATE_STYLE },
    {USE_TRIGGER_1,         "use trigger 1",    FLAG_CREATE_STYLE },
    {USE_TRIGGER_2,         "use trigger 2",    FLAG_CREATE_STYLE },
    {USE_TRIGGER_3A,        "use trigger 3A",   FLAG_CREATE_STYLE },
    {USE_TRIGGER_3B,        "use trigger 3B",   FLAG_CREATE_STYLE },
    {NO_ALIGN_CURVES,       "no align curves",  FLAG_CREATE_STYLE },
    {NO_UNDER,              "no under (1)",     FLAG_CREATE_STYLE },
    {NO_OVER,               "no over (2)",      FLAG_CREATE_STYLE },
    {NO_2ND_CORRECTION,     "no 2nd correct",   FLAG_CREATE_STYLE },
    {VALIDATE,              "validate",         FLAG_CREATE_STYLE },
    {NULL_MARKS_1,          "marks1null",       FLAG_CREATE_STYLE },
    {NULL_MARKS_2,          "marks2null",       FLAG_CREATE_STYLE },
    {MARK_JOIN,             "mark join",        FLAG_CREATE_STYLE },
    {USE_OLDER_CODE,        "use older code",   FLAG_CREATE_STYLE },

    {SIDE_1_LINE,           "side1-line",       FLAG_REPAINT },
    {SIDE_2_LINE,           "side2-line",       FLAG_REPAINT },

    {USE_OUTLINE_INIT,      "use outline init", FLAG_CREATE_STYLE },
    {USE_OUTLINE_ALIGN,     "use outline align",FLAG_CREATE_STYLE },
    {APPROACH_6,            "approach 6",       FLAG_CREATE_STYLE },
    {DUMP_CASINGS,          "dump casings",     FLAG_CREATE_STYLE },
    {USE_NEWER_CODE,        "use newer code",   FLAG_CREATE_STYLE },
    {NO_GAP,                "no gap",           FLAG_CREATE_STYLE },
    {USE_MID_FIX,           "use mid fix",      FLAG_CREATE_STYLE },

    {USE_NEWER_PAINT,       "newer paint",      FLAG_REPAINT },
};

DebugFlags::DebugFlags()
{
    _version      = 8;      // NOTE - always bump this when table modified
    _ref          = 0;
    _dbgIndex     = 0;
    _triggerIndex = 0;
    _xval         = 0;
    _yval         = 0;
    _indexEnabled = false;
    _enabled      = false;

    retrieveOld();      // may or may not be used
}

// called from dlg (and only from dlg)
void DebugFlags::toggleFlag(eDbgFlag flag)
{
    DbgFlag * dflag = find(flag);
    Q_ASSERT(dflag);
    dflag->enb = !dflag->enb;
    if (dflag->type == FLAG_REPAINT )
        Sys::viewController->repaintView();
    else
        emit sig_dbgChanged(dflag->type);
    bumpRef();
}

// called by app to test flag
bool DebugFlags::flagged(eDbgFlag flag)
{
    if (!_enabled)
    {
        return false;
    }

    DbgFlag * dflag = find(flag);;
    if (dflag)
    {
        // found
        return dflag->enb;
    }

    dflag = findOld(flag);
    if (dflag)
    {
        // found old
        DbgFlag * newflag = new DbgFlag(flag, dflag->name,dflag->enb,dflag->type);
        flags[flag] = newflag;
        bumpRef();
        return newflag->enb;
    }

    // create new
    auto defn = definitions[flag];
    DbgFlag * newflag = new DbgFlag(flag,defn.name,false,defn.type);
    flags[flag] = newflag;
    bumpRef();
    return false;
}

void DebugFlags::clearFlags()
{
    flags.clear();
    oldFlags.clear();
    bumpRef();
}

void DebugFlags::persist()
{
    QSettings settings;

    settings.setValue("dbgIndex", _dbgIndex);
    settings.setValue("dbgTriggerIndex", _triggerIndex);
    settings.setValue("dbgFlagsEnb",_enabled);
    settings.setValue("dbgVer", _version);

    settings.beginGroup("debugflags");
    QMap<eDbgFlag, DbgFlag*>::const_iterator i = flags.constBegin();
    while (i != flags.constEnd())
    {
        eDbgFlag flag = i.key();
        const DbgFlag * dflag = i.value();
        QString name = dflag->name;
        settings.setValue(name, flag);
        settings.setValue(name + "_enb", dflag->enb);
        ++i;
    }
    settings.endGroup();
}

void DebugFlags::retrieveOld()
{
    QSettings settings;
    _dbgIndex     = settings.value("dbgIndex",0).toInt();
    _triggerIndex = settings.value("dbgTriggerIndex",0).toInt();
    _enabled      = settings.value("dbgFlagsEnb",false).toBool();
    uint version  = settings.value("dbgVer", 0).toUInt();

    if (version == _version)
    {
        settings.beginGroup("debugflags");

    QStringList keys = settings.childKeys();
    foreach (QString key, keys)
    {
        if (!key.contains("_enb"))
        {
            QString name    = key;
            eDbgFlag flag   = (eDbgFlag)settings.value(name, NUM_DEBUG_FLAGS).toUInt();
            if (flag < NUM_DEBUG_FLAGS)
            {
                eDbgFlagDefinition & ed = definitions[flag];
                if (ed.name == name)
                {
                    bool enb        = settings.value(name + "_enb", false).toBool();
                    eDbgType type   = ed.type;
                    DbgFlag * dflag = new DbgFlag(flag,name,enb,type);
                    oldFlags[flag]  = dflag;
                }
            }
        }
    }
    settings.endGroup();

    }
    else
    {
        settings.beginGroup("debugflags");
        settings.remove(""); // Removes all keys within the current group
        settings.endGroup();
        settings.sync();
    }

    if (!Sys::config->insightMode)
    {
        _enabled = false;
    }
}

void DebugFlags::eraseHistory()
{
    QSettings settings;
    settings.beginGroup("debugflags");
    settings.remove(""); // Removes all keys within the current group
    settings.endGroup();
    settings.sync();

    clearFlags();
}

void DebugFlags::setDbgIndex(int val)
{
    _dbgIndex = val;
    if (_indexEnabled)
    {
        emit sig_dbgChanged(FLAG_CREATE_STYLE);
    }
}

void DebugFlags::setIndexEnable(bool enb)
{
    _indexEnabled = enb;
    emit sig_dbgChanged(FLAG_CREATE_STYLE);
}

void DebugFlags::setTriggerIndex(int val)
{
    _triggerIndex = val;
}

void DebugFlags::trigger()
{
    emit sig_dbgTrigger(_triggerIndex);
}

void DebugFlags::setXval(qreal val, bool repaint)
{
    _xval = val;
    if (repaint)
        Sys::viewController->repaintView();

}
void DebugFlags::setYval(qreal val, bool repaint)
{
    _yval = val;
    if (repaint)
        Sys::viewController->repaintView();
}

DbgFlag * DebugFlags::find(eDbgFlag flag)
{
    auto it = flags.find(flag);
    if (it == flags.end())
        return nullptr;
    else
        return *it;
}

DbgFlag * DebugFlags::findOld(eDbgFlag flag)
{
    auto it = oldFlags.find(flag);
    if (it == oldFlags.end())
        return nullptr;
    else
        return *it;
}

DbgFlag * DebugFlags::find(QString name)
{
    for (auto it = flags.keyValueBegin(); it != flags.keyValueEnd(); ++it)
    {
        DbgFlag * flag = it->second;
        if (flag->name == name)
            return flag;
    }
    return nullptr;
}


QStringList DebugFlags::names()
{
    QStringList qsl;
    for (DbgFlag * flag : std::as_const(flags))
    {
        qsl << flag->name;
        qsl << ((flag->enb) ? "2"  : "1");
    }
    return qsl;
}

void DebugFlags::dumpSet()
{
    QStringList qsl;
    for (DbgFlag * flag : std::as_const(flags))
    {
        if (flag->enb)
            qsl << flag->name;
    }
    qDebug().noquote() << "Flags:" << qsl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// DbgFlag
//
/////////////////////////////////////////////////////////////////////////////////////////////////

DbgFlag::DbgFlag(eDbgFlag flag, QString name, bool enb, eDbgType type)
{
    this->flag = flag;
    this->name = name;
    this->enb  = enb;
    this->type = type;
}

