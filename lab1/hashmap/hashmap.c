#include "hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

int hash_str(wchar_t* key) {
    // 模仿 JDK 1.7

    int      h = 0;
    wchar_t* p = key;

    while (*p != '\0') {
        h = 31 * h + *p;
        p++;
    }

    return h ^ (h >> 16);
}

struct hashmap* hashmap_init() {
    struct hashmap* map = (struct hashmap*)malloc(sizeof(struct hashmap));
    for (int i = 0; i < HASHMAP_ARRAY_SIZE; i++) {
        memset(map->hashmap_head->key, 0, HASHMAP_MAX_WCHAR);
        map->hashmap_head->next = NULL;
    }

    return map;
}

int put(struct hashmap* map, wchar_t* key, int value) {
    int                  h     = hash_str(key);
    int                  index = h % HASHMAP_ARRAY_SIZE;
    struct hashmap_node* p     = &map->hashmap_head[index];

    while (p->next) {
        if (!wcscmp(p->next->key, key)) {
            // 如果 key 已经存在，则修改
            p->next->value = value;
            return 2;
        }
        p = p->next;
    }

    // 如果 key 不存在，则添加
    p->next = (struct hashmap_node*)malloc(sizeof(struct hashmap_node));
    if (p->next == NULL) {
        return 0;
    }

    p = p->next;

    wcscpy(p->key, key);
    p->value = value;
    p->next  = NULL;

    return 1;
}

int get(struct hashmap* map, wchar_t* key) {
    int                  h     = hash_str(key);
    int                  index = h % HASHMAP_ARRAY_SIZE;
    struct hashmap_node* p     = &map->hashmap_head[index];

    while (p->next) {
        if (!wcscmp(p->next->key, key)) {
            return p->next->value;
        }
        p = p->next;
    }

    // 未找到 key
    return 0;
}