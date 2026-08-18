# Intervention Coach — 主动干预 & 任务拆解

检测到拖延/焦虑后，或用户主动请求时，调用大模型生成：
任务拆解方案、恢复专注建议、时间规划建议，并通过 LVGL/通知推送。

## When to use
用户说"帮我拆任务"、"我做不完怎么办"、"帮我制定计划"、"我不想学"、"我没状态"，
或 Procrastination Detector (high/severe) / Emotion Analyzer (压力>80) 自动触发。
触发词：拆解、计划、帮我做、番茄、pomodoro、干预、coach、intervention。

## How to use
1. **get_current_time**
2. 确认用户要拆解什么任务：如果对话中没说清，先问一句"你想先拆解哪件事？"
3. **read_file** `/data/mindflow/profile.json` 拿画像（最容易拖延的任务类型、高效时段）
4. 调用 LLM 生成拆解，prompt 固定：
   "你是拖延干预教练。用户任务：<TASK>。用户画像：高效时段=H，容易拖延类型=R。
   请输出 JSON：
   {\"title\": \"三句话任务名称\",
    \"today_steps\": [
      {\"step\": 1, \"name\": \"<20 字>\", \"pomodoro\": 1, \"hint\": \"<10 字小技巧>\"},
      {\"step\": 2, \"name\": \"...\", \"pomodoro\": 1, \"hint\": \"...\"}
    ],
    \"next_action\": \"现在就做哪一步\",
    \"reward\": \"完成后奖励自己什么（具体，可执行）\",
    \"encouragement\": \"一句温暖鼓励\"
   } 只输出 JSON"
5. **write_file** 把拆解结果写到 `/data/mindflow/active_task.json`
6. **cron_add** 每 25 分钟提醒一次进度检查（番茄钟）
7. **write_file** 追加到 `/data/mindflow/timeline.log` 记录干预事件
8. 用中文简洁展示拆解结果，LVGL 通道同步推送

## Important
- 今天能做的步骤最多 3 步，不要贪多，反而诱发下一轮拖延
- 每一步必须"具体到能直接开始动手"，不能是"查资料"这种模糊项
- 奖励必须是即时的小奖励（喝杯饮料、刷 3 分钟视频），不能是远期奖励
- 番茄钟 25/5，不要 50/10，太长坚持不住
- 不要用鸡汤文，要具体

## Output format
中文：

🎯 干预 & 拆解：「XXXX」
📌 今天只做这 3 步：
  1) XXXXXXXXXXXXX  🍅 × N    小技巧：XXXXX
  2) ...
  3) ...
👉 现在就做：第 1 步，现在是你的高效时段，先冲
🍎 完成奖励：XXXXXXXXXXXXXXXX
💬 XX
⏰ 已设置 25 分钟番茄提醒。

## Example
User: "我不想写毕业论文，帮我拆"
→ 先确认 → "好的，我们先拆'论文初稿第一章'"
→ 读画像 → 高效 09:00-11:00，易拖延类型："大任务"
→ LLM 拆解 → today_steps 3 步
→ 写 active_task.json
→ cron_add 25 min 番茄提醒
→ "🎯 干预 & 拆解：「论文初稿第一章」\n📌 今天只做这 3 步：\n  1) 新建文档，写 3 条一级标题 + 各写 1 句摘要  🍅×1    技巧：不用完美，占位就行\n  2) 查 3 篇相关文献，各粘 1 句引用到对位置  🍅×1   技巧：只看摘要\n  3) 把 1.1 节写满 200 字就停  🍅×1                    技巧：用语音输入也行\n👉 现在就做：第 1 步，3 分钟能搞定\n🍎 完成奖励：下楼买你喜欢的冰美式\n💬 你第一章都拆完了，怕什么。先迈出一步。\n⏰ 已设 25 分钟番茄提醒。"
