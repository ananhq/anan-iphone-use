// SPDX-License-Identifier: Apache-2.0
// First-party IDF transport/filter adapter; unmodified Bosch driver in vendor/.
#include "imu.h"
#include "vendor/bmi270/bmi270.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <stdatomic.h>
#include <string.h>
extern i2c_master_bus_handle_t board_i2c_bus(void);
static i2c_master_dev_handle_t chip;
static struct bmi2_dev sensor;
static tilt_state filter;
static atomic_uint position,updated;
static atomic_bool valid,recenter;
static int8_t read_imu(uint8_t reg,uint8_t *data,uint32_t n,void *ctx){
    if(!n||n>32)return BMI2_E_COM_FAIL;
    return i2c_master_transmit_receive(ctx,&reg,1,data,n,20)==ESP_OK?BMI2_OK:BMI2_E_COM_FAIL;
}
static int8_t write_imu(uint8_t reg,const uint8_t *data,uint32_t n,void *ctx){
    if(!n||n>32)return BMI2_E_COM_FAIL;
    uint8_t bytes[33];bytes[0]=reg;memcpy(bytes+1,data,n);
    return i2c_master_transmit(ctx,bytes,n+1,20)==ESP_OK?BMI2_OK:BMI2_E_COM_FAIL;
}
static void wait_imu(uint32_t us,void *ctx){
    (void)ctx;
    if(us>=1000)vTaskDelay(pdMS_TO_TICKS((us+999)/1000));else esp_rom_delay_us(us);
}
static bool initialize(void){
    i2c_device_config_t cfg={.dev_addr_length=I2C_ADDR_BIT_LEN_7,.device_address=0x68,.scl_speed_hz=100000};
    if(i2c_master_bus_add_device(board_i2c_bus(),&cfg,&chip)!=ESP_OK)return false;
    sensor=(struct bmi2_dev){.intf=BMI2_I2C_INTF,.intf_ptr=chip,.read=read_imu,.write=write_imu,
        .delay_us=wait_imu,.read_write_len=32};
    bool ok=bmi270_init(&sensor)==BMI2_OK;
    struct bmi2_sens_config config[2]={{.type=BMI2_ACCEL},{.type=BMI2_GYRO}};
    if(ok)ok=bmi2_get_sensor_config(config,2,&sensor)==BMI2_OK;
    if(ok){
        config[0].cfg.acc.odr=BMI2_ACC_ODR_50HZ;config[0].cfg.acc.range=BMI2_ACC_RANGE_4G;
        config[0].cfg.acc.bwp=BMI2_ACC_NORMAL_AVG4;config[0].cfg.acc.filter_perf=BMI2_PERF_OPT_MODE;
        config[1].cfg.gyr.odr=BMI2_GYR_ODR_50HZ;config[1].cfg.gyr.range=BMI2_GYR_RANGE_500;
        config[1].cfg.gyr.bwp=BMI2_GYR_NORMAL_MODE;config[1].cfg.gyr.filter_perf=BMI2_PERF_OPT_MODE;
        ok=bmi2_set_sensor_config(config,2,&sensor)==BMI2_OK;
    }
    uint8_t enabled[]={BMI2_ACCEL,BMI2_GYRO};
    if(ok)ok=bmi2_sensor_enable(enabled,2,&sensor)==BMI2_OK;
    if(!ok){i2c_master_bus_rm_device(chip);chip=NULL;}
    return ok;
}
void imu_recenter(void){atomic_store(&recenter,true);}
static void poll_imu(uint32_t now){
    if(atomic_exchange(&recenter,false))tilt_reset(&filter);
    struct bmi2_sens_data data={0};
    if(bmi2_get_sensor_data(&data,&sensor)!=BMI2_OK){atomic_store(&valid,false);return;}
    tilt_sample sample={data.acc.x/8192.0f,data.acc.y/8192.0f,data.acc.z/8192.0f,
        data.gyr.x/65.536f,data.gyr.y/65.536f,data.gyr.z/65.536f};
    tilt_gaze gaze=tilt_update(&filter,&sample,now);
    int16_t x=(int16_t)(gaze.x*1000),y=(int16_t)(gaze.y*1000);
    atomic_store(&position,((uint32_t)(uint16_t)y<<16)|(uint16_t)x);
    atomic_store(&updated,now);atomic_store(&valid,gaze.ready);
}
tilt_gaze imu_snapshot(uint32_t now){
    if(!atomic_load(&valid)||atomic_load(&recenter)||(uint32_t)(now-atomic_load(&updated))>250)
        return (tilt_gaze){0};
    uint32_t xy=atomic_load(&position);
    return (tilt_gaze){(int16_t)(xy&65535)/1000.0f,(int16_t)(xy>>16)/1000.0f,true};
}
static void imu_worker(void *arg){
    (void)arg;
    if(!initialize()){ESP_LOGW("imu","BMI270 unavailable; avatar uses centered ambient gaze");vTaskDelete(NULL);return;}
    ESP_LOGI("imu","BMI270 ready");
    for(;;){poll_imu((uint32_t)(esp_timer_get_time()/1000));vTaskDelay(pdMS_TO_TICKS(25));}
}
void imu_start(void){
    if(xTaskCreate(imu_worker,"avatar-imu",8192,NULL,1,NULL)!=pdPASS)
        ESP_LOGW("imu","IMU task unavailable; control remains usable without tilt");
}
