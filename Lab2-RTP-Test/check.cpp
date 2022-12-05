#include <fstream>
#include <iostream>
#include <iomanip>
#include <string.h>
int main()
{
    std::ifstream file_1("save_int", std::ios::in | std::ios::binary);
    std::ifstream file_2("testdata", std::ios::in | std::ios::binary);
    char buf_1[8];
    char buf_2[8];
    int size = 0;
    while (true)
    {
        memset(buf_1, 0, 8);
        memset(buf_2, 0, 8);
        file_1.read(buf_1, 8);
        file_2.read(buf_2, 8);
        if (file_1.gcount() == 0 || file_2.gcount() == 0)
            break;
        uint64_t n1 = *(reinterpret_cast<uint64_t *>(buf_1));
        uint64_t n2 = *(reinterpret_cast<uint64_t *>(buf_2));
        if (n1 != n2)
        {
            std::cout << "byte " << std::dec << size << ": " << std::hex << std::setw(16) << std::setfill('0') << n1 << ' ' << std::hex << std::setw(16) << std::setfill('0') << n2 << ' ' << '\n';
        }
        size += file_2.gcount();
    }
    file_1.close();
    file_2.close();
    return 0;
}