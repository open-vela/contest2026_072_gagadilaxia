# Emotion Analyzer — 语音情绪分析（成员四）

通过 ESP32-S3 EYE 麦克风采集 → ASR → LLM 情绪分析。
识别：焦虑、疲劳、消极、压力、平静、兴奋。

## When to use
用户语音流露消极（"写不完"、"好烦"、"好累"），或直接询问情绪/压力。
触发词：压力、焦虑、累、烦、情绪、emotion、stress、anxiety。

## How to use
1. **get_current_time** — 拿时间戳
2. **mf_audio_record_2s** — 调用成员四的语音采集接口
- 硬件：ESP32-S3 I2S0，引脚：SDA=GPIO5，SCL=GPIO6
- 格式：PCM S16LE 16kHz 单声道，2 秒 = 64000 字节
- 失败（QEMU 或无硬件）：返回错误码，降级为纯文本分析
3. **voice_asr** — 把 PCM 送 ASR，拿到转写文本
4. LLM 情绪分析（严格 JSON 输出）：
5. **read_file** profile.json — 取 emotion_baseline
6. 和基线对比（↗ 更平静 / → 持平 / ↘ 更焦虑疲劳）
7. **mf_profile_update_emotion** — 写回 emotion_history（保留最近 20 条）
8. **mf_timeline_push** — 记录 emotion_high 事件（压力>80 时）
9. 中文情绪分析 + 温和建议

## Fallback（无硬件/QEMU）
mf_audio_record_2s 失败时：
- 用用户输入的文本直接做 LLM 分析
- 回复开头加："⚠️ 使用文本近似分析，请接开发板麦克风获得更准确的语音情绪"

## 自动触发条件
- 压力 > 80 → 主动触发 Intervention Coach
- 连续 3 次 anxious/negative/fatigued → 主动干预

## Output format

## Example
User: "我根本写不完，好烦啊"
→ get_current_time
→ mf_audio_record_2s → PCM 64KB
→ voice_asr → "我根本写不完好烦啊"
→ LLM → emotion:anxious, pressure:85, fatigue:72
→ read profile → baseline: calm
→ mf_profile_update_emotion
→ mf_timeline_push (因为压力85>80)
→ "🧠 情绪分析\n当前情绪：焦虑  压力：85/100  疲劳：72/100\n判断依据：deadline焦虑+挫败感\n对比基线：↘ 明显恶化\n🤝 我想对你说：\n  先停一下，深呼吸三次。任务大就拆成小块。"
