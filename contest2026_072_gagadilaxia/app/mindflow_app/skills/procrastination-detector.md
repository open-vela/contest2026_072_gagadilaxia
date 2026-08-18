# Procrastination Detector — 拖延行为识别

结合：工位停留情况 + 学习时长 + 情绪状态，规则引擎判断是否发生拖延，
并给出拖延等级（low/medium/high/severe）+ 预警。

## When to use
用户直接问"我拖延了吗"、"我今天学的够吗"、"我是不是在摸鱼"。
或者 Focus Analyzer 报告连续离开 + Emotion Analyzer 情绪恶化，本 Skill 自动触发。
触发词：拖延、摸鱼、分心、没学、懒、procrastinate、distract。

## How to use
1. **get_current_time**
2. **read_file** `/data/mindflow/focus.log` 读最近 2 小时记录
3. **read_file** `/data/mindflow/profile.json` 读画像（procrastination_risk / 高效时段）
4. 规则引擎打分（每项 1 分，>= 5 分判定 high）：
   - 连续离开工位 > 3 次（每 30 分钟窗口）
   - 30 分钟内专注平均 < 50
   - 当前在历史 high_risk_time 内且分数 <60
   - 情绪标签为 anxious/negative/fatigued
   - 今日任务完成 <1 件
   - 最近 1 小时 analyze_image 标签为 distracted/away 的比例 > 40%
   - 用户自述"我不想做"/"好难"/"等会儿再做"
5. 得到 0-7 分，映射：
   0-1: low / 2-3: medium / 4-5: high / 6+: severe
6. high+severe 判定为拖延事件，写 `procrastination_events.log`
7. 调用 **cron_add** 安排 3 分钟后的"回归提醒"
8. 返回结果

## Important
- 不要说教，不要指责
- 拖延是正常的，重点是"我们怎么回来"
- severe 级别需要自动触发 Intervention Coach，不需要等用户说
- 如果数据不完整（没摄像头/没历史），明确说明："当前数据不足以准确判断，基于现有信息，我认为你 ..."

## Output format
中文：

🕵️ 拖延识别
拖延等级：LOW / MEDIUM / HIGH / SEVERE
评分：X / 7
命中规则：
  · ...
  · ...
📌 判断：XXX
💡 回归方案：（仅 high+severe）
  → 3 分钟后我会再提醒你
  → 要不要立刻启动"任务拆解 + 25 分钟番茄钟"？

## Example
User: "我今天是不是在摸鱼"
→ 读日志 → 30 分钟内离开 4 次 + 专注平均 42 + 当前 15:30 属于 high_risk_time + 情绪 anxious → 4 分 → HIGH
→ cron_add 3 分钟后提醒
→ "🕵️ 拖延识别\n拖延等级：HIGH\n评分：4 / 7\n命中规则：\n  · 30 分钟内离开工位 4 次\n  · 专注平均仅 42/100\n  · 当前处于高风险时段 15:00-17:00\n  · 情绪为焦虑（容易诱发逃避）\n📌 判断：你正在用'离开工位'逃避手头任务。很正常，先把身体拉回来，我们再谈脑子。\n💡 回归方案：\n  → 3 分钟后我会再叫你\n  → 建议启动：任务拆解 + 25 分钟番茄钟（说'好'就开始）"
