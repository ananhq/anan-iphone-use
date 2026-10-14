#pragma once
#include <stddef.h>
typedef void *QueueHandle_t;
static inline void *xQueueCreate(unsigned n,size_t s){(void)n;(void)s;return (void*)1;}
static inline int xQueueReceive(void*q,void*v,unsigned t){(void)q;(void)v;(void)t;return 0;}
static inline int xQueueSend(void*q,const void*v,unsigned t){(void)q;(void)v;(void)t;return 1;}
