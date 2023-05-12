#pragma once
#include "CRC_general.h"

class CRC_image :
    public CRC_general
{
public:
    CRC_image();
    ~CRC_image();
protected:
    virtual int32_t sec_start(const IMAGE_SECTION_HEADER& sec) const override;
    virtual void get_inst() override;
};

