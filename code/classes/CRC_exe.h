#pragma once
#include "CRC_general.h"

class CRC_exe :
    public CRC_general
{
public:
    CRC_exe(const std::string& path = "");
    virtual ~CRC_exe();
    bool read_file(const std::string& path);
    bool any_key(uint32_t key) const;
    void replace_all_key(uint32_t key, uint32_t crc);
    bool write_file(const std::string& new_path) const;
protected:
    virtual int32_t sec_start(const IMAGE_SECTION_HEADER& sec) const override;
    virtual void get_inst() override;

private:
    std::vector<char> _fbuffer;
};

