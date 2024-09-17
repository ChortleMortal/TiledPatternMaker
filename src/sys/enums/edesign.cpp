#include "edesign.h"

#define E2STR(x) #x

QMap<int, QString> designs;

void init_legacy_designs()
{
    designs[DESIGN_5] = E2STR(DESIGN_5);
    designs[DESIGN_6] = E2STR(DESIGN_6);
    designs[DESIGN_7] = E2STR(DESIGN_7);
    designs[DESIGN_8] = E2STR(DESIGN_8);
    designs[DESIGN_9] = E2STR(DESIGN_8);
    designs[DESIGN_HU_INSERT] = E2STR(DESIGN_HU_INSERT);
    designs[DESIGN_10] = E2STR(DESIGN_10);
    designs[DESIGN_11] = E2STR(DESIGN_11);
    designs[DESIGN_12] = E2STR(DESIGN_12);
    designs[DESIGN_13] = E2STR(DESIGN_13);
    designs[DESIGN_14] = E2STR(DESIGN_14);
    designs[DESIGN_16] = E2STR(DESIGN_15);
    designs[DESIGN_17] = E2STR(DESIGN_16);
    designs[DESIGN_18] = E2STR(DESIGN_17);
    designs[DESIGN_19] = E2STR(DESIGN_18);
    designs[DESIGN_KUMIKO1] = E2STR(DESIGN_KUMIKO1);
    designs[DESIGN_KUMIKO2] = E2STR(DESIGN_KUMIKO2);
    designs[NO_DESIGN] = E2STR(NO_DESIGN);
}
