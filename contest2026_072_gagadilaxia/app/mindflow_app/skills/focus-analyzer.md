# Focus Analyzer — 专注状态检测

基于摄像头画面持续感知用户工位状态，生成专注度评分与趋势。
端侧轻量判断 + LLM 综合分析。

## When to use
当用户询问专注度、学习效率、是否在摸鱼、今天学了多久、我今天表现怎么样、工位状态。
触发词：专注、学习、效率、摸鱼、工位、坐了多久、今天学了、focus。

## How to use
1. **get_current_time** 获取当前时间和日期
2. **camera_capture** 拍一张当前工位照片
3. **analyze_image** 分析内容，prompt 固定：
   "请从这张照片判断：[1]是否有人（0/1） [2]人是否在看屏幕（0/1）[3]是否在玩手机（0/1）[4]是否打瞌睡（0/1）[5]状态标签（focused/distracted/away/sleepy）。只输出 JSON，例如 {\"person\":1,\"screen\":1,\"phone\":0,\"sleepy\":0,\"tag\":\"focused\"}"
4. **read_file** 读取 `/data/mindflow/focus.log`（每一行形如：epoch,score,tag,emotion），获取当天历史
5. 结合历史，计算 0-100 专注评分，并给出标签
6. 同时**write_file** 把当前这分钟的结果追加到 focus.log，格式：
   `now_epoch,score,tag,calm\n`
7. 给出简洁结论 + 建议（用户的语言，中文）

## Important
- **必须执行 camera_capture + analyze_image 拿到真实数据，不要编造**
- 如果 camera_capture 失败（如 QEMU 无摄像头），则：
  从 `/data/mindflow/profile.json` 里读 focus_score 作为近似
  明确提示："⚠️ 摄像头不可用，使用历史画像数据作为近似，请接入开发板获得真实检测"
- 输出建议时，要具体：例如 "建议站起来活动 3 分钟" 而不是 "你应该休息"
- 如果离开工位超过 10 分钟，提醒回来

## Output format
中文回复，结构：

📊 今日专注度总结
总分：X/100 （趋势：↗↘→）
  在工位：X%   看屏幕：X%   离开：X%   玩手机：X%
  连续专注最长：X 分钟
  最高效时段：HH:MM-HH:MM
  ⚠️ 风险：连续离开次数 N / 长时间发呆 M 次
💡 建议：
  1. ...
  2. ...

## Example
User: "我今天的专注度怎么样"
→ get_current_time → 2026-08-09 15:30
→ camera_capture → 拿到图
→ analyze_image → {person:1, screen:1, phone:0, sleepy:0, tag:focused}
→ read_file focus.log → 今天已有 42 条记录，平均 78 分
→ write_file 追加新一行
→ "📊 今日专注度总结\n总分：78/100 （趋势 → 平稳）\n  在工位：92%  看屏幕：81%  离开：8%  玩手机：6%\n  连续专注最长：68 分钟\n  最高效时段：09:10-10:20\n  ⚠️ 风险：下午 14:00 左右曾离开工位 3 次\n💡 建议：\n  1. 当前状态很好，再坚持 30 分钟\n  2. 16:00 提醒你起身倒水\n  3. 手机放回抽屉，降低干扰"
