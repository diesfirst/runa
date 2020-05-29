#include <cstdlib>
#include <cassert>
#include <cstring>
#include <stdio.h>

namespace sword
{

class Region
{
public:
    Region(size_t initialSize)
    {
        handle = (unsigned char*)malloc(initialSize);
        maxSize = initialSize;
    }

    void copyTo(void* src, int size)
    {
        while (offset + size > maxSize)
        {
            grow();
        }
        unsigned char* pos = handle + offset;
        memcpy(pos, src, size);
        offset += size;
        itemCount++;
        assert(itemCount <= max_item_count);
        itemSizes[itemCount - 1] = size;
    }

    void ripFrom(void* dest, int size)
    {
         if (size + offset > maxSize)
         {
             printf("Not enough data in region to rip from.\n");
             return;
         }
         unsigned char* pos = handle + offset;
         memcpy(dest, handle, size);
         assert(itemCount > 0);
         offset -= itemSizes[itemCount - 1];
         itemCount--;
    }

    ~Region()
    {
        assert(handle);
        free(handle);
    }
    
private:
    static constexpr int max_item_count = 10;
    void grow()
    {
        void* newHandle = realloc(handle, maxSize * 2);
        assert(newHandle && "Reallocation failed");
        handle = (unsigned char*)newHandle;
        maxSize *= 2;
    }
    unsigned char* handle{nullptr};
    int maxSize{0};
    int itemCount{0};
    int offset{0};
    int itemSizes[max_item_count];
};

}; // namespace sword
