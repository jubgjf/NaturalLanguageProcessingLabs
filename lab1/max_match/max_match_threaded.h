#pragma once

#include "timer.h"
#include <wchar.h>

/**
 * 待分词文件行数
 */
#define TOTAL_LINES 23032

/**
 * 线程数
 */
#define THREAD_COUNT 2000

/**
 * 子线程参数
 */
struct thread_param {
    wchar_t*       line_w;       // 当前行内容
    unsigned*      lineno;       // 当前行号
    unsigned*      thread_index; // 子线程序号
    unsigned long* max_word_len; // 字典中词的最大长度
};

/**
 * @brief 前向最大匹配依次查找字典时的子线程；
 *        需要待分词文件末尾空一行
 *
 * @param tp_v 子线程参数，实际类型为 struct thread_param
 */
void thread(void* tp_v);

/**
 * @brief 从 \p filename 读取词典文件到 hashmap 中
 *
 * @param filename 词典文件路径
 * @param max_word_len 字典中最大词长度将被存放于 \p max_word_len
 * @return struct hashmap* 返回词典
 */
struct hashmap* read_dic_threaded(const char*    filename,
                                  unsigned long* max_word_len);

/**
 * @brief 对文件 \p src_filename 进行前向最大匹配，默认将分词结果打印到终端
 *
 * @param src_filename 待分词文件
 * @param max_word_len 字典中词的最大长度
 */
void fmm_threaded(const char* src_filename, unsigned long max_word_len);

/**
 * @brief 基于机械匹配的最大分词
 *
 * @param dic_file 字典文件路径
 * @param origin_data_file 待分词文件路径
 */
void max_match_threaded(const char* dic_file, const char* origin_data_file);
