#!/usr/bin/env python3
"""Predefined actual BMI270 adapter fixture; fake I2C/SDK/sensor data, no board."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from firmware_paths import include_flags
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
fixture=r'''
#include "vendor/bmi270/bmi270.h"
#include "imu.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "driver/i2c_master.h"
unsigned fake_ms;static bool fail_init,fail_data,missing;
static unsigned removes,configured,reads,writes;
i2c_master_bus_handle_t board_i2c_bus(void){return (void*)1;}
int i2c_master_bus_add_device(void*b,const i2c_device_config_t*c,void**out){assert(b==(void*)1&&c->device_address==0x68);if(missing)return -1;*out=(void*)2;return 0;}
int i2c_master_bus_rm_device(void*d){assert(d==(void*)2);removes++;return 0;}
int i2c_master_transmit_receive(void*d,const uint8_t*r,size_t n,uint8_t*out,size_t k,int timeout){assert(d==(void*)2&&r&&n==1&&k<=32&&timeout<=20);memset(out,0,k);reads++;return 0;}
int i2c_master_transmit(void*d,const uint8_t*in,size_t n,int timeout){assert(d==(void*)2&&in&&n<=33&&timeout<=20);writes++;return 0;}
int8_t bmi270_init(struct bmi2_dev*d){uint8_t v[32]={0};assert(d->intf==BMI2_I2C_INTF&&d->read_write_len<=32);assert(d->write(0x5e,v,32,d->intf_ptr)==0);assert(d->read(0,v,1,d->intf_ptr)==0);return fail_init?BMI2_E_COM_FAIL:BMI2_OK;}
int8_t bmi2_get_sensor_config(struct bmi2_sens_config*c,uint8_t n,struct bmi2_dev*d){(void)c;(void)d;assert(n==2);return 0;}
int8_t bmi2_set_sensor_config(struct bmi2_sens_config*c,uint8_t n,struct bmi2_dev*d){(void)d;assert(n==2&&c[0].cfg.acc.range==BMI2_ACC_RANGE_4G&&c[1].cfg.gyr.range==BMI2_GYR_RANGE_500);configured++;return 0;}
int8_t bmi2_sensor_enable(const uint8_t*s,uint8_t n,struct bmi2_dev*d){(void)d;assert(n==2&&s[0]==BMI2_ACCEL&&s[1]==BMI2_GYRO);return 0;}
int8_t bmi2_get_sensor_data(struct bmi2_sens_data*s,struct bmi2_dev*d){(void)d;memset(s,0,sizeof(*s));s->acc.z=8192;return fail_data?BMI2_E_COM_FAIL:0;}
#include "imu.c"
int main(void) {
    assert(initialize()&&configured==1&&reads==1&&writes==1);
    poll_imu(0);assert(imu_snapshot(0).ready&&!imu_snapshot(251).ready);
    imu_recenter();assert(!imu_snapshot(10).ready);poll_imu(25);assert(imu_snapshot(25).ready);
    fail_data=true;poll_imu(50);assert(!imu_snapshot(50).ready);fail_data=false;
    poll_imu(75);assert(imu_snapshot(75).ready);
    uint8_t data[40]={0};assert(sensor.write(1,data,33,sensor.intf_ptr)!=0);assert(sensor.read(1,data,33,sensor.intf_ptr)!=0);
    fail_init=true;assert(!initialize()&&removes);fail_init=false;missing=true;assert(!initialize());
    puts("Actual BMI270 adapter: address/chunks/timeouts/ranges, init/read failure, recenter, freshness and recovery: passed");
}
'''
with tempfile.TemporaryDirectory(prefix='anan-imu-') as work:
    work=Path(work);(work/'driver').mkdir();(work/'freertos').mkdir()
    (work/'driver/i2c_master.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
typedef void* i2c_master_bus_handle_t;
typedef void* i2c_master_dev_handle_t;
typedef struct {int dev_addr_length,device_address,scl_speed_hz;} i2c_device_config_t;
#define I2C_ADDR_BIT_LEN_7 0
#define ESP_OK 0
int i2c_master_bus_add_device(void*,const i2c_device_config_t*,void**);
int i2c_master_bus_rm_device(void*);
int i2c_master_transmit_receive(void*,const uint8_t*,size_t,uint8_t*,size_t,int);
int i2c_master_transmit(void*,const uint8_t*,size_t,int);
''')
    for name in ['FreeRTOS.h','task.h']:(work/'freertos'/name).write_text((root/'tools/hid_acceptance/freertos'/name).read_text())
    with (work/'freertos/task.h').open('a') as f:f.write('\nstatic inline void vTaskDelete(void*x){(void)x;}\n')
    (work/'esp_timer.h').write_text((root/'tools/hid_acceptance/esp_timer.h').read_text())
    (work/'esp_rom_sys.h').write_text('static inline void esp_rom_delay_us(unsigned n){(void)n;}\n')
    (work/'esp_log.h').write_text('#define ESP_LOGW(...) ((void)0)\n#define ESP_LOGI(...) ((void)0)\n')
    (work/'fixture.c').write_text(fixture);exe=work/'imu'
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-I',str(work),*include_flags(),str(work/'fixture.c'),str(root/'main/hardware/tilt.c'),'-lm','-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
