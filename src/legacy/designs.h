#pragma once
#ifndef TPM_DESIGN_H
#define TPM_DESIGN_H

#include "legacy/design.h"

class Design5 : public Design
{
public:
    Design5(eDesign design, QString title);
    bool build();
};

class Design6 : public Design
{
public:
    Design6(eDesign design, QString title);
    bool build();
};

class Design7 : public Design
{
public:
    Design7(eDesign design, QString title);
    bool build();
};

class Design8 : public Design
{
public:
    Design8(eDesign design, QString title);
    bool build();
};

class Design9 : public Design
{
public:
    Design9(eDesign design, QString title);
    bool build();
};

class DesignHuPacked : public Design
{
public:
    DesignHuPacked(eDesign design, QString title);
    bool build();
};

class DesignHuInsert : public Design
{
public:
    DesignHuInsert(eDesign design, QString title);
    bool build();
};

class Design11 : public Design
{
public:
    Design11(eDesign design, QString title);
    bool build();
};

class Design12 : public Design
{
public:
    Design12(eDesign design, QString title);
    bool build();
};

class Design13 : public Design
{
public:
    Design13(eDesign design, QString title);
    bool build();
    //void repeat();
};

class Design14 : public Design
{
public:
    Design14(eDesign design, QString title);
    bool build();
    //void repeat();
};

class Design16 : public Design
{
public:
    Design16(eDesign design, QString title);
    bool build();
};

class Design17 : public Design
{
public:
    Design17(eDesign design, QString title);
    bool build();
};

class Design18 : public Design
{
public:
    Design18(eDesign design, QString title);
    bool build();
    //void repeat();
};

class Design19 : public Design
{
public:
    Design19(eDesign design, QString title);
    bool build();
};

class DesignKumiko1 : public Design
{
public:
    DesignKumiko1(eDesign design, QString title);
    bool build();
};

class DesignKumiko2 : public Design
{
public:
    DesignKumiko2(eDesign design, QString title);
    void init() override;
    bool build() override;
};

#endif //TPM_DESIGN_H
