#ifndef TEST_TWAI_H
#define TEST_TWAI_H
#include <stdint.h>
#include "../esp_err.h"
typedef int gpio_num_t;
struct twai_general_config_t { int tx_io,rx_io,mode,tx_queue_len,rx_queue_len; };
struct twai_timing_config_t { int unused; };
struct twai_filter_config_t { int unused; };
struct twai_message_t { uint32_t identifier; uint8_t extd,rtr,data_length_code,data[8]; };
struct twai_status_info_t { int state; uint32_t tx_error_counter,rx_error_counter,msgs_to_tx,msgs_to_rx,tx_failed_count,rx_missed_count,bus_error_count,arb_lost_count; };
#define TWAI_MODE_NORMAL 0
#define TWAI_GENERAL_CONFIG_DEFAULT(tx,rx,mode) twai_general_config_t{tx,rx,mode,5,5}
#define TWAI_TIMING_CONFIG_500KBITS() twai_timing_config_t{0}
#define TWAI_FILTER_CONFIG_ACCEPT_ALL() twai_filter_config_t{0}
extern esp_err_t testTwaiInstallResult,testTwaiStartResult,testTwaiTxResult,testTwaiRxResult;
extern twai_message_t testTwaiTx,testTwaiRx;
extern int testTwaiTxCount;
inline esp_err_t twai_driver_install(const twai_general_config_t*,const twai_timing_config_t*,const twai_filter_config_t*){return testTwaiInstallResult;}
inline esp_err_t twai_start(){return testTwaiStartResult;}
inline esp_err_t twai_driver_uninstall(){return ESP_OK;}
inline esp_err_t twai_transmit(const twai_message_t* m,uint32_t){testTwaiTx=*m;testTwaiTxCount++;return testTwaiTxResult;}
inline esp_err_t twai_receive(twai_message_t* m,uint32_t){if(testTwaiRxResult==ESP_OK)*m=testTwaiRx;return testTwaiRxResult;}
inline esp_err_t twai_get_status_info(twai_status_info_t* s){*s={};return ESP_OK;}
#endif
