#include "save_thread.h"
#include <cstring>
using namespace std;

void dump_file(const char* path, void* data, size_t size)
{
    FILE* fp = fopen(path, "wb");
    if(!fp)
        return;
    
    fwrite(data, 1, size, fp);
    fclose(fp);
}

void SaveBuffer::store(void* src, size_t byte_count)
{
    data.resize(byte_count);
    memcpy(&data[0], src, byte_count);
}

void SaveBuffer::save()
{
    
}

void SaveBuffer::clear()
{
    
}

