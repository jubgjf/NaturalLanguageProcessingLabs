#include "max_match_threaded.h"
#include "../hashmap/hashmap.h"
#include "../stack/stack.h"
#include "../string/str.h"
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// stdio 方面的读写锁
pthread_rwlock_t io_lock;

// 子线程传参时，给参数的读写锁
pthread_rwlock_t tp_lock;

// curr_write_lineno 的读写锁
pthread_rwlock_t cwl_lock;

// 当前应该处理的文本行号
unsigned curr_write_lineno = 1;

// 各个线程向 stdio 写时的互斥锁
pthread_mutex_t line_mutex[THREAD_COUNT];

// 哈希表
// 由于在各个子进程中哈希表只读不写，因此省去读写锁
struct hashmap* map;

void thread(void* tp_v) {
    // 从 tp_v 解析传递给本线程的参数
    pthread_rwlock_rdlock(&tp_lock);
    struct thread_param tp = *((struct thread_param*)tp_v); // 参数结构体
    unsigned            index = *tp.thread_index;           // 子线程序号
    wchar_t* line_w = malloc(sizeof(wchar_t) * 102400);     // 当前行内容
    wcscpy(line_w, tp.line_w);
    unsigned      lineno       = *tp.lineno;       // 当前行号
    unsigned long max_word_len = *tp.max_word_len; // 字典中词的最大长度
    pthread_rwlock_unlock(&tp_lock);

    // FMM 中最耗时的部分，即依次缩短单词长度，查找字典
    char* seg_result = malloc(sizeof(char) * 102400);
    memset(seg_result, 0, sizeof(char) * 102400);
    unsigned long len = wcslen(line_w);
    for (int i = 0; i < len;) {
        for (int j = (int)max_word_len; j > 0; j -= 1) {
            wchar_t select_word_w[1024] = {0};
            wcsmncpy(select_word_w, line_w, i, i + j);

            // 查询词典
            char select_word[1024] = {0};
            wcs2cs(select_word, select_word_w);
            if (get(map, select_word_w)) {
                seg_result = strcat(seg_result, select_word);
                seg_result = strcat(seg_result, "/ ");
                i += j;
                break;
            }

            if (j == 1) {
                seg_result = strcat(seg_result, select_word);
                seg_result = strcat(seg_result, "/ ");
                i += 1;
            }
        }
    }
    seg_result = strcat(seg_result, "\n");

    // 按照行号顺序依次输出分词结果
    //
    // 由于线程的随机调度，本函数最复杂的地方就是顺序输出每一行
    //
    // 思路如下：
    // 对于一共 THREAD_COUNT 个线程（以下用 n 代替），每个线程都绑定了
    // 一行字符串和对应的行号。首先定义全局变量 curr_write_lineno，
    // 其只能顺序增长，表示期望输出的行的行号。
    // 再对这 n 个线程（即 n 个无序的行）定义 n 个互斥锁，互斥锁锁定时代表
    // 当前行不是期望的输出，打开时则代表当前行可以被输出。这样的好处是当一个
    // 不该被输出的行想要输出时，对应的进程会被互斥锁阻塞，直到轮到它输出时自动从阻塞恢复。
    //
    //  lock     lock    unlock    lock     lock     lock
    // -------  -------  -------  -------  -------  -------
    // |  1  |  |  2  |  |  3  |  |  4  |  |  5  |  |  6  |  ......
    // -------  -------  -------  -------  -------  -------
    //                      ^
    //                      |
    //                      curr_write_lineno
    //
    // 因此，当需要输出一行时，首先需要尝试锁定互斥锁。
    // 如果当前行不应该输出，则自动被阻塞；否则可以正常向下检验行号以及执行输出语句。
    //
    // 由于线程的序号 thread_index 是重复使用的，因此上述的这些互斥锁
    // 在逻辑上是一个环形结构。
    // 当输出结束后，应该解锁下一个互斥锁，表示下一行可以被输出了。
    // 如果下一个互斥锁的数组下标超出范围，则应该重置为 0，以体现出环形结构。
    pthread_mutex_lock(&line_mutex[index]);
    pthread_rwlock_wrlock(&cwl_lock);
    if (curr_write_lineno == lineno) {
        curr_write_lineno++;
        pthread_rwlock_unlock(&cwl_lock);

        // 输出分词结果
        char* line = malloc(sizeof(char) * 102400);
        wcs2cs(line, line_w);
        pthread_rwlock_wrlock(&io_lock);
        fprintf(stdout, "%s", seg_result);
        pthread_rwlock_unlock(&io_lock);

        if (index + 1 < THREAD_COUNT) {
            // 解锁下一个互斥锁
            pthread_mutex_unlock(&line_mutex[index + 1]);
        } else {
            // 环形结构，回到开始
            pthread_mutex_unlock(&line_mutex[0]);
        }
    }

    pthread_exit(NULL);
}

struct hashmap* read_dic_threaded(const char*    filename,
                                  unsigned long* max_word_len) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("open");
    }

    // 词典
    map = hashmap_init();

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

void fmm_threaded(const char* src_filename, unsigned long max_word_len) {
    // 待分词文件
    FILE* src_fp = fopen(src_filename, "r");
    if (!src_fp) {
        perror("open");
    }

    // 初始化锁
    pthread_rwlock_init(&io_lock, NULL);
    pthread_rwlock_init(&tp_lock, NULL);
    pthread_rwlock_init(&cwl_lock, NULL);
    for (unsigned i = 0; i < THREAD_COUNT; i++) {
        pthread_mutex_init(&line_mutex[i], NULL);
        pthread_mutex_lock(&line_mutex[i]);
    }
    pthread_mutex_unlock(&line_mutex[0]);

    // 正则表达式匹配句首日期
    regex_t      reg;
    const char*  pattern_date = "[0-9]{8}-[0-9]{2}-[0-9]{3}-[0-9]{3}";
    const size_t nmatch       = 1;
    regmatch_t   pmatch[1];
    regcomp(&reg, pattern_date, REG_EXTENDED);

    unsigned lineno = 1;
    for (unsigned j = 0; j <= TOTAL_LINES / THREAD_COUNT; j++) {
        char raw_line[10240] = {0};

        // 在此处对 tp_lock 加了锁。由于在子线程一开始也尝试对 tp_lock 加锁，
        // 因此 tp_lock 真正所有者是主线程，而子线程一开始就被阻塞。
        //
        // 在变量 i 从 0 到 THREAD_COUNT 全部遍历完，生成了所有的子线程之后，
        // 主线程中的 tp_lock 才会被解开，此时所有的子线程同时工作。
        //
        // 这里生成子线程的方式类似于 GBN 协议。
        // 可以改成环形滑动窗口式的协议（例如 SR）以进一步加速进程
        pthread_rwlock_wrlock(&tp_lock);
        for (unsigned i = 0; i < THREAD_COUNT; i++) {
            if (fgets(raw_line, sizeof(raw_line), src_fp)) {
                char line[10240] = {0};

                // 跳过空行
                if (!strcmp(raw_line, "\n")) {
                    continue;
                }

                // 删除空白符、换行符
                strtrim(raw_line);

                // 匹配日期
                if (regexec(&reg, raw_line, nmatch, pmatch, 0) == 0) {
                    for (int i = pmatch[0].rm_so; i < pmatch[0].rm_eo; i++) {
                        // fprintf(fmm_fp, "%c", raw_line[i]);
                    }
                    // fprintf(fmm_fp, "/ ");
                    strmncpy(line, raw_line, pmatch[0].rm_eo, strlen(raw_line));
                }

                // 转 wchar_t
                wchar_t line_w[102400] = {0};
                cs2wcs(line_w, line);

                // 构造子线程参数
                struct thread_param* tp = malloc(sizeof(struct thread_param));
                tp->line_w              = malloc(sizeof(wchar_t) * 102400);
                tp->lineno              = malloc(sizeof(unsigned));
                tp->thread_index        = malloc(sizeof(unsigned));
                tp->max_word_len        = malloc(sizeof(unsigned long));
                wcscpy(tp->line_w, line_w);
                *(tp->lineno)       = lineno;
                *(tp->thread_index) = i;
                *(tp->max_word_len) = max_word_len;
                pthread_t tid;
                if (pthread_create(&tid, NULL, (void*)(&thread), (void*)(tp)) ==
                    -1) {
                    perror("[error] pthread");
                    return;
                }

                lineno++;
            }
        }
        pthread_rwlock_unlock(&tp_lock);
    }

    pthread_exit(NULL);
}

void max_match_threaded(const char* dic_file, const char* origin_data_file) {
    unsigned long max_word_len = 0;
    map                        = read_dic_threaded(dic_file, &max_word_len);

    // fmm 所用时间
    fmm_threaded(origin_data_file, max_word_len);
}
