#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_axs15231b.h"
#include "esp_lvgl_port.h"

#include "lvgl.h"

static const char *TAG = "LCD";

/* ==== PINOS ==== */
#define LCD_HOST   SPI2_HOST

#define LCD_PCLK   GPIO_NUM_47
#define LCD_D0     GPIO_NUM_21
#define LCD_D1     GPIO_NUM_48
#define LCD_D2     GPIO_NUM_40
#define LCD_D3     GPIO_NUM_39
#define LCD_CS     GPIO_NUM_45
#define LCD_RST    GPIO_NUM_NC
#define LCD_BL     GPIO_NUM_1

#define LCD_H_RES  320
#define LCD_V_RES  480

#define LCD_LEDC_CH 1

static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_display_t *lv_disp;

/* ==== Backlight ==== */
static esp_err_t bsp_display_brightness_init(void)
{
    const ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = 1,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    const ledc_channel_config_t channel = {
        .gpio_num = LCD_BL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LCD_LEDC_CH,
        .timer_sel = 1,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    ESP_ERROR_CHECK(ledc_channel_config(&channel));
    return ESP_OK;
}

static void bsp_display_backlight_on(void)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH, 1023);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH);
}

/* ==== LVGL tick ==== */
static uint32_t my_tick_cb(void)
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

/* ==== LVGL flush ==== */
static void my_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel = lv_display_get_user_data(disp);

    esp_lcd_panel_draw_bitmap(
        panel,
        area->x1,
        area->y1,
        area->x2 + 1,
        area->y2 + 1,
        px_map
    );

    lv_display_flush_ready(disp);
}

/* ================= LVGL TASK ================= */
static void lvgl_task(void *arg)
{
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/* ==== LCD init ==== */
static void lcd_init_qspi(void)
{
    const spi_bus_config_t buscfg = AXS15231B_PANEL_BUS_QSPI_CONFIG(
        LCD_PCLK, LCD_D0, LCD_D1, LCD_D2, LCD_D3,
        LCD_H_RES * 80 * sizeof(uint16_t)
    );

    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle;
    const esp_lcd_panel_io_spi_config_t io_cfg =
        AXS15231B_PANEL_IO_QSPI_CONFIG(LCD_CS, NULL, NULL);

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_cfg, &io_handle)
    );

    const axs15231b_vendor_config_t vendor_cfg = {
        .flags.use_qspi_interface = 1,
    };

    const esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = LCD_RST,
        .bits_per_pixel = 16,
        .vendor_config = &vendor_cfg,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_axs15231b(io_handle, &panel_cfg, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}



/* ================= app_main ================= */ 
void app_main(void) { 
    ESP_LOGI(TAG, "Init LCD + LVGL"); 
    /* 1. Init LVGL core */ 
    lv_init(); 
    
    /* 1. Tick */ 
    lv_tick_set_cb(my_tick_cb); 
    
    /* 2. Hardware */ 
    lcd_init_qspi(); 
    bsp_display_brightness_init(); 
    bsp_display_backlight_on(); 
    
    /* 3. Display LVGL */ 
    lv_disp = lv_display_create(LCD_H_RES, LCD_V_RES); 
    lv_display_set_user_data(lv_disp, panel_handle); 
    lv_display_set_flush_cb(lv_disp, my_flush_cb); 
    static uint8_t buf1[LCD_H_RES * 40 * 2]; 
    static uint8_t buf2[LCD_H_RES * 40 * 2]; 
    lv_display_set_buffers( 
        lv_disp, 
        buf1, 
        buf2, 
        sizeof(buf1), 
        LV_DISPLAY_RENDER_MODE_PARTIAL 
    ); 
    
    /* 4. Agora SIM: UI */ 
    lvgl_port_lock(0); 
    lv_obj_t * scr = lv_scr_act(); 
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x00FFBC), LV_PART_MAIN); 
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN); 
    lv_obj_t *label = lv_label_create(scr); 
    lv_label_set_text(label, "LVGL OK!"); 
    lv_obj_center(label); 
    lvgl_port_unlock(); 
    
    /* 5. Task LVGL */ 
    xTaskCreate(lvgl_task, "lvgl", 4096, NULL, 5, NULL); 
}
