#ifndef _PRINT_
#define _PRINT_

#include "stdio.h"
#include "stdarg.h"
#include "Arduino.h"

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static LogLevel  mLevel;
    Logger(LogLevel level){mLevel = level;}

    static void dumpHex(const char *prefix, const byte *buffer, byte len)
    {
        if (mLevel > DEBUG)
            return;
        Serial.print(","+ String(prefix)+ ":");
        // Serial.print(":");
        for (int i = 0; i < len; i++) {
            Serial.print("0x");
            Serial.print(buffer[i], HEX);
            if (i < len - 1) Serial.print(", ");
        }
        Serial.println();
    }

    static void print(LogLevel level, const char* format, ...)
    {
        va_list argptr;
        char buffer[256] ={0}; 
        if (level < mLevel)
            return;
        switch(level) { 
            case DEBUG:
                Serial.print("[D]: ");
                break;
            case INFO:
                Serial.print("[I]: ");
                break;
            case WARNING:
                Serial.print("[W]: ");
                break;
            case ERROR:
                Serial.print("[E]: ");
                break;
        };
        va_start(argptr, format);
        vsnprintf(buffer, sizeof(buffer), format, argptr);
        Serial.print(buffer);
        va_end(argptr);
        Serial.println();
    }

    static void setLevel(LogLevel level)
    {
        mLevel = level;
    }
};

#endif
