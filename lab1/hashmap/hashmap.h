#pragma once

#include <wchar.h>

/**
 * @brief hashmap 数组大小
 */
#define HASHMAP_ARRAY_SIZE 102400

/**
 * @brief hashmap 最大字符串大小
 *
 */
#define HASHMAP_MAX_WCHAR 1024

/**
 * @brief 哈希表链表的结点
 */
struct hashmap_node {
    wchar_t              key[HASHMAP_MAX_WCHAR];
    int                  value;
    struct hashmap_node* next;
};

/**
 * @brief hashmap
 */
struct hashmap {
    struct hashmap_node hashmap_head[HASHMAP_ARRAY_SIZE];
};

/**
 * @brief 字符串的哈希函数
 *
 * @param key 需要计算哈希值的字符串
 * @return int 返回哈希值
 */
int hash_str(wchar_t* key);

/**
 * @brief 初始化空 hashmap
 *
 * @return struct hashmap* 返回空的 hashmap
 */
struct hashmap* hashmap_init();

/**
 * @brief 向 hashmap \p map 中添加一个 \p key - \p value 键值对；
 *        若 \p key 已经存在，则修改原来的 \p value 值；
 *        若 \p key 不存在，则添加新的的 \p value 值
 *
 * @param map hashmap
 * @param key 键
 * @param value 值
 * @return int 添加成功则返回 1；
 *             修改成功则返回 2；
 *             失败则返回 0
 */
int put(struct hashmap* map, wchar_t* key, int value);

/**
 * @brief 在 hashmap \p map 中查找 \p key 对应的 value
 *
 * @param map hashmap
 * @param key 键
 * @return int 若找到 \p key 则返回对应的 value；
 *         若找不到则返回 0
 */
int get(struct hashmap* map, wchar_t* key);
