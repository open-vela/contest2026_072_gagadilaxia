#ifndef MINDFLOW_AUDIO_H
#define MINDFLOW_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* ESP32-S3 EYE 麦克风配置
 * I2S0: SDA=GPIO5, SCL=GPIO6
 * 格式: PCM S16LE 16kHz 单声道
 * 2 秒 = 64000 字节
 */
#define MF_AUDIO_SAMPLE_RATE  16000
#define MF_AUDIO_CHANNELS     1
#define MF_AUDIO_BITS         16
#define MF_AUDIO_REC_SECONDS  2
#define MF_AUDIO_BUF_SIZE     (MF_AUDIO_SAMPLE_RATE * MF_AUDIO_CHANNELS * MF_AUDIO_BITS / 8 * MF_AUDIO_REC_SECONDS)

/* 初始化 I2S 麦克风（启动时调用一次） */
int mf_audio_init(void);

/* 采集 2 秒语音到 buf
 * 返回: 0 成功, -1 无硬件 (QEMU), -2 I2S 错误
 * 成功时 *out_len = MF_AUDIO_BUF_SIZE
 */
int mf_audio_record_2s(uint8_t *buf, int *out_len);

/* 关闭 I2S */
void mf_audio_deinit(void);

#ifdef __cplusplus
}
#endif
#endif /* MINDFLOW_AUDIO_H */
