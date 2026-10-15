// SPDX-License-Identifier: Apache-2.0
// M5IOE1 wake/timing/register facts: M5Stack/M5IOE1
// 846eec7d05e25c09013be2acdb8804487f48a62e; see THIRD_PARTY.md and
// licenses/M5IOE1-MIT.txt. This file uses native IDF APIs, not the vendor library.
#include "display.h"
#include "panel_init.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdatomic.h>
#include <string.h>
#define WIDTH 466
#define HEIGHT 466
#define STRIP 16
static esp_lcd_panel_handle_t panel;
static esp_lcd_panel_io_handle_t io;
static SemaphoreHandle_t transferred;
static uint16_t *pixels;
static bool available;
typedef enum {
    SCREEN_STARTING,
    SCREEN_IOE_DISCOVERY,
    SCREEN_IOE_CONFIG,
    SCREEN_POWER,
    SCREEN_MEMORY,
    SCREEN_SPI,
    SCREEN_PANEL_IO,
    SCREEN_PANEL_INIT,
    SCREEN_DRAWING,
    SCREEN_READY
} screen_stage;
static atomic_int stage;
static atomic_int last_error;
static bool failed(esp_err_t error) {
    atomic_store(&last_error, error);
    ESP_LOGE("display", "%s: %s", display_status(), esp_err_to_name(error));
    return false;
}
const char *display_status(void) {
    static const char *const names[] = {
        "starting", "ioe_discovery", "ioe_config", "screen_power",   "dma_memory",
        "spi_bus",  "panel_io",      "panel_init", "pixel_transfer", "ready"};
    return names[atomic_load(&stage)];
}
int display_error(void) { return atomic_load(&last_error); }

extern i2c_master_bus_handle_t board_i2c_bus(void);
// First-party, hand-authored 5x7 glyph rows. No external font or graphics stack.
static const uint8_t letters[][7] = {
    {14, 17, 17, 31, 17, 17, 17}, {30, 17, 17, 30, 17, 17, 30}, {14, 17, 16, 16, 16, 17, 14},
    {30, 17, 17, 17, 17, 17, 30}, {31, 16, 16, 30, 16, 16, 31}, {31, 16, 16, 30, 16, 16, 16},
    {14, 17, 16, 23, 17, 17, 15}, {17, 17, 17, 31, 17, 17, 17}, {31, 4, 4, 4, 4, 4, 31},
    {7, 2, 2, 2, 18, 18, 12},     {17, 18, 20, 24, 20, 18, 17}, {16, 16, 16, 16, 16, 16, 31},
    {17, 27, 21, 21, 17, 17, 17}, {17, 25, 25, 21, 19, 19, 17}, {14, 17, 17, 17, 17, 17, 14},
    {30, 17, 17, 30, 16, 16, 16}, {14, 17, 17, 17, 21, 18, 13}, {30, 17, 17, 30, 20, 18, 17},
    {15, 16, 16, 14, 1, 1, 30},   {31, 4, 4, 4, 4, 4, 4},       {17, 17, 17, 17, 17, 17, 14},
    {17, 17, 17, 17, 17, 10, 4},  {17, 17, 17, 21, 21, 21, 10}, {17, 17, 10, 4, 10, 17, 17},
    {17, 17, 10, 4, 4, 4, 4},     {31, 1, 2, 4, 8, 16, 31}};
static const uint8_t digits[][7] = {{14, 17, 19, 21, 25, 17, 14}, {4, 12, 4, 4, 4, 4, 14},
                                    {14, 17, 1, 2, 4, 8, 31},     {30, 1, 1, 14, 1, 1, 30},
                                    {2, 6, 10, 18, 31, 2, 2},     {31, 16, 16, 30, 1, 1, 30},
                                    {14, 16, 16, 30, 17, 17, 14}, {31, 1, 2, 4, 8, 8, 8},
                                    {14, 17, 17, 14, 17, 17, 14}, {14, 17, 17, 15, 1, 1, 14}};
static uint8_t glyph(char c, unsigned row) {
    if (c >= 'a' && c <= 'z')
        c -= 32;
    if (c >= 'A' && c <= 'Z')
        return letters[c - 'A'][row];
    if (c >= '0' && c <= '9')
        return digits[c - '0'][row];
    if (c == ':')
        return row == 2 || row == 5 ? 4 : 0;
    if (c == '.')
        return row == 6 ? 4 : 0;
    if (c == '/')
        return (row == 0 || row == 1) ? 1 : row == 2 ? 2 : row == 3 ? 4 : row == 4 ? 8 : 16;
    if (c == '-')
        return row == 3 ? 14 : 0;
    if (c == '+')
        return row == 3 ? 14 : (row >= 1 && row <= 5 ? 4 : 0);
    if (c == '?') {
        const uint8_t q[] = {14, 17, 1, 2, 4, 0, 4};
        return q[row];
    }
    if (c == '%') {
        const uint8_t percent[] = {25, 26, 2, 4, 8, 11, 19};
        return percent[row];
    }
    return 0;
}
static bool IRAM_ATTR color_done(esp_lcd_panel_io_handle_t handle,
                                 esp_lcd_panel_io_event_data_t *event, void *ctx) {
    (void)handle;
    (void)event;
    (void)ctx;
    BaseType_t wake = pdFALSE;
    xSemaphoreGiveFromISR(transferred, &wake);
    return wake == pdTRUE;
}
// M5IOE1 is an MCU-backed slave. Its documented compatibility layer waits
// 500us after each transaction; use a yielding 1ms delay instead of busy-waiting.
static esp_err_t read_register(i2c_master_dev_handle_t chip, uint8_t reg, uint8_t *value) {
    esp_err_t result = ESP_FAIL;
    for (unsigned attempt = 0; attempt < 3; ++attempt) {
        result = i2c_master_transmit_receive(chip, &reg, 1, value, 1, 100);
        vTaskDelay(pdMS_TO_TICKS(result == ESP_OK ? 1 : 5));
        if (result == ESP_OK)
            return result;
    }
    return result;
}
static bool update_bits(i2c_master_dev_handle_t chip, uint8_t reg, uint8_t set, uint8_t clear) {
    uint8_t value;
    for (unsigned attempt = 0; attempt < 3; ++attempt) {
        esp_err_t result = read_register(chip, reg, &value);
        if (result != ESP_OK)
            continue;
        uint8_t write[] = {reg, (value | set) & (uint8_t)~clear};
        result = i2c_master_transmit(chip, write, 2, 100);
        vTaskDelay(pdMS_TO_TICKS(1));
        if (result != ESP_OK)
            continue;
        result = read_register(chip, reg, &value);
        if (result == ESP_OK && (value & (set | clear)) == set)
            return true;
    }
    ESP_LOGE("display", "IOE register 0x%02x failed verification", reg);
    return false;
}
static i2c_master_dev_handle_t find_expander(void) {
    i2c_master_bus_handle_t bus = board_i2c_bus();
    const uint8_t addresses[] = {0x4f, 0x6f};
    const unsigned rates[] = {100000, 100000, 400000};
    for (unsigned address = 0; address < 2; ++address) {
        for (unsigned attempt = 0; attempt < 3; ++attempt) {
            if (attempt == 1)
                vTaskDelay(pdMS_TO_TICKS(800));
            // START can wake a sleeping expander even when that first probe NACKs.
            (void)i2c_master_probe(bus, addresses[address], 10);
            vTaskDelay(pdMS_TO_TICKS(10));
            i2c_device_config_t config = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                          .device_address = addresses[address],
                                          .scl_speed_hz = rates[attempt]};
            i2c_master_dev_handle_t chip;
            if (i2c_master_bus_add_device(bus, &config, &chip) != ESP_OK)
                continue;
            uint8_t revision;
            if (read_register(chip, 0x02, &revision) == ESP_OK)
                return chip;
            i2c_master_bus_rm_device(chip);
        }
    }
    return NULL;
}
static bool power_screen(void) {
    atomic_store(&stage, SCREEN_IOE_DISCOVERY);
    i2c_master_dev_handle_t chip = find_expander();
    if (!chip)
        return failed(ESP_ERR_NOT_FOUND);
    atomic_store(&stage, SCREEN_IOE_CONFIG);
    // Only clear SLEEP bits in I2C_CFG; retain speed/wake/interrupt settings.
    // This follows the factory's explicit setI2cSleepTime(0) before screen power.
    const uint8_t mask = (1u << 7) | (1u << 4);
    bool ok = update_bits(chip, 0x23, 0, 0x0f) && update_bits(chip, 0x09, 0, mask) &&
              update_bits(chip, 0x0b, 0, mask) && update_bits(chip, 0x0d, 0, mask) &&
              update_bits(chip, 0x13, 0, mask) && update_bits(chip, 0x03, mask, 0);
    if (!ok) {
        i2c_master_bus_rm_device(chip);
        return failed(ESP_FAIL);
    }
    atomic_store(&stage, SCREEN_POWER);
    // PYG8/PYG5 are low-register bits 7/4: enable L3B and pulse OLED reset.
    ok = update_bits(chip, 0x05, 1u << 7, 1u << 4);
    vTaskDelay(pdMS_TO_TICKS(10));
    if (ok)
        ok = update_bits(chip, 0x05, mask, 0);
    vTaskDelay(pdMS_TO_TICKS(150));
    // The factory also reads the driven L3B enable pin and retries if still low.
    uint8_t pins = 0;
    if (ok)
        ok = read_register(chip, 0x07, &pins) == ESP_OK && (pins & mask) == mask;
    i2c_master_bus_rm_device(chip);
    return ok ? true : failed(ESP_FAIL);
}
bool display_init(void) {
    if (!power_screen())
        return false;
    atomic_store(&stage, SCREEN_MEMORY);
    transferred = xSemaphoreCreateBinary();
    pixels = heap_caps_malloc(WIDTH * STRIP * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!transferred || !pixels)
        return failed(ESP_ERR_NO_MEM);
    atomic_store(&stage, SCREEN_SPI);
    spi_bus_config_t bus = CO5300_PANEL_BUS_QSPI_CONFIG(40, 41, 42, 46, 45, WIDTH * STRIP * 2);
    esp_err_t result = spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
    if (result != ESP_OK)
        return failed(result);
    atomic_store(&stage, SCREEN_PANEL_IO);
    esp_lcd_panel_io_spi_config_t io_config = CO5300_PANEL_IO_QSPI_CONFIG(39, color_done, NULL);
    result = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io);
    if (result != ESP_OK)
        return failed(result);
    atomic_store(&stage, SCREEN_PANEL_INIT);
    co5300_vendor_config_t vendor = {.init_cmds = stopwatch_panel_commands,
                                     .init_cmds_size = sizeof(stopwatch_panel_commands) /
                                                       sizeof(stopwatch_panel_commands[0]),
                                     .flags.use_qspi_interface = 1};
    esp_lcd_panel_dev_config_t config = {.reset_gpio_num = -1,
                                         .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
                                         .bits_per_pixel = 16,
                                         .vendor_config = &vendor};
    result = esp_lcd_new_panel_co5300(io, &config, &panel);
    if (result == ESP_OK)
        result = esp_lcd_panel_init(panel);
    if (result == ESP_OK)
        result = esp_lcd_panel_set_gap(panel, 6, 0);
    if (result == ESP_OK)
        result = esp_lcd_panel_co5300_set_brightness(panel, 60);
    if (result != ESP_OK)
        return failed(result);
    available = true;
    ESP_LOGI("display", "panel initialized; first frame pending");
    return true;
}
static void draw_text(const char *text, int x, int y, int scale, uint16_t color, int strip_y) {
    int cursor = x;
    for (const char *p = text; *p; ++p) {
        if (*p == '\n') {
            y += scale * 10;
            cursor = x;
            continue;
        }
        for (unsigned row = 0; row < 7; ++row)
            for (unsigned col = 0; col < 5; ++col)
                if (glyph(*p, row) & (1u << (4 - col)))
                    for (int dy = 0; dy < scale; ++dy)
                        for (int dx = 0; dx < scale; ++dx) {
                            int px = cursor + col * scale + dx, py = y + row * scale + dy;
                            if (px >= 0 && px < WIDTH && py >= strip_y && py < strip_y + STRIP &&
                                py < HEIGHT)
                                pixels[(py - strip_y) * WIDTH + px] = color;
                        }
        cursor += scale * 6;
    }
}
void display_text(const char *title, const char *body, const char *footer) {
    if (!available)
        return;
    atomic_store(&stage, SCREEN_DRAWING);
    for (int y = 0; y < HEIGHT; y += STRIP) {
        memset(pixels, 0, WIDTH * STRIP * 2);
        // RGB565 words are byte-swapped for the panel's big-endian pixel stream.
        draw_text(title, (WIDTH - (int)strlen(title) * 18) / 2, 76, 3, 0x89fe, y);
        draw_text(body, 50, 138, 2, 0xffff, y);
        draw_text(footer, 50, 352, 2, 0xe0c7, y);
        int end = y + STRIP < HEIGHT ? y + STRIP : HEIGHT;
        esp_err_t result = esp_lcd_panel_draw_bitmap(panel, 0, y, WIDTH, end, pixels);
        if (result == ESP_OK && xSemaphoreTake(transferred, pdMS_TO_TICKS(500)) != pdTRUE)
            result = ESP_ERR_TIMEOUT;
        if (result != ESP_OK) {
            // Never reuse a DMA buffer after an uncertain completion.
            available = false;
            failed(result);
            return;
        }
    }
    atomic_store(&stage, SCREEN_READY);
}

bool display_ready(void) { return available; }

uint8_t display_ascii_row(char c, unsigned row) { return row < 7 ? glyph(c, row) : 0; }
bool display_reset_touch(void) {
    // Called after display initialization; no concurrent expander RMW at boot.
    i2c_master_dev_handle_t chip = find_expander();
    if (!chip)
        return false;
    const uint8_t mask = 1u << 3; // PYG4
    bool ok = update_bits(chip, 0x09, 0, mask) && update_bits(chip, 0x0b, 0, mask) &&
              update_bits(chip, 0x0d, 0, mask) && update_bits(chip, 0x13, 0, mask) &&
              update_bits(chip, 0x03, mask, 0) && update_bits(chip, 0x05, 0, mask);
    vTaskDelay(pdMS_TO_TICKS(10));
    if (ok)
        ok = update_bits(chip, 0x05, mask, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    i2c_master_bus_rm_device(chip);
    return ok;
}
bool display_paint(display_strip_painter painter, void *context) {
    if (!available)
        return false;
    atomic_store(&stage, SCREEN_DRAWING);
    for (int y = 0; y < HEIGHT; y += STRIP) {
        painter(pixels, y, context);
        int end = y + STRIP < HEIGHT ? y + STRIP : HEIGHT;
        esp_err_t result = esp_lcd_panel_draw_bitmap(panel, 0, y, WIDTH, end, pixels);
        if (result == ESP_OK && xSemaphoreTake(transferred, pdMS_TO_TICKS(500)) != pdTRUE)
            result = ESP_ERR_TIMEOUT;
        if (result != ESP_OK) {
            available = false;
            return failed(result);
        }
    }
    atomic_store(&stage, SCREEN_READY);
    return true;
}
bool display_paint_region(int x,int y,int end,int bottom,display_strip_painter painter,void *context) {
    if(!available||x<0||y<0||end>WIDTH||bottom>HEIGHT||x>=end||y>=bottom)
        return false;
    atomic_store(&stage,SCREEN_DRAWING);
    int width=end-x;
    for(int top=y;top<bottom;top+=STRIP){
        painter(pixels,top,context);
        int last=top+STRIP<bottom?top+STRIP:bottom;
        // Painter uses the unchanged full-width scratch stride. Compact forward
        // in-place only after the previous DMA completed, before queuing this one.
        for(int row=0;row<last-top;row++)
            memmove(pixels+row*width,pixels+row*WIDTH+x,width*sizeof(*pixels));
        esp_err_t result=esp_lcd_panel_draw_bitmap(panel,x,top,end,last,pixels);
        if(result==ESP_OK&&xSemaphoreTake(transferred,pdMS_TO_TICKS(500))!=pdTRUE)
            result=ESP_ERR_TIMEOUT;
        if(result!=ESP_OK){available=false;return failed(result);}
    }
    atomic_store(&stage,SCREEN_READY);
    return true;
}

// Compact avatar regions: same DMA buffer/completion contract; no row compaction.
static display_paint_metrics paint_metrics;
display_paint_metrics display_last_paint_metrics(void){return paint_metrics;}
bool display_paint_regions(const display_region *regions,unsigned count,display_region_painter painter,void *context){
    paint_metrics=(display_paint_metrics){0};
    if(!available||count>3||(!regions&&count)||!painter)return false;
    display_region merged[3];unsigned used=0;
    for(unsigned i=0;i<count;i++){
        display_region r=regions[i];
        if(r.x<0||r.y<0||r.w<=0||r.h<=0||r.x>WIDTH-r.w||r.y>HEIGHT-r.h)return false;
        // CO5300 windows need even starts and sizes on both axes. Expand before
        // merging, then repaint the added pixels from the same current scene.
        int end=(r.x+r.w+1)&~1,bottom=(r.y+r.h+1)&~1;
        r.x&=~1;r.y&=~1;r.w=end-r.x;r.h=bottom-r.y;
        merged[used++]=r;
    }
    // Three regions at most: repeated merging handles transitive overlap.
    bool again=true;
    while(again){
        again=false;
        for(unsigned i=0;i<used&&!again;i++)for(unsigned j=i+1;j<used;j++){
            display_region a=merged[i],b=merged[j];
            if(a.x>=b.x+b.w||b.x>=a.x+a.w||a.y>=b.y+b.h||b.y>=a.y+a.h)continue;
            int x=a.x<b.x?a.x:b.x,y=a.y<b.y?a.y:b.y;
            int end=a.x+a.w>b.x+b.w?a.x+a.w:b.x+b.w;
            int bottom=a.y+a.h>b.y+b.h?a.y+a.h:b.y+b.h;
            merged[i]=(display_region){x,y,end-x,bottom-y};
            merged[j]=merged[--used];again=true;break;
        }
    }
    if(!used)return true;
    atomic_store(&stage,SCREEN_DRAWING);
    for(unsigned i=0;i<used;i++){
        // Keep every chunk aligned too: e.g. 80px width fits 93 rows, but sending
        // 93 would put the next window on an odd row. Capacity is at least 16.
        display_region r=merged[i];int batch=(WIDTH*STRIP/r.w)&~1;
        for(int top=r.y;top<r.y+r.h;){
            int rows=r.y+r.h-top;if(rows>batch)rows=batch;
            painter(pixels,r.x,top,r.w,rows,context);
            esp_err_t result=esp_lcd_panel_draw_bitmap(panel,r.x,top,r.x+r.w,top+rows,pixels);
            paint_metrics.transfers++;paint_metrics.pixels+=(uint32_t)(r.w*rows);
            if(result==ESP_OK&&xSemaphoreTake(transferred,pdMS_TO_TICKS(500))!=pdTRUE)result=ESP_ERR_TIMEOUT;
            if(result!=ESP_OK){available=false;return failed(result);}
            top+=rows;
        }
    }
    atomic_store(&stage,SCREEN_READY);return true;
}
