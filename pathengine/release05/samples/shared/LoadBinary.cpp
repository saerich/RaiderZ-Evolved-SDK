
#include "LoadBinary.h"
#include "Error.h"
#include <stdio.h>

bool
FileExists(const char* fileName)
{
    FILE* fp = fopen(fileName, "rb");
    if(!fp)
    {
		return false;
    }
    int failed = fclose(fp);
    assertD(!failed);
	return true;
}

void
LoadBinary(const char* fileName, std::vector<char>& buffer)
{
    buffer.clear();

    FILE* fp = fopen(fileName, "rb");
    if(!fp)
    {
        Error("NonFatal", "loadBinary: Failed to open file. Returning an empty buffer.", 0);
        return;
    }

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer.resize(size);
    if(size)
    {
        fread(&buffer.front(), 1, size, fp);
    }
    fclose(fp);
}
