/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-08-05 18:53:12
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-10-22 14:25:54
 * @Description  : 
 */

#include "ipmi_sdr.h"
#include "ipmi_event.h"
#include "ipmi.h"
#include <string.h>
#include "platform.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"

typedef struct {
    uint8_t sdr_id;
    void* init_arg;
    uint8_t read_arg;
    void (*sensor_init)(void*);
    /* 返回结果需补全为16位(有符号的数据) */
    uint16_t (*sensor_read)(uint8_t);
    ipmi_sdr sdr;
}sensor_info;

static sensor_info sdr_list[SENSOR_MAX_NUMBER];
static uint16_t sdr_count = 0;
static TimerHandle_t update_handler_timer;
static void update_handler_callback(TimerHandle_t xTimer);
static uint8_t battery_flag = 0;

#define MIN(a, b) (a > b ? b : a)
#define FILL_SDR_STRUCT(p_sensor_info, no, id, type, unit, sig, higher, lower, read_handler, readArg, init_func, initArg, M, K, name) \
    {                                                                                                                                 \
        (p_sensor_info)->sdr.sensor_no = no;                                                                                          \
        (p_sensor_info)->sdr.ipmc_addr = BMC_ADDR;                                                                                    \
        (p_sensor_info)->sdr_id = id;                                                                                                 \
        (p_sensor_info)->sdr.sensor_type = type;                                                                                      \
        (p_sensor_info)->sdr.unit_code = unit;                                                                                        \
        (p_sensor_info)->sdr.is_signed = sig;                                                                                         \
        (p_sensor_info)->sdr.read_data = 0;                                                                                           \
        (p_sensor_info)->sdr.higher_threshold = higher;                                                                               \
        (p_sensor_info)->sdr.lower_threshold = lower;                                                                                 \
        (p_sensor_info)->sensor_read = read_handler;                                                                                  \
        (p_sensor_info)->read_arg = readArg;                                                                                          \
        (p_sensor_info)->sensor_init = init_func;                                                                                     \
        (p_sensor_info)->init_arg = initArg;                                                                                          \
        (p_sensor_info)->sdr.argM = M;                                                                                                \
        (p_sensor_info)->sdr.argK = K;                                                                                                \
        memset((p_sensor_info)->sdr.sensor_name, 0, SENSOR_NAME_MAX_LEN);                                                             \
        strncpy((p_sensor_info)->sdr.sensor_name, name, MIN(strlen(name), SENSOR_NAME_MAX_LEN));                                      \
        (p_sensor_info)->sdr.name_len = MIN(strlen(name), SENSOR_NAME_MAX_LEN);                                                       \
        sdr_count += 1;                                                                                                               \
    }

void init_ipmi_sdr(void)
{
    uint8_t i = 0;

    FILL_SDR_STRUCT(&(sdr_list[0]), 1, 1,
                    SENSOR_TYPE_POWER,
                    SENSOR_UNIT_CODE_A,
                    0, 0xFFFF, 0x000A,
                    get_channel_data, 0,
                    init_adc, NULL,
                    30961, -7, "12V_MON");

    FILL_SDR_STRUCT(&(sdr_list[BATTERY_INX]), 2, 2,
                    SENSOR_TYPE_VOLTAGE,
                    SENSOR_UNIT_CODE_V,
                    0, 0xFFFF, 0x4D9,
                    get_channel_data, 1,
                    sdr_init_battery_pin, NULL,
                    16113, -7, "BAT_MON");

    FILL_SDR_STRUCT(&(sdr_list[2]), 3, 3,
                    SENSOR_TYPE_VOLTAGE,
                    SENSOR_UNIT_CODE_V,
                    0, 0xFFFF, 0x0000,
                    get_channel_data, 2,
                    NULL, NULL,
                    16113, -7, "GBE_1V1");

    FILL_SDR_STRUCT(&(sdr_list[3]), 4, 4,
                    SENSOR_TYPE_VOLTAGE,
                    SENSOR_UNIT_CODE_V,
                    0, 0xFFFF, 0x0000,
                    get_channel_data, 3,
                    NULL, NULL,
                    16113, -7, "P3V3");

    FILL_SDR_STRUCT(&(sdr_list[4]), 5, 5,
                    SENSOR_TYPE_TEMPERATURE,
                    SENSOR_UNIT_CODE_DC,
                    1, 0xFFF, 0x0000,
                    read_nct75_row_data, SENSOR_NCT75_1_ADDR,
                    NULL, NULL,
                    625, -4, "TMP_A");

    FILL_SDR_STRUCT(&(sdr_list[5]), 6, 6,
                    SENSOR_TYPE_TEMPERATURE,
                    SENSOR_UNIT_CODE_DC,
                    1, 0xFFF, 0x0000,
                    read_nct75_row_data, SENSOR_NCT75_2_ADDR,
                    NULL, NULL,
                    625, -4, "TMP_B");

    for (i = 0; i < SENSOR_MAX_NUMBER; i++) {
        if (sdr_list[i].sensor_init != NULL) {
            sdr_list[i].sensor_init(sdr_list[i].init_arg);
        }
    }

    update_handler_timer = xTimerCreate("sen", 3000, pdTRUE, (void *)1, update_handler_callback);
    if (update_handler_timer != NULL) {
        PRINTF("create update_handler\r\n");
    }
    xTimerStart(update_handler_timer, 0);
}

static int get_index(uint8_t id)
{
    int ret = -1;
    uint16_t i = 0;

    if (sdr_count > 0) {
        if (id == 0) {
            ret = 0;
        } else {
            for (i = 0; i < sdr_count; i++) {
                if (sdr_list[i].sdr_id == id)
                    ret = i;
            }
        }
    }

    return ret;
}

/***
 * @brief 根据sdr id获取指定sdr，传入0则是获取第一个sdr
 * @param id [uint8_t]    sdr id
 * @param sdr [ipmi_sdr*] [out]传入一段空间，存放返回的sdr, sensor no为0则是没有指定sdr
 * @return [uint8_t] 下一个sdr的id，返回0则是没有下一个sdr
 */
uint8_t get_sdr_by_id(uint8_t id, ipmi_sdr *const sdr)
{
    uint8_t ret = 0;
    int index = 0;

    memset(sdr, 0, sizeof(ipmi_sdr));
    index = get_index(id);

    if (index == -1) {  /* 查找的sdr不存在 */
        ret = 0;
    } else {
        memcpy(sdr, &(sdr_list[index].sdr), sizeof(ipmi_sdr));
        if (index < (sdr_count - 1))    /* 后面还有sdr */
            ret = sdr_list[index + 1].sdr_id;
    }

    return ret;
}

uint16_t get_sdr_count(void)
{
    return sdr_count;
}

void update_sensor(void)
{
    uint8_t i = 0;
    uint16_t data = 0;

    for (i = 0; i < sdr_count; i++) {
        /* 更新传感器读数 */
        if (i == BATTERY_INX) {
            if (battery_flag == 1) {
                OS_PRINTF("Battery flag SET, continue\r\n");
                continue;
            } else {
                battery_flag = 1;
                /* close battery */
                close_battery_pin();
            }
        }
        data = sdr_list[i].sensor_read(sdr_list[i].read_arg);
        sdr_list[i].sdr.read_data = data;
        
        PRINTF("id: %d, data: %d\r\n", sdr_list[i].sdr_id, data);
        
        /* 是否超出阈值 */
        is_over_value(&(sdr_list[i].sdr));
    }
    
    PRINTF("\r\n");
}

static void update_handler_callback(TimerHandle_t xTimer)
{
    update_sensor();
}
