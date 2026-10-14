#pragma once
extern unsigned fake_ms;
static inline void vTaskDelay(unsigned t){fake_ms+=t;}
static inline int xTaskCreate(void(*f)(void*),const char*n,unsigned s,void*v,unsigned p,void*h){(void)f;(void)n;(void)s;(void)v;(void)p;(void)h;return 1;}
