#pragma once

class Tristate
{
public:

    enum class eTristate
    {
        False,
        True,
        Undefined
    };

    Tristate() { state = eTristate::Undefined; }

    void        set(eTristate state) { this->state = state; }
    void        reset() { state = eTristate::Undefined; }
    eTristate   get() { return state; }

private:
    eTristate state;
};
