#include "max_match.h"
#include "../stack/stack.h"
#include "../string/str.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <wchar.h>

struct dict* read_dic(const char* filename, unsigned long* max_word_len) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("open");
    }

    // 词典
    struct dict* dic = (struct dict*)malloc(sizeof(struct dict));

    *max_word_len = 0;

    char line[102400] = {0};
    while (fgets(line, sizeof line, fp)) {
        // 获取词
        char  key[1024] = {0};
        char* token     = strtok(line, " ");
        strcpy(key, token);

        // 插入词典
        wchar_t wkey[1024] = {0};
        cs2wcs(wkey, key);
        put_dict(dic, wkey);

        // 更新最大词长度
        unsigned long word_len = strlen(key);
        if (*max_word_len < word_len) {
            *max_word_len = word_len;
        }
    }

    fclose(fp);

    return dic;
}

int search_dict(struct dict* dic, wchar_t* word) {
    struct dict* p = dic;

    while (p) {
        if (!wcscmp(p->word, word)) {
            return 1;
        } else {
            p = p->next;
        }
    }

    return 0;
}

void put_dict(struct dict* dic, wchar_t* word) {
    struct dict* p = (struct dict*)malloc(sizeof(struct dict));
    wcscpy(p->word, word);
    p->next   = dic->next;
    dic->next = p;
}

void fmm(const char* src_filename, const char* fmm_filename, struct dict* dic,
         unsigned long max_word_len) {
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

    char line[10240] = {0};
    while (fgets(line, sizeof line, src_fp)) {
        // 删除空白符、换行符
        strtrim(line);

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
                if (search_dict(dic, select_word_w)) {
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

void bmm(const char* src_filename, const char* bmm_filename, struct dict* dic,
         unsigned long max_word_len) {
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

    char line[10240] = {0};
    while (fgets(line, sizeof line, src_fp)) {
        // 删除空白符、换行符
        strtrim(line);

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
                if (search_dict(dic, select_word_w)) {
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
                continue;
            }
            char top[10240] = {0};
            wcs2cs(top, top_w);
            fprintf(bmm_fp, "%s/ ", top);

            if (is_empty(stack)) {
                break;
            }
        }
        fprintf(bmm_fp, "\n");
    }

    fclose(src_fp);
    fclose(bmm_fp);
}

struct max_match_time* max_match(const char* dic_file,
                                 const char* origin_data_file,
                                 const char* seg_fmm_file,
                                 const char* seg_bmm_file) {
    unsigned long max_word_len = 0;
    struct dict*  dic          = read_dic(dic_file, &max_word_len);

    // fmm 所用时间
    time_t fmm_start, fmm_end;
    fmm_start = clock();
    fmm(origin_data_file, seg_fmm_file, dic, max_word_len);
    fmm_end = clock();

    // bmm 所用时间
    time_t bmm_start, bmm_end;
    bmm_start = clock();
    bmm(origin_data_file, seg_bmm_file, dic, max_word_len);
    bmm_end = clock();

    struct max_match_time* mm_time =
        (struct max_match_time*)malloc(sizeof(struct max_match_time));
    mm_time->fmm_time = (double)(fmm_end - fmm_start) / CLOCKS_PER_SEC;
    mm_time->bmm_time = (double)(bmm_end - bmm_start) / CLOCKS_PER_SEC;

    return mm_time;
}
