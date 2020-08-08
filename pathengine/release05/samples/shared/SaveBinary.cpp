
#include "Header.h"
#include "SaveBinary.h"
#include "Error.h"
#include <stdio.h>

static bool
SaveBinary(const char* fileName, const char* buffer, tUnsigned32 bufferSize)
{
    FILE* fp = fopen(fileName, "wb");
    if(!fp)
    {
        const char* attributes[3];
        attributes[0] = "fileName";
        attributes[1] = fileName;
        attributes[2] = 0;
        Error("NonFatal", "SaveBinary: Failed to open file. Returning failure.", attributes);
        return false;
    }
    if(bufferSize > 0)
    {
        tSigned32 amountWritten = static_cast<tSigned32>(fwrite(buffer, 1, bufferSize, fp));
        if(amountWritten != bufferSize)
        {
            const char* attributes[3];
            attributes[0] = "fileName";
            attributes[1] = fileName;
            attributes[2] = 0;
            Error("NonFatal", "SaveBinary: Failed writing file.", attributes);
            fclose(fp);
            return false;
        }
    }
    int failure = fclose(fp);
    if(failure)
    {
        const char* attributes[3];
        attributes[0] = "fileName";
        attributes[1] = fileName;
        attributes[2] = 0;
        Error("NonFatal", "SaveBinary: Failed closing file.", attributes);
        return false;
    }
    return true;
}

bool
SaveBinary(const char* fileName, const std::vector<char>& buffer)
{
    if(buffer.empty())
    {
        return SaveBinary(fileName, 0, static_cast<tUnsigned32>(buffer.size()));
    }
    return SaveBinary(fileName, &buffer.front(), static_cast<tUnsigned32>(buffer.size()));
}
