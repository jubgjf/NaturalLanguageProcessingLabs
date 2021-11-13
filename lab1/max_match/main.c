#include "max_match.h"
#include "max_match_fast.h"
#include "timer.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    const char dic_file[]          = "lab1/dic/dic.txt";
    const char origin_data_file[]  = "lab1/dataset/199801_sent.txt";
    const char seg_fmm_file[]      = "lab1/seg_result/seg_FMM.txt";
    const char seg_bmm_file[]      = "lab1/seg_result/seg_BMM.txt";
    const char seg_fmm_fast_file[] = "lab1/seg_result/seg_FMM_fast.txt";
    const char seg_bmm_fast_file[] = "lab1/seg_result/seg_BMM_fast.txt";

    // 基于机械匹配的分词
    struct max_match_time* mm_time =
        max_match(dic_file, origin_data_file, seg_fmm_file, seg_bmm_file);

    // 优化基于机械匹配的分词
    struct max_match_time* mm_time_fast = max_match_fast(
        dic_file, origin_data_file, seg_fmm_fast_file, seg_bmm_fast_file);

    // 分词计时
    char time_cost_buf[1024] = {0};
    sprintf(time_cost_buf,
            "plain | fmm: %fs | bmm: %fs\n"
            "fast  | fmm: %fs | bmm: %fs\n",
            mm_time->fmm_time, mm_time->bmm_time, mm_time_fast->fmm_time,
            mm_time_fast->bmm_time);

    // 将时间写入文件
    const char time_cost_file[] = "lab1/evaluate/TimeCost.txt";
    int        fd               = open(time_cost_file, O_RDWR | O_CREAT, 0664);
    if (fd == -1) {
        perror("[error] open");
        return -1;
    }
    write(fd, time_cost_buf, strlen(time_cost_buf));
    close(fd);
}
