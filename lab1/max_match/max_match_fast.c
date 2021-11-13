#include "max_match_fast.h"
#include "../hashmap/hashmap.h"
#include "../stack/stack.h"
#include "../string/str.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <wchar.h>

struct hashmap* read_dic_fast(const char*    filename,
                              unsigned long* max_word_len) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("open");
    }

    // 词典
    struct hashmap* map = hashmap_init();

    *max_word_len = 0;

    char line[102400] = {0};
    while (fgets(line, sizeof line, fp)) {
        // 获取 key(词) - value(词频)
        char  key[1024]     = {0};
        char  value_str[20] = {0};
        int   value;
        char* token = strtok(line, " ");
        strcpy(key, token);
        token = strtok(NULL, " ");
        strcpy(value_str, token);
        strpop(value_str);
        value = atoi(value_str);

        // 插入词典
        wchar_t wkey[1024] = {0};
        cs2wcs(wkey, key);
        put(map, wkey, value);

        // 更新最大词长度
        unsigned long word_len = wcslen(wkey);
        if (*max_word_len < word_len) {
            *max_word_len = word_len;
        }
    }

    fclose(fp);

    return map;
}

void fmm_fast(const char* src_filename, const char* fmm_filename,
              struct hashmap* map, unsigned long max_word_len) {
    // 前向最大匹配分词结果文件
    FILE* fmm_fp = fopen(fmm_filename, "w");
    if (!fmm_fp) {
        perror("open");
    }

    // 待分词文件
    FILE* src_fp = fopen(src_filename, "r");
    if (!src_fp) {
        perror("open");
    }

    char raw_line[10240] = {0};
    while (fgets(raw_line, sizeof raw_line, src_fp)) {
        // 删除空白符、换行符、日期
        strtrim(raw_line);
        char line[102400] = {0};
        strmncpy(line, raw_line, strlen("19980101-01-001-001"),
                 (int)strlen(raw_line));

        // 转 wchar_t
        wchar_t line_w[102400] = {0};
        cs2wcs(line_w, line);

        for (int i = 0; i < wcslen(line_w);) {
            for (int j = (int)max_word_len; j > 0; j -= 1) {
                wchar_t select_word_w[1024] = {0};
                wcsmncpy(select_word_w, line_w, i, i + j);

                // 查询词典
                char select_word[1024] = {0};
                wcs2cs(select_word, select_word_w);
                if (get(map, select_word_w)) {
                    fprintf(fmm_fp, "%s/ ", select_word);
                    i += j;
                    break;
                }

                if (j == 1) {
                    fprintf(fmm_fp, "%s/ ", select_word);
                    i += 1;
                }
            }
        }

        fprintf(fmm_fp, "\n");
    }

    fclose(src_fp);
    fclose(fmm_fp);
}

void bmm_fast(const char* src_filename, const char* bmm_filename,
              struct hashmap* map, unsigned long max_word_len) {
    // 反向最大匹配分词结果文件
    FILE* bmm_fp = fopen(bmm_filename, "w");
    if (!bmm_fp) {
        perror("open");
    }

    // 待分词文件
    FILE* src_fp = fopen(src_filename, "r");
    if (!src_fp) {
        perror("open");
    }

    // 反向最大匹配的分词结果是倒着的，需要用栈进行倒序
    struct stack* stack = stack_init();

    char raw_line[10240] = {0};
    while (fgets(raw_line, sizeof raw_line, src_fp)) {
        // 删除空白符、换行符、日期
        strtrim(raw_line);
        char line[102400] = {0};
        strmncpy(line, raw_line, strlen("19980101-01-001-001"),
                 (int)strlen(raw_line));

        // 转 wchar_t
        wchar_t line_w[102400] = {0};
        cs2wcs(line_w, line);

        for (int i = wcslen(line_w); i >= 0;) {
            for (int j = (int)max_word_len; j > 0; j -= 1) {
                wchar_t select_word_w[1024] = {0};
                wcsmncpy(select_word_w, line_w, i - j, i);

                // 查询词典
                char select_word[1024] = {0};
                wcs2cs(select_word, select_word_w);
                if (get(map, select_word_w)) {
                    push(stack, select_word_w);
                    i -= j;
                    break;
                }

                if (j == 1) {
                    push(stack, select_word_w);
                    i -= 1;
                }
            }
        }

        // 倒序打印
        while (!is_empty(stack)) {
            wchar_t top_w[1024] = {0};
            pop(stack, top_w);
            if (!wcscmp(top_w, L"")) {
                break;
            }
            char top[10240] = {0};
            wcs2cs(top, top_w);
            fprintf(bmm_fp, "%s/ ", top);
        }
        fprintf(bmm_fp, "\n");
    }

    fclose(src_fp);
    fclose(bmm_fp);
}

struct max_match_time* max_match_fast(const char* dic_file,
                                      const char* origin_data_file,
                                      const char* seg_fmm_file,
                                      const char* seg_bmm_file) {
    unsigned long   max_word_len = 0;
    struct hashmap* map          = read_dic_fast(dic_file, &max_word_len);

    // fmm 所用时间
    time_t fmm_start, fmm_end;
    fmm_start = clock();
    fmm_fast(origin_data_file, seg_fmm_file, map, max_word_len);
    fmm_end = clock();

    // bmm 所用时间
    time_t bmm_start, bmm_end;
    bmm_start = clock();
    bmm_fast(origin_data_file, seg_bmm_file, map, max_word_len);
    bmm_end = clock();

    struct max_match_time* mm_time =
        (struct max_match_time*)malloc(sizeof(struct max_match_time));
    mm_time->fmm_time = (double)(fmm_end - fmm_start) / CLOCKS_PER_SEC;
    mm_time->bmm_time = (double)(bmm_end - bmm_start) / CLOCKS_PER_SEC;

    return mm_time;
}
