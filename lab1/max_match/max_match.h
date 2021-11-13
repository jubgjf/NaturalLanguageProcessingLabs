#pragma once

#include "timer.h"
#include <wchar.h>

/**
 * @brief 字典的一个条目，指针形式
 */
struct dict {
    wchar_t      word[100];
    struct dict* next;
};

/**
 * @brief 从 \p filename 读取词典文件到 dict 中
 *
 * @param filename 词典文件路径
 * @param max_word_len 字典中最大词长度将被存放于 \p max_word_len
 * @return struct dict* 返回词典
 */
struct dict* read_dic(const char* filename, unsigned long* max_word_len);

/**
 * @brief 从词典 \p dic 中搜索单词 \p w_word
 *
 * @param dic 词典
 * @param word 单词
 * @return int 若找到，则返回 1；否则返回 0
 */
int search_dict(struct dict* dic, wchar_t* word);

/**
 * @brief 将 \p w_word 放入词典 \p dic
 */
void put_dict(struct dict* dic, wchar_t* word);

/**
 * @brief 基于词典 \p dic 对文件 \p src_filename 进行前向最大匹配，
 *        分词结果存放于 \p fmm_filename
 *
 * @param src_filename 待分词文件
 * @param fmm_filename 分词结果文件
 * @param dic 字典
 * @param max_word_len 字典中词的最大长度
 */
void fmm(const char* src_filename, const char* fmm_filename, struct dict* dic,
         unsigned long max_word_len);

/**
 * @brief 基于词典 \p dic 对文件 \p src_filename 进行反向最大匹配，
 *        分词结果存放于 \p fmm_filename
 *
 * @param src_filename 待分词文件
 * @param bmm_filename 分词结果文件
 * @param dic 字典
 * @param max_word_len 字典中词的最大长度
 */

void bmm(const char* src_filename, const char* bmm_filename, struct dict* dic,
         unsigned long max_word_len);

/**
 * @brief 基于机械匹配的最大分词
 *
 * @param dic_file 字典文件路径
 * @param origin_data_file 待分词文件路径
 * @param seg_fmm_file 前向最大匹配结果路径
 * @param seg_bmm_file 反向最大匹配结果路径
 * @return struct max_match_time 正反向最大匹配分词所用时间
 */
struct max_match_time* max_match(const char* dic_file,
                                 const char* origin_data_file,
                                 const char* seg_fmm_file,
                                 const char* seg_bmm_file);
