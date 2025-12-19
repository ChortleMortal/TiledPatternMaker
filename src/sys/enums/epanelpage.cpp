#include "sys/enums/epanelpage.h"

#define E2STR(x) #x

const QString  sPageState[] =
{
    E2STR(PAGE_ATTACHED),
    E2STR(PAGE_SUB_ATTACHED),
    E2STR(PAGE_DETACHED)
};
