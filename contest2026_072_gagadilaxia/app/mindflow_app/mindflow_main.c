/****************************************************************************
 * Contest 2026 team 072 - MindFlow Agent - 主入口
 *
 * 功能：
 *   1. 启动 AI Agent 框架
 *   2. 加载 MindFlow 专属 Skills（专注/情绪/拖延/干预/记忆）
 *   3. 初始化 LVGL 仪表盘界面
 *   4. 启动专注度监测循环
 *
 * 板级硬件使用（ESP32-S3 EYE）：
 *   - 摄像头  → camera_capture + analyze_image（Skill 调用）
 *   - 麦克风  → voice_asr / 语音情绪分析（Skill 调用）
 *   - WiFi    → 云端 LLM（Agent 内部）
 *   - PSRAM   → 图像/音频缓冲
 *   - LCD     → LVGL 仪表盘（CONFIG_MINDFLOW_USE_LVGL）
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <nuttx/pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

/* --- 前向声明 --- */
static void *focus_monitor_loop(void *arg);

/* --- 配置 --- */
#define MINDFLOW_VERSION          "1.0.0"
#define MINDFLOW_AGENT_SKILL_DIR  "/data/agent/skills"
#define MINDFLOW_STORE_DIR        "/data/mindflow"
#define MINDFLOW_PROFILE_JSON     MINDFLOW_STORE_DIR "/profile.json"
#define MINDFLOW_FOCUS_LOG        MINDFLOW_STORE_DIR "/focus.log"
#define MINDFLOW_PROC_LOG         MINDFLOW_STORE_DIR "/procrastination_events.log"
#define MINDFLOW_TIMELINE_LOG     MINDFLOW_STORE_DIR "/timeline.log"

/* --- 默认用户画像（首次启动写入） --- */
static const char *g_default_profile =
    "{\n"
    "  \"name\": \"MindFlow 用户\",\n"
    "  \"high_focus_time\": \"09:00-11:00\",\n"
    "  \"high_risk_time\": \"15:00-17:00,22:00-23:00\",\n"
    "  \"focus_score\": 75,\n"
    "  \"procrastination_risk\": \"medium\",\n"
    "  \"emotion_baseline\": \"calm\",\n"
    "  \"daily_goals_completed\": 0,\n"
    "  \"total_focus_minutes_today\": 0,\n"
    "  \"emotion_history\": [],\n"
    "  \"summary\": \"首次启动，画像将在 3 天内逐步收敛\"\n"
    "}\n";

/* --- 写一条 timeline 事件 --- */
static void timeline_push(const char *type, const char *content)
{
    int fd = open(MINDFLOW_TIMELINE_LOG, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return;
    char buf[256];
    int n = snprintf(buf, sizeof(buf), "%ld,%s,%s\n",
                     (long)time(NULL), type, content ? content : "");
    write(fd, buf, n);
    close(fd);
}

/* --- 专注度监测循环（QEMU/开发板通用；开发板上可替换为真实摄像头+麦克风采集） --- */
static void *focus_monitor_loop(void *arg)
{
    (void)arg;
    printf("[MindFlow] 专注度监测线程已启动（每分钟模拟采样）\n");

    int minute = 0;
    while (1) {
        minute++;
        /*
         * 开发板接入真实硬件时，这里的逻辑：
         *   1. ioctl(/dev/video0) 抓一帧 → 交给 analyze_image skill
         *   2. ioctl(/dev/audio/pcm0) 录 2s → ASR + emotion skill 分析
         *   3. 写 focus.log 一行
         * QEMU 下就用伪随机近似，避免整个线程退出
         */
        int r = rand();
        int score   = 55 + (r % 45);                      /* 55~99 */
        int away    = ((r >> 3) & 0xF) == 0;              /* ~6% */
        int phone   = ((r >> 7) & 0xF) == 0;              /* ~6% */
        const char *tag = away  ? "away"
                        : phone ? "distracted"
                        : score >= 85 ? "focused" : "focused";
        const char *emo = (score < 65) ? "anxious" : "calm";

        /* 追加到 focus.log */
        int fd = open(MINDFLOW_FOCUS_LOG, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd >= 0) {
            char line[96];
            int n = snprintf(line, sizeof(line), "%ld,%d,%s,%s\n",
                             (long)time(NULL), score, tag, emo);
            write(fd, line, n);
            close(fd);
        }

        printf("[MindFlow] 第 %d 分钟 | 专注=%d %s 情绪=%s\n",
               minute, score, tag, emo);
        sleep(60);
    }
    return NULL;
}

/* --- 初始化存储目录 & 用户画像 --- */
static int init_storage(void)
{
    struct stat st;
    if (stat(MINDFLOW_STORE_DIR, &st) != 0) {
        if (mkdir(MINDFLOW_STORE_DIR, 0755) != 0) {
            /* /data 本身可能在 NSH 里还没 mount，给提示但不失败 */
            fprintf(stderr,
                    "[MindFlow] mkdir %s 失败，请先 mount /data\n",
                    MINDFLOW_STORE_DIR);
        }
    }
    if (stat(MINDFLOW_AGENT_SKILL_DIR, &st) != 0) {
        mkdir(MINDFLOW_AGENT_SKILL_DIR, 0755);
    }
    if (stat(MINDFLOW_PROFILE_JSON, &st) == 0) {
        return 0;
    }
    int fd = open(MINDFLOW_PROFILE_JSON, O_WRONLY | O_CREAT, 0644);
    if (fd < 0) return -1;
    write(fd, g_default_profile, strlen(g_default_profile));
    close(fd);

    timeline_push("profile_init", "首次启动，写入默认用户画像");
    printf("[MindFlow] 初始化 profile.json 完成\n");
    return 0;
}

/* --- 可选 LVGL 仪表盘入口（弱符号，没有 LVGL 也能链接通过） --- */
int __attribute__((weak)) mindflow_lvgl_start(void)         { return 0; }
void __attribute__((weak)) mindflow_lvgl_update_focus(int s, const char *t) { (void)s;(void)t; }

/* --- 主函数 --- */
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    printf("\n");
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║   MindFlow Agent  主动认知调节智能体      ║\n");
    printf("║   v%s  |  Team 072 · gagadilaxia           ║\n", MINDFLOW_VERSION);
    printf("╚═══════════════════════════════════════════╝\n");
    printf("[MindFlow] 目标板：ESP32-S3 EYE（摄像头+麦克风+WiFi+LCD）\n\n");

    /* 1. 存储初始化 */
    init_storage();

    /* 2. 启动后台专注监测线程 */
    pthread_t tid;
    if (pthread_create(&tid, NULL, focus_monitor_loop, NULL) != 0) {
        fprintf(stderr, "[MindFlow] 监测线程创建失败，忽略\n");
    }

    /* 3. 可选：LVGL 仪表盘（若启用了 CONFIG_MINDFLOW_USE_LVGL 则是强符号） */
    (void)mindflow_lvgl_start();
    mindflow_lvgl_update_focus(75, "boot_ok");

    /* 4. 给用户的使用提示 */
    printf("\n");
    printf("[MindFlow] 就绪！在 NSH 里通过 Agent 调用 Skills：\n");
    printf("  NSH> ask 帮我分析今天的专注度       (focus-analyzer)\n");
    printf("  NSH> ask 我感觉有点焦虑怎么办       (emotion-analyzer)\n");
    printf("  NSH> ask 我今天拖延了吗             (procrastination-detector)\n");
    printf("  NSH> ask 把毕业论文拆解成步骤       (intervention-coach)\n");
    printf("  NSH> ask 给我看长期状态画像         (profile-memory)\n");
    printf("\n");

    timeline_push("boot", "MindFlow Agent 启动完成");

    /* 5. 前台交互 shell（便于调试） */
    char line[256];
    printf("MindFlow> ");
    fflush(stdout);
    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) { printf("MindFlow> "); fflush(stdout); continue; }
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) break;
        printf("[MindFlow] 收到 '%s' → 用 'ask ...' 交给 Agent 处理\n", line);
        printf("MindFlow> ");
        fflush(stdout);
    }

    printf("[MindFlow] 退出\n");
    return 0;
}
