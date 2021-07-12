#ifndef TRISTATE_H
#define TRISTATE_H

class Tristate
{
public:

    enum eTristate
    {
        False,
        True,
        Unknown
    };

    Tristate() { state = Unknown; }

    void        set(bool b) { if (b) state = True; else state = False; }
    void        reset() { state = Unknown; }
    eTristate   get() { return state; }

private:
    eTristate state;
};

#endif // TRISTATE_H
