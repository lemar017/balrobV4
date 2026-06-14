#ifndef SDCARD_MANAGER_H
#define SDCARD_MANAGER_H

#include "SD.h"

class SDCardManager {
public:
    SDCardManager(uint8_t csPin = 5);

    bool begin();

    String readFile(const char* path);
    bool writeFile(const char* path, const String& data);
    bool deleteFile(const char* path);

    bool createDirectory(const char* path);
    bool deleteDirectory(const char* path, bool recursive = false);

private:
    uint8_t _csPin;
    bool _initialized;

    bool _checkInit();
    bool _createParentDirectories(const char* filepath);
    bool _deleteDirectoryRecursive(const char* path);
};

#endif
