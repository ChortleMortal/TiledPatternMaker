#pragma once

#include <QString>

#define LINUX_MEDIA_FILES_VERSION 6

class TpmVersion
{
public:
    static QString getVersion();
    static QString getTimestamp();
};


