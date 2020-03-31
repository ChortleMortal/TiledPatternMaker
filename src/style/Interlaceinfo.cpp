#include "style/InterlaceInfo.h"
#include "geometry/threads.h"

InterlaceInfo::InterlaceInfo()
{
    visited     = false;
    start_under = false;
    thread      = nullptr;
}

void InterlaceInfo::initInterlace()
{
    visited     = false;
    start_under = false;
}

void InterlaceInfo::initThread()
{
    visited = false;
    thread  = nullptr;
}
