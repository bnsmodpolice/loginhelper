#pragma once

struct Inputs
{
    int unknown1;
    int number;
};
struct Numpads
{
    int unknown1;
    int number;
};

class PinClass
{
public:
#ifdef _M_X64
    char Padding_1[0xB0];
    // char Padding_1a[6 * 8];
#else
    char Padding_1[0x6C];
    // char Padding_1a[6 * 4];
#endif
    struct Inputs inputs[8];
    struct Numpads numpads[10];
    char Padding_2[0x1C];
    int failCount;
    int inputCount;
    char Padding_3[0xC];
    bool useKeyboard;
};
