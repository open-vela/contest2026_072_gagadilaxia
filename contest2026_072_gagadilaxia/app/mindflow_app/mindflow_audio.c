/************************************************************
 * MindFlow Audio — ESP32-S3 EYE 麦克风采集实现（成员四）
 *
 * 硬件: ESP32-S3 I2S0, GPIO5(SDA) / GPIO6(SCL)
 * 格式: PCM S16LE 16kHz 单声道, 2 秒 = 64KB
 *
 * QEMU 下自动返回 -1 (无硬件), 上层 emotion-analyzer 降级为文本分析
 * 接硬件时: 打开 CONFIG_MF_AUDIO_HW, 接入真实 I2S 驱动
 ************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include "mindflow_audio.h"

/* 硬件可用标志：编译时如果没启用 I2S，直接返回 QEMU 模式 */
#ifdef CONFIG_MF_AUDIO_HW
  #include <arch/board/board.h>
  #include <driver/i2s.h>
  #define MF_HW_AVAILABLE  1
#else
  #define MF_HW_AVAILABLE  0
#endif

static int  g_audio_inited = 0;
static int  g_hw_available = 0;

int mf_audio_init(void)
{
    if (g_audio_inited) return 0;

#if MF_HW_AVAILABLE
    /* TODO: 接入真实 ESP32-S3 I2S0 初始化
     * i2s_driver_install(I2S_NUM_0, &i2sconfig, 0, NULL);
     * i2s_set_pin(I2S_NUM_0, &pins);
     * i2s_set_sample_rates(I2S_NUM_0, 16000);
     * 实际引脚请查 ESP32-S3 EYE 原理图
     */
    g_hw_available = 1;
    printf("[MindFlow][Audio] I2S0 初始化完成（真实硬件模式）\n");
#else
    g_hw_available = 0;
    printf("[MindFlow][Audio] QEMU 模式，音频硬件不可用\n");
#endif

    g_audio_inited = 1;
    return 0;
}

int mf_audio_record_2s(uint8_t *buf, int *out_len)
{
    if (!g_audio_inited) mf_audio_init();
    if (!buf || !out_len) return -3;

    memset(buf, 0, MF_AUDIO_BUF_SIZE);
    *out_len = 0;

    if (!g_hw_available) {
        /* QEMU / 无硬件：返回错误码，上层降级为文本分析 */
        return -1;
    }

#if MF_HW_AVAILABLE
    /* TODO: 真实 I2S 读取
     * int bytes_read = i2s_read(I2S_NUM_0, buf, MF_AUDIO_BUF_SIZE,
     *                           &out_len, portMAX_DELAY);
     * if (bytes_read < 0) return -2;
     * return 0;
     */
    return -2; /* 桩实现：未接硬件时走这里 */
#else
    return -1;
#endif
}

void mf_audio_deinit(void)
{
    if (g_audio_inited) {
#if MF_HW_AVAILABLE
        /* i2s_driver_uninstall(I2S_NUM_0); */
#endif
        g_audio_inited = 0;
        g_hw_available  = 0;
        printf("[MindFlow][Audio] 已关闭\n");
    }
}
