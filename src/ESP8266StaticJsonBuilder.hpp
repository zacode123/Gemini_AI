/*
 * ESP8266StaticJsonBuilder.hpp - A fast, zero-allocation static JSON builder for ESP8266/ESP32 using fixed-size character buffers.
 *
 * Supports deeply nested JSON objects and arrays, various primitive types, and macro-driven syntax for easy usage.
 * Specifically optimized for memory-constrained microcontrollers, avoiding all heap usage by working with caller-provided buffers.
 *
 * This library avoids all dynamic memory allocation (no malloc/new/String) and builds JSON into a user-provided `char[]` buffer
 * using only fixed stack allocations and static template dispatch for serialization.
 *
 * Features:
 * - Zero heap allocations (100% stack/static)
 * - Optional pretty-printing
 * - Configurable float precision and max nesting depth
 * - Support for strings, integers, floats, booleans, null, and arrays
 * - Safe for very low-RAM devices like ESP8266
 * - Simple macro wrappers to manage JSON structure
 *
 * MIT License
 * Created by zacode123, 18-07-2025
 * Version 2.2.0 (Improved user simplicity)
 */

#pragma once

#ifndef ESP8266JSON_FLOAT_PRECISION
#define ESP8266JSON_FLOAT_PRECISION 2
#endif

#ifndef ESP8266JSON_MAX_DEPTH
#define ESP8266JSON_MAX_DEPTH 10
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

class ESP8266StaticJsonBuilder {
private:
    char* buffer;
    size_t capacity;
    size_t position;
    uint8_t depth;
    bool isFirst[ESP8266JSON_MAX_DEPTH + 1];
    bool justWroteKey[ESP8266JSON_MAX_DEPTH + 1];
    char containerType[ESP8266JSON_MAX_DEPTH + 1];
    bool pretty;

    void writeChar(char c) {
        if (position + 1 >= capacity) return;
        buffer[position++] = c;
        buffer[position] = '\0';
    }

    void writeLiteral(const char* str) {
        for (size_t i = 0; str[i] != '\0'; ++i) {
            if (position + 1 >= capacity) return;
            buffer[position++] = str[i];
        }
        buffer[position] = '\0';
    }

    void indent() {
        if (!pretty) return;
        for (uint8_t i = 0; i < depth; ++i) {
            writeLiteral("  ");
        }
    }

    void newline() {
        if (pretty) writeChar('\n');
    }

    void writeSeparator() {
      if (!justWroteKey[depth]) {
        if (!isFirst[depth]) writeChar(',');
        newline();
        indent();
        isFirst[depth] = false;
      } else {
        justWroteKey[depth] = false;
      }
    }

    void serializeString(const char* str) {
        writeChar('"');
        if (str) {
            for (size_t i = 0; str[i] != '\0'; ++i) {
                if (position + 2 >= capacity) break;
                switch (str[i]) {
                    case '"': writeLiteral("\\\""); break;
                    case '\\': writeLiteral("\\\\"); break;
                    case '\n': writeLiteral("\\n"); break;
                    case '\t': writeLiteral("\\t"); break;
                    case '\r': writeLiteral("\\r"); break;
                    default: writeChar(str[i]); break;
                }
            }
        }
        writeChar('"');
    }

    void serializeNull() { writeLiteral("null"); }
    void serializeBool(bool v) { writeLiteral(v ? "true" : "false"); }
    void serializeInt(long v) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%ld", v);
        writeLiteral(buf);
    }
    void serializeUInt(unsigned long v) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%lu", v);
        writeLiteral(buf);
    }
    void serializeFloat(float v) {
        char buf[32];
        dtostrf(v, 0, ESP8266JSON_FLOAT_PRECISION, buf);
        writeLiteral(buf);
    }

    template <typename T>
    void serializeValue(const T& v);

public:
    ESP8266StaticJsonBuilder(char* buf, size_t cap, bool prettyPrint = false)
        : buffer(buf), capacity(cap), position(0), depth(0), pretty(prettyPrint) {
        memset(isFirst, 1, sizeof(isFirst));
        memset(justWroteKey, 0, sizeof(justWroteKey));
        memset(containerType, 0, sizeof(containerType));
        if (capacity > 0) buffer[0] = '\0';
    }

    void setPretty(bool enable) { pretty = enable; }

    void beginObject() {
        if (depth >= ESP8266JSON_MAX_DEPTH) return;
        writeSeparator();
        writeChar('{');
        if (pretty) {
            ++depth;
            newline();
            indent();
        }
        isFirst[depth] = true;
        containerType[depth] = 'O';
    }

    void beginArray() {
        if (depth >= ESP8266JSON_MAX_DEPTH) return;
        writeSeparator();
        writeChar('[');
        if (pretty) {
            ++depth;
            newline();
            indent();
        }
        isFirst[depth] = true;
        containerType[depth] = 'A';
    }

    void endObject() {
        uint8_t d = depth;
        if (pretty && depth > 0) --depth;
        if (!isFirst[d]) {
            newline();
            indent();
        }
        writeChar('}');
        isFirst[depth] = false;
    }

    void endArray() {
        uint8_t d = depth;
        if (pretty && depth > 0) --depth;
        if (!isFirst[d]) {
            newline();
            indent();
        }
        writeChar(']');
        isFirst[depth] = false;
    }

    void key(const char* k) {
        writeSeparator();
        serializeString(k);
        writeChar(':');
        if (pretty) writeChar(' ');
        isFirst[depth] = false;
        justWroteKey[depth] = true;
    }

    void value(const char* v) { writeSeparator(); serializeString(v); isFirst[depth] = false; }
    void value(const String& v) { writeSeparator(); serializeString(v.c_str()); isFirst[depth] = false; }
    void value(bool v) { writeSeparator(); serializeBool(v); isFirst[depth] = false; }
    void value(void* v) { (void)v; writeSeparator(); serializeNull(); isFirst[depth] = false; }
    void value(float v) { writeSeparator(); serializeFloat(v); isFirst[depth] = false; }
    void value(double v) { writeSeparator(); serializeFloat((float)v); isFirst[depth] = false; }
    void value(char v) { writeSeparator(); writeChar(v); isFirst[depth] = false; }
    void value(unsigned char v) { writeSeparator(); serializeUInt(v); isFirst[depth] = false; }
    void value(int v) { writeSeparator(); serializeInt(v); isFirst[depth] = false; }
    void value(unsigned int v) { writeSeparator(); serializeUInt(v); isFirst[depth] = false; }
    void value(long v) { writeSeparator(); serializeInt(v); isFirst[depth] = false; }
    void value(unsigned long v) { writeSeparator(); serializeUInt(v); isFirst[depth] = false; }

#if defined(ESP8266) || defined(ESP32)
    void value(long long v) {
        char buf[21];
        snprintf(buf, sizeof(buf), "%lld", v);
        writeSeparator();
        writeLiteral(buf);
        isFirst[depth] = false;
    }

    void value(unsigned long long v) {
        char buf[21];
        snprintf(buf, sizeof(buf), "%llu", v);
        writeSeparator();
        writeLiteral(buf);
        isFirst[depth] = false;
    }
#endif

    template <typename T>
    void value(const T arr[], size_t len) {
        beginArray();
        for (size_t i = 0; i < len; ++i) {
            value(arr[i]);
        }
        endArray();
    }

    const char* c_str() const { return buffer; }
    size_t length() const { return position; }
};
