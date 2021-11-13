#pragma once

#include <wchar.h>

/**
 * @brief 栈节点
 */
struct stack {
    wchar_t       value[1024];
    struct stack* next;
};

/**
 * @brief 初始化空栈
 *
 * @return struct node* 空栈
 */
struct stack* stack_init();

/**
 * @brief 将 \p value 压入栈 \p stack 中
 *
 * @param stack 栈
 * @param value 压栈的变量
 * @return int 若成功则返回 1；否则返回 0
 */
int push(struct stack* stack, wchar_t* value);

/**
 * @brief 从栈 \p stack 中弹出变量，存放于 \p top
 *
 * @param stack 栈
 * @param top 栈顶变量存放于此
 * @return int 若成功则返回 1；否则返回 0
 */
int pop(struct stack* stack, wchar_t* top);

/**
 * @brief 判断栈 \p stack 是否为空栈
 *
 * @param stack 栈
 * @return int 若是空栈则返回 1，否则返回 0
 */

int is_empty(struct stack* stack);
