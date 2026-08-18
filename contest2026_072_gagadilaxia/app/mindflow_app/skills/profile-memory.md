# Profile Memory — 长期状态记忆（成员四）

聚合 focus.log + procrastination_events.log + timeline.log，生成用户画像。

## When to use
用户问："我什么时候最有效率"、"我这周怎么样"、"我的状态画像"、"总结这一周"。
触发词：画像、记忆、总结、规律、最高效、趋势、profile、summary。

## How to use
1. **get_current_time** — 确定时间范围（默认最近 7 天）
2. **read_file** profile.json — 读当前画像
3. **read_file** focus.log — 读近 7 天所有记录
4. **read_file** procrastination_events.log — 读拖延事件
5. **read_file** timeline.log — 读干预时间线
6. 聚合：
   - 按小时统计 focus score 平均 → 高效时段 / 低效时段
   - 按小时统计拖延事件数 → 高风险时段
   - emotion_history 滑动平均 → 情绪趋势
   - daily_goals_completed 累计 → 周完成率
7. **mf_profile_update_summary** — 写回 profile.json 的 summary 字段
8. 中文输出画像摘要 + 3 条可执行建议

## 数据不足处理
- 日志 < 3 天："数据不足，当前初步画像..."
- 日志 < 1 天："先积累几天数据再来看画像哦"

## Output format

## Example
User: "我最高效的时间段"
→ 读 focus.log 7 天
→ 按小时聚合 → 09-11 平均 89 分
→ "🧠 MindFlow 长期画像\n【高效时段】09:00-11:00（平均 89 分）\n【风险时段】14:00-16:00（拖延率 52%）\n💡 建议：把最难任务排到上午 9-11 点"
