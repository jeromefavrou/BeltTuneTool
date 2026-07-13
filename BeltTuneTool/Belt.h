
#ifndef BELT_H
#define BELT_H

#include <stdint.h>

//======================================================
// Structures
//======================================================

struct BeltLength
{
    uint16_t mm;
};

struct BeltWidth
{
    uint8_t mm;
    float linearMass;
};

struct BeltPitch
{
    const char* name;

    const BeltWidth* widths;
    uint8_t widthCount;

    const BeltLength* lengths;
    uint16_t lengthCount;
};

struct BeltFamily
{
    const char* name;

    const BeltPitch* pitches;
    uint8_t pitchCount;
};

//======================================================
// Longueurs
//======================================================

constexpr BeltLength L3M[] =
{
    {150},{180},{210},{240},{270},{300},
    {330},{360},{390},{420},{450},{480},
    {510},{540},{570},{600},{630},{660},
    {690},{720},{750},{780},{810},{840},
    {900},{960},{1020}
};

constexpr BeltLength L5M[] =
{
    {225},{250},{275},{300},{325},{350},
    {375},{400},{425},{450},{475},{500},
    {525},{550},{575},{600},{625},{650},
    {675},{700},{725},{750},{775},{800},
    {850},{900},{950},{1000},{1125},
    {1250},{1400},{1500},{1700},
    {1800},{2000},{2240},{2500},{2800},{3000}
};

constexpr BeltLength L8M[] =
{
    {440},{480},{560},{600},{640},{720},
    {800},{880},{960},{1000},{1040},
    {1120},{1200},{1280},{1360},
    {1440},{1600},{1760},{2000},{2240},{2800}
};

constexpr BeltLength L2M[] =
{
    {120},{140},{160},{180},{200},{220},
    {240},{260},{280},{300},{320},{340},
    {360},{380},{400},{420},{440},{460},
    {480},{500},{520},{540},{560},{600}
};

//======================================================
// Largeurs
//======================================================

constexpr BeltWidth HTD3MWidths[] =
{
    {6 ,0.0144f},
    {9 ,0.0216f},
    {15,0.0360f}
};

constexpr BeltWidth HTD5MWidths[] =
{
    {9 ,0.0351f},
    {15,0.0585f},
    {25,0.0975f},
    {40,0.1560f}
};

constexpr BeltWidth HTD8MWidths[] =
{
    {20,0.1160f},
    {30,0.1740f},
    {50,0.2900f},
    {85,0.4930f}
};

constexpr BeltWidth GT2Widths[] =
{
    {6 ,0.0084f},
    {9 ,0.0126f},
    {15,0.0210f}
};

constexpr BeltWidth GT3Widths3M[] =
{
    {6 ,0.0168f},
    {9 ,0.0252f},
    {15,0.0420f}
};

constexpr BeltWidth GT3Widths5M[] =
{
    {15,0.0615f},
    {25,0.1025f}
};

constexpr BeltWidth GT3Widths8M[] =
{
    {20,0.1160f},
    {30,0.1740f}
};

//======================================================
// Pas
//======================================================

constexpr BeltPitch HTDPitches[] =
{
    {
        "3M",
        HTD3MWidths,
        sizeof(HTD3MWidths)/sizeof(BeltWidth),
        L3M,
        sizeof(L3M)/sizeof(BeltLength)
    },

    {
        "5M",
        HTD5MWidths,
        sizeof(HTD5MWidths)/sizeof(BeltWidth),
        L5M,
        sizeof(L5M)/sizeof(BeltLength)
    },

    {
        "8M",
        HTD8MWidths,
        sizeof(HTD8MWidths)/sizeof(BeltWidth),
        L8M,
        sizeof(L8M)/sizeof(BeltLength)
    }
};

constexpr BeltPitch GT2Pitches[] =
{
    {
        "2M",
        GT2Widths,
        sizeof(GT2Widths)/sizeof(BeltWidth),
        L2M,
        sizeof(L2M)/sizeof(BeltLength)
    }
};

constexpr BeltPitch GT3Pitches[] =
{
    {
        "3M",
        GT3Widths3M,
        sizeof(GT3Widths3M)/sizeof(BeltWidth),
        L3M,
        sizeof(L3M)/sizeof(BeltLength)
    },

    {
        "5M",
        GT3Widths5M,
        sizeof(GT3Widths5M)/sizeof(BeltWidth),
        L5M,
        sizeof(L5M)/sizeof(BeltLength)
    },

    {
        "8M",
        GT3Widths8M,
        sizeof(GT3Widths8M)/sizeof(BeltWidth),
        L8M,
        sizeof(L8M)/sizeof(BeltLength)
    }
};

//======================================================
// Familles
//======================================================

constexpr BeltFamily BeltCatalog[] =
{
    {
        "HTD",
        HTDPitches,
        sizeof(HTDPitches)/sizeof(BeltPitch)
    },

    {
        "GT2",
        GT2Pitches,
        sizeof(GT2Pitches)/sizeof(BeltPitch)
    },

    {
        "GT3",
        GT3Pitches,
        sizeof(GT3Pitches)/sizeof(BeltPitch)
    }
};

constexpr uint8_t BeltCatalogCount =
sizeof(BeltCatalog)/sizeof(BeltFamily);

#endif

