#include "str.h"
#include <string.h>
#include <wchar.h>

void strpop(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    str[i - 1] = '\0';
}

void strtrim(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    if (str[i - 1] == '\n' || str[i - 1] == '\r') {
        str[i - 1] = '\0';
        strtrim(str);
    }
}

void strmncpy(char* dst, char* src, int begin, int end) {
    if (begin < 0) {
        begin = 0;
    } else if (strlen(src) < end) {
        end = strlen(src);
    } else if (strlen(src) - 1 < begin) {
        src = NULL;
        return;
    }

    for (int src_i = begin, dst_i = 0; src_i < end; src_i++, dst_i++) {
        dst[dst_i] = src[src_i];
    }
}

void wcsmncpy(wchar_t* dst, wchar_t* src, int begin, int end) {
    if (begin < 0) {
        begin = 0;
    } else if (wcslen(src) < end) {
        end = wcslen(src);
    } else if (wcslen(src) - 1 < begin) {
        src = NULL;
        return;
    }

    for (int src_i = begin, dst_i = 0; src_i < end; src_i++, dst_i++) {
        dst[dst_i] = src[src_i];
    }
}

void cs2wcs(wchar_t* dst, char* src) {
    char*    p   = (char*)src;
    wchar_t* wpt = dst;
    while (*p) {
        if (*p & 0x80) {
            if (*p & 0x20) {
                if (*p & 0x10) {
                    for (int i = 0; i < 4; i++, p++) {
                        if (i == 0)
                            *wpt = (*p & 0x07);
                        else {
                            *wpt = *wpt * 0x40 + (*p & 0x3F);
                        }
                    }
                } else {
                    for (int i = 0; i < 3; i++, p++) {
                        if (i == 0)
                            *wpt = (*p & 0x0F);
                        else {
                            *wpt = *wpt * 0x40 + (*p & 0x3F);
                        }
                    }
                }
            } else {
                for (int i = 0; i < 2; i++, p++) {
                    if (i == 0)
                        *wpt = (*p & 0x1F);
                    else {
                        *wpt = *wpt * 0x40 + (*p & 0x3F);
                    }
                }
            }
        } else {
            *wpt = (wchar_t)*p & 0x7F;
            p++;
        }
        wpt++;
    }
    *wpt = L'\0';
}

void wcs2cs(char* dst, wchar_t* src) {
    char*    pt  = dst;
    wchar_t* wpt = (wchar_t*)src;
    while (*wpt) {
        int temp = *wpt;
        if (*wpt < 0x80) {
            *pt++ = (char)temp;
        } else if (*wpt < 0x800) {
            *pt++ = 0xC0 + temp / 0x40;
            temp  = temp % 0x40;
            *pt++ = 0x80 + temp;
        } else if (*wpt < 0x10000) {
            *pt++ = 0xE0 + temp / 0x1000;
            temp  = temp % 0x1000;
            *pt++ = 0x80 + temp / 0x40;
            temp  = temp % 0x40;
            *pt++ = 0x80 + temp;
        } else {
            *pt++ = 0xF0 + temp / 0x40000;
            temp  = temp % 0x40000;
            *pt++ = 0x80 + temp / 0x1000;
            temp  = temp % 0x1000;
            *pt++ = 0x80 + temp / 0x40;
            temp  = temp % 0x40;
            *pt++ = 0x80 + temp;
        }
        wpt++;
    }
    *pt++ = '\0';
}
