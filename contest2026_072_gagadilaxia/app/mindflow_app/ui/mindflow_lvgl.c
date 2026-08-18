/****************************************************************************
 * MindFlow LVGL 仪表盘界面
 *  三屏：
 *   1. HOME：专注度圆环 + 状态 + 情绪
 *   2. INSIGHT：今日数据 + 最高效时段 + 拖延事件数
 *   3. INTERVENE：当前任务拆解 + 番茄钟按钮
 *
 *  若关闭 CONFIG_MINDFLOW_USE_LVGL，文件末尾提供同名弱符号占位，
 *  保证 mindflow_main.c 不用改一行代码也能链接通过。
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_MINDFLOW_USE_LVGL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <lvgl/lvgl.h>
#include "mindflow_lvgl.h"

/* ---------- 全局控件句柄 ---------- */
static lv_obj_t *g_root        = NULL;
static lv_obj_t *g_focus_arc   = NULL;
static lv_obj_t *g_focus_lbl   = NULL;
static lv_obj_t *g_tag_lbl     = NULL;
static lv_obj_t *g_emotion_lbl = NULL;
static lv_obj_t *g_msg_box     = NULL;

/* ---------- 颜色 ---------- */
#define MINDFLOW_COLOR_BG      lv_color_hex(0x0f172a)
#define MINDFLOW_COLOR_ACCENT  lv_color_hex(0x22d3ee)
#define MINDFLOW_COLOR_WARN    lv_color_hex(0xf59e0b)
#define MINDFLOW_COLOR_DANGER  lv_color_hex(0xef4444)
#define MINDFLOW_COLOR_TEXT    lv_color_hex(0xe2e8f0)

static lv_color_t score_color(int score)
{
    if (score >= 80) return MINDFLOW_COLOR_ACCENT;
    if (score >= 55) return MINDFLOW_COLOR_WARN;
    return MINDFLOW_COLOR_DANGER;
}

/* ---------- 首页 ---------- */
static void build_home_tab(lv_obj_t *parent)
{
    lv_obj_set_style_bg_color(parent, MINDFLOW_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);

    lv_obj_t *title = lv_label_create(parent);
    lv_label_set_text(title, "MindFlow · 专注搭子");
    lv_obj_set_style_text_color(title, MINDFLOW_COLOR_ACCENT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    g_focus_arc = lv_arc_create(parent);
    lv_arc_set_rotation(g_focus_arc, 270);
    lv_arc_set_bg_angles(g_focus_arc, 0, 360);
    lv_obj_remove_style(g_focus_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(g_focus_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_value(g_focus_arc, 0);
    lv_obj_set_size(g_focus_arc, 180, 180);
    lv_obj_align(g_focus_arc, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_arc_color(g_focus_arc, MINDFLOW_COLOR_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(g_focus_arc, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(g_focus_arc, lv_color_hex(0x1e293b), LV_PART_MAIN);
    lv_obj_set_style_arc_width(g_focus_arc, 14, LV_PART_MAIN);

    g_focus_lbl = lv_label_create(parent);
    lv_label_set_text(g_focus_lbl, "--");
    lv_obj_set_style_text_font(g_focus_lbl, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_focus_lbl, MINDFLOW_COLOR_TEXT, 0);
    lv_obj_align_to(g_focus_lbl, g_focus_arc, LV_ALIGN_CENTER, 0, -10);

    g_tag_lbl = lv_label_create(parent);
    lv_label_set_text(g_tag_lbl, "等待数据");
    lv_obj_set_style_text_color(g_tag_lbl, MINDFLOW_COLOR_TEXT, 0);
    lv_obj_align_to(g_tag_lbl, g_focus_arc, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    g_emotion_lbl = lv_label_create(parent);
    lv_label_set_text(g_emotion_lbl, "情绪: --  压力: --/100");
    lv_obj_set_style_text_color(g_emotion_lbl, MINDFLOW_COLOR_WARN, 0);
    lv_obj_align(g_emotion_lbl, LV_ALIGN_BOTTOM_MID, 0, -14);
}

/* ---------- Insight 页 ---------- */
static void build_insight_tab(lv_obj_t *parent)
{
    lv_obj_set_style_bg_color(parent, MINDFLOW_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);

    lv_obj_t *t = lv_label_create(parent);
    lv_label_set_text(t, "今日数据");
    lv_obj_set_style_text_color(t, MINDFLOW_COLOR_ACCENT, 0);
    lv_obj_align(t, LV_ALIGN_TOP_LEFT, 12, 10);

    lv_obj_t *body = lv_label_create(parent);
    lv_label_set_text(body,
        "  今日专注：--- 分钟\n"
        "  最高效时段：--:-- ~ --:--\n"
        "  拖延事件：-- 次\n"
        "  任务完成：-- / --\n"
        "\n"
        "  查看完整画像：\n"
        "  NSH> ask 给我看长期画像"
    );
    lv_obj_set_style_text_color(body, MINDFLOW_COLOR_TEXT, 0);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 12, 40);
}

/* ---------- Intervene 页 ---------- */
static void btn_start_cb(lv_event_t *e)
{
    (void)e;
    mindflow_lvgl_push_message("tomato",
        "番茄钟启动！",
        "25 分钟后提醒你休息");
}

static void build_intervene_tab(lv_obj_t *parent)
{
    lv_obj_set_style_bg_color(parent, MINDFLOW_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);

    lv_obj_t *t = lv_label_create(parent);
    lv_label_set_text(t, "当前任务");
    lv_obj_set_style_text_color(t, MINDFLOW_COLOR_ACCENT, 0);
    lv_obj_align(t, LV_ALIGN_TOP_LEFT, 12, 10);

    lv_obj_t *steps = lv_label_create(parent);
    lv_label_set_text(steps,
        "  （尚无拆解任务）\n"
        "\n"
        "  去 NSH 里说：\n"
        "  ask 帮我把 <任务> 拆开"
    );
    lv_obj_set_style_text_color(steps, MINDFLOW_COLOR_TEXT, 0);
    lv_obj_align(steps, LV_ALIGN_TOP_LEFT, 12, 40);

    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 200, 42);
    lv_obj_set_style_bg_color(btn, MINDFLOW_COLOR_ACCENT, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_add_event_cb(btn, btn_start_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *bt = lv_label_create(btn);
    lv_label_set_text(bt, "启动番茄钟 25m");
    lv_obj_center(bt);
}

/* ---------- 推送消息 ---------- */
static void msgbox_close_cb(lv_event_t *e)
{
    lv_obj_t *m = lv_event_get_target(e);
    if (m) lv_obj_delete_async(m);
    g_msg_box = NULL;
}

void mindflow_lvgl_push_message(const char *icon, const char *line1, const char *line2)
{
    if (!g_root) return;
    if (g_msg_box) {
        lv_obj_delete_async(g_msg_box);
        g_msg_box = NULL;
    }

    static const char *btns[] = {"知道了", ""};
    char txt[192];
    snprintf(txt, sizeof(txt), "%s\n%s\n%s",
             icon  ? icon  : "",
             line1 ? line1 : "",
             line2 ? line2 : "");

    g_msg_box = lv_msgbox_create(g_root);
    lv_msgbox_add_title(g_msg_box, "MindFlow");
    lv_msgbox_add_text(g_msg_box, txt);
    lv_msgbox_add_close_button(g_msg_box);
    for (size_t i = 0; btns[i][0]; i++) {
        lv_msgbox_add_footer_button(g_msg_box, btns[i]);
    }
    lv_obj_add_event_cb(g_msg_box, msgbox_close_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(g_msg_box);
}

/* ---------- 更新数据 ---------- */
void mindflow_lvgl_update_focus(int score, const char *tag)
{
    if (!g_focus_arc || !g_focus_lbl || !g_tag_lbl) return;
    if (score < 0) score = 0;
    if (score > 100) score = 100;

    lv_arc_set_value(g_focus_arc, score);
    lv_obj_set_style_arc_color(g_focus_arc, score_color(score), LV_PART_INDICATOR);

    char buf[16];
    snprintf(buf, sizeof(buf), "%d", score);
    lv_label_set_text(g_focus_lbl, buf);

    const char *t = tag ? tag : "--";
    lv_obj_set_style_text_color(g_tag_lbl, score_color(score), 0);
    lv_label_set_text_fmt(g_tag_lbl, "# %s", t);
}

void mindflow_lvgl_update_emotion(const char *emotion, int pressure, int fatigue)
{
    if (!g_emotion_lbl) return;
    lv_label_set_text_fmt(g_emotion_lbl,
        "情绪: %s  压力:%d/100  疲劳:%d/100",
        emotion ? emotion : "--", pressure, fatigue);
}

/* ---------- 入口 ---------- */
int mindflow_lvgl_start(void)
{
    printf("[MindFlow][LVGL] 构建仪表盘界面...\n");
    g_root = lv_scr_act();
    lv_obj_set_style_bg_color(g_root, MINDFLOW_COLOR_BG, 0);

    lv_obj_t *tv = lv_tabview_create(g_root);
    lv_obj_set_size(tv, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(tv, MINDFLOW_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(tv, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(tv, MINDFLOW_COLOR_TEXT, LV_PART_TABS | LV_PART_ITEMS);

    lv_obj_t *tab1 = lv_tabview_add_tab(tv, "首页");
    lv_obj_t *tab2 = lv_tabview_add_tab(tv, "数据");
    lv_obj_t *tab3 = lv_tabview_add_tab(tv, "干预");

    build_home_tab(tab1);
    build_insight_tab(tab2);
    build_intervene_tab(tab3);

    mindflow_lvgl_update_focus(75, "focused");
    mindflow_lvgl_update_emotion("calm", 42, 30);
    mindflow_lvgl_push_message("hi", "MindFlow 已启动",
                              "问我：今天我表现怎么样？");

    printf("[MindFlow][LVGL] 界面构建完成\n");
    return 0;
}

#else /* CONFIG_MINDFLOW_USE_LVGL 未启用 → 提供弱符号占位 */

#include "mindflow_lvgl.h"
int  __attribute__((weak)) mindflow_lvgl_start(void)                              { return 0; }
void __attribute__((weak)) mindflow_lvgl_update_focus(int s, const char *t)       { (void)s;(void)t; }
void __attribute__((weak)) mindflow_lvgl_update_emotion(const char *e, int p, int f) { (void)e;(void)p;(void)f; }
void __attribute__((weak)) mindflow_lvgl_push_message(const char *i, const char *l1, const char *l2) { (void)i;(void)l1;(void)l2; }
#endif
