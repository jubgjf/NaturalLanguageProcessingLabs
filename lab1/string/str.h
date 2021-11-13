#pragma once

#include <time.h>
#include <wchar.h>

/**
 * @brief 删除字符串最后一个字符
 *
 * @param str 字符串
 */
void strpop(char* str);

/**
 * @brief 删除字符串行尾换行符
 *
 * @param str 字符串
 */
void strtrim(char* str);

/**
 * @brief 从 \p src 的左闭右开区间 [ \p begin, \p end ] 复制字符到 \p dst 中
 *
 * @param dst 复制目的
 * @param src 复制源
 * @param begin 起始位置
 * @param end 结束位置
 */
void strmncpy(char* dst, char* src, int begin, int end);

/**
 * @brief 从 \p src 的左闭右开区间 [ \p begin, \p end ] 复制字符到 \p dst 中
 *
 * @param dst 复制目的
 * @param src 复制源
 * @param begin 起始位置
 * @param end 结束位置
 */
void wcsmncpy(wchar_t* dst, wchar_t* src, int begin, int end);

/**
 * @brief 将 char* 字符串转换为 wchar_t* 字符串
 *
 * @param dst wchar_t* 字符串
 * @param src char* 字符串
 */
void cs2wcs(wchar_t* dst, char* src);

/**
 * @brief 将 char* 字符串转换为 wchar_t* 字符串
 *
 * @param dst char* 字符串
 * @param src wchar_t* 字符串
 */
void wcs2cs(char* dst, wchar_t* src);
