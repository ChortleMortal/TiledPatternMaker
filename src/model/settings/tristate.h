#pragma once
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

    Tristate() { state = eTristate::Unknown; }

    void        set(eTristate state) { this->state = state; }
    void        reset() { state = eTristate::Unknown; }
    eTristate   get() { return state; }

private:
    eTristate state;
};

#endif // TRISTATE_H
