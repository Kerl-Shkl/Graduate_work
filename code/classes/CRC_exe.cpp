#include "CRC_exe.h"

CRC_exe::CRC_exe(const std::string& path) :
    CRC_general(), _fbuffer()
{

    if (!path.empty()) {
        read_file(path);
    }

#if defined(_DEBUG ) || defined(DEBUG_PRINT_BYTESSUM)
    _log.open("log_exe.txt", std::ios::trunc);
#endif
}

CRC_exe::~CRC_exe()
{
#if defined(_DEBUG) || defined(DEBUG_PRINT_BYTESSUM)
    _log.close();
#endif
}

bool CRC_exe::read_file(const std::string& path)
{
    if (!_fbuffer.empty())
        _fbuffer.clear();
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return false;
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    _fbuffer.reserve(size);
    //_fbuffer.assign(std::istream_iterator<uint8_t>(file),
    //                std::istream_iterator<uint8_t>());
    _fbuffer.assign(std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
    return true;
}

bool CRC_exe::any_key(uint32_t key) const
{
    for (size_t i = 0; i < _fbuffer.size(); ++i) {
        uint32_t dw = *(MAKE_PTR(uint32_t*, _inst, i));
        if (dw == key) {
            return true;
        }
    }
    return false;
}

void CRC_exe::replace_all_key(uint32_t key, uint32_t crc)
{
    for (size_t i = 0; i < _fbuffer.size(); ++i) {
        uint32_t* pdw = MAKE_PTR(uint32_t*, _inst, i);
        if (*pdw == key) {
            *pdw = crc;
        }
    }
}

bool CRC_exe::write_file(const std::string& new_path) const
{
    std::ofstream output_file(new_path, std::ios::binary | std::ios::trunc);
    if (!output_file.is_open()) {
        return false;
    }
    std::ostreambuf_iterator<char> output_iterator(output_file);
    std::copy(_fbuffer.begin(), _fbuffer.end(), output_iterator);
    output_file.close();
    return true;
}

int32_t CRC_exe::sec_start(const IMAGE_SECTION_HEADER& sec) const
{
    return sec.PointerToRawData;
}

void CRC_exe::get_inst()
{
    _inst = reinterpret_cast<uint8_t*>(_fbuffer.data());
}
