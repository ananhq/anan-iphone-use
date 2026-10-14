// SPDX-License-Identifier: Apache-2.0
#include "ui.h"
#include "ui_font.h"
#include "display.h"
#include "wifi_qr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>
static SemaphoreHandle_t front_lock;
static ui_model front;
static bool front_valid;
typedef struct {
    ui_widget widgets[UI_WIDGET_LIMIT];
    size_t count;
    ui_action focus;
    wifi_qr qr;
} scene;
void ui_init(void) {
    front_lock = xSemaphoreCreateMutex();
    configASSERT(front_lock);
}
bool ui_displayed(ui_model *m) {
    xSemaphoreTake(front_lock, portMAX_DELAY);
    bool valid = front_valid;
    if (valid)
        *m = front;
    xSemaphoreGive(front_lock);
    return valid;
}
static unsigned utf8(const char **s) {
    const unsigned char *p = (const unsigned char *)*s;
    unsigned code;
    if (*p < 128) {
        code = *p++;
    } else if ((*p & 0xe0) == 0xc0) {
        code = (*p & 31) << 6;
        code |= p[1] & 63;
        p += 2;
    } else {
        code = (*p & 15) << 12;
        code |= (p[1] & 63) << 6;
        code |= p[2] & 63;
        p += 3;
    }
    *s = (const char *)p;
    return code;
}
static const ui_font_glyph *find_glyph(unsigned code) {
    size_t low = 0, high = UI_FONT_COUNT;
    while (low < high) {
        size_t mid = (low + high) / 2;
        if (ui_font[mid].code < code)
            low = mid + 1;
        else
            high = mid;
    }
    return low < UI_FONT_COUNT && ui_font[low].code == code ? &ui_font[low] : NULL;
}
static int cjk_size(unsigned scale) { return scale == 1 ? 18 : scale >= 3 ? 28 : 24; }
static unsigned text_width(const char *text, unsigned scale) {
    unsigned width = 0;
    while (*text) {
        unsigned code = utf8(&text);
        width += code < 128 ? 6 * scale : cjk_size(scale);
    }
    return width;
}
static void pixel(uint16_t *pixels, int strip_y, int x, int y, uint16_t color) {
    if (x >= 0 && x < 466 && y >= strip_y && y < strip_y + 16 && y < 466)
        pixels[(y - strip_y) * 466 + x] = (color >> 8) | (color << 8);
}
static bool inside(ui_rect r, int x, int y, int radius) {
    if (x < r.x || x >= r.x + r.w || y < r.y || y >= r.y + r.h)
        return false;
    int cx = x < r.x + radius ? r.x + radius : x >= r.x + r.w - radius ? r.x + r.w - radius - 1 : x;
    int cy = y < r.y + radius ? r.y + radius : y >= r.y + r.h - radius ? r.y + r.h - radius - 1 : y;
    int dx = x - cx, dy = y - cy;
    return dx * dx + dy * dy <= radius * radius;
}
static void draw_label(uint16_t *pixels, int strip_y, const ui_widget *w) {
    unsigned width = text_width(w->text, w->scale);
    int cursor = w->rect.x + (w->rect.w - (int)width) / 2;
    const char *p = w->text;
    while (*p) {
        unsigned code = utf8(&p);
        if (code < 128) {
            int top = w->rect.y + (w->rect.h - (int)w->scale * 7) / 2;
            for (unsigned row = 0; row < 7; ++row) {
                uint8_t bits = display_ascii_row((char)code, row);
                for (unsigned col = 0; col < 5; ++col)
                    if (bits & (1u << (4 - col)))
                        for (unsigned dy = 0; dy < w->scale; ++dy)
                            for (unsigned dx = 0; dx < w->scale; ++dx)
                                pixel(pixels, strip_y, cursor + col * w->scale + dx,
                                      top + row * w->scale + dy, w->foreground);
            }
            cursor += 6 * w->scale;
        } else {
            const ui_font_glyph *glyph = find_glyph(code);
            int size = cjk_size(w->scale), top = w->rect.y + (w->rect.h - size) / 2;
            if (glyph)
                for (int dy = 0; dy < size; ++dy)
                    if (top + dy >= strip_y && top + dy < strip_y + 16) {
                        unsigned row = dy * 24 / size;
                        for (int dx = 0; dx < size; ++dx) {
                            unsigned col = dx * 24 / size;
                            if (glyph->rows[row * 3 + col / 8] & (1u << (7 - col % 8)))
                                pixel(pixels, strip_y, cursor + dx, top + dy, w->foreground);
                        }
                    }
            cursor += size;
        }
    }
}
static void paint(uint16_t *pixels, int strip_y, void *context) {
    scene *s = context;
    for (int i = 0; i < 466 * 16; ++i)
        pixels[i] = 0x8208; // swapped RGB565 background 0x0882
    for (size_t i = 0; i < s->count; ++i) {
        const ui_widget *w = &s->widgets[i];
        if(w->qr) {
            unsigned side=wifi_qr_size(&s->qr);
            if(side) {
                unsigned size=(side+8)*5;
                int left=(466-(int)size)/2,top=w->rect.y;
                for(int y=strip_y;y<strip_y+16&&y<top+(int)size;y++)
                    if(y>=top)for(int x=left;x<left+(int)size;x++)
                        pixel(pixels,strip_y,x,y,wifi_qr_module(&s->qr,(x-left)/5-4,(y-top)/5-4)?0x0000:0xffff);
            }
            continue;
        }
        if (w->button) {
            ui_rect inner = {w->rect.x + 2, w->rect.y + 2, w->rect.w - 4, w->rect.h - 4};
            for (int y = strip_y; y < strip_y + 16 && y < 466; ++y)
                for (int x = w->rect.x; x < w->rect.x + w->rect.w; ++x)
                    if (inside(w->rect, x, y, 14))
                        pixel(pixels, strip_y, x, y,
                              ((w->selected || w->action == s->focus) && !inside(inner, x, y, 12))
                                  ? 0xfe89
                                  : w->background);
        }
        draw_label(pixels, strip_y, w);
    }
}
bool ui_present(const ui_model *m) {
    scene s = {.focus = m->focus};
    s.count = ui_layout(m, s.widgets);
    if(m->setup_ap)wifi_qr_make(&s.qr,m->ap_name,m->ap_password);
    bool ok = display_paint(paint, &s);
    xSemaphoreTake(front_lock, portMAX_DELAY);
    front_valid = ok;
    if (ok)
        front = *m;
    xSemaphoreGive(front_lock);
    return ok;
}
