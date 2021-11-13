#include "stack.h"
#include <stdlib.h>
#include <string.h>

struct stack* stack_init() {
    struct stack* stack =
        (struct stack*)malloc(sizeof(struct stack));
    stack->next = NULL;

    return stack;
}

int push(struct stack* stack, wchar_t* value) {
    struct stack* p = stack;

    struct stack* node =
        (struct stack*)malloc(sizeof(struct stack));
    if (!node) {
        return 0;
    }

    wcscpy(node->value, value);
    node->next = p->next;
    p->next    = node;

    return 1;
}

int pop(struct stack* stack, wchar_t* top) {
    struct stack* p = stack;

    if (!p->next) {
        // 空栈
        return 0;
    }

    wcscpy(top, p->next->value);
    p->next = p->next->next;

    return 1;
}

int is_empty(struct stack* stack) { return stack->next ? 0 : 1; }
