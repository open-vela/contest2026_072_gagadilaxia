# 长期记忆 — 数据格式规范

## 共享数据路径
所有 Skill 使用以下 5 个文件，路径固定为 /data/mindflow/

## 1. profile.json — 用户画像（JSON，完整覆盖写）

## 2. focus.log — 专注日志（追加写，CSV）
格式：`epoch,score,tag,emotion`
示例：`1723195200,87,focused,calm`
tag 枚举：focused | distracted | away | sleepy

## 3. procrastination_events.log — 拖延事件（追加写）
格式：`epoch,level(1-4),score(0-7),reason1,reason2,...`
示例：`1723195500,3,4,离开工位3次,专注分<50,处于高风险时段`

## 4. timeline.log — 干预时间线（追加写）
格式：`epoch,event_type,content_json`
event_type 枚举：
  intervention | pomodoro_start | pomodoro_end |
  profile_update | procrastination_alert | emotion_high
示例：`1723195600,intervention,"{\"task\":\"论文第一章\",\"steps\":3}"`

## 5. active_task.json — 当前任务拆解（完整覆盖写）

## 原子写入约定
写 JSON 文件时：先写 .tmp → fsync → rename 覆盖，避免断电损坏。
