#ifndef __MINDFLOW_LVGL_H
#define __MINDFLOW_LVGL_H

#ifdef __cplusplus
extern "C" {
#endif

/* MindFlow 仪表盘 LVGL 界面
 * 依赖：CONFIG_LVGL && CONFIG_VELA_LVGL
 * 没有 LVGL 编译时整个 ui/ 会被忽略，mindflow_main.c 里有 __attribute__((weak)) 占位
 */

int  mindflow_lvgl_start(void);
void mindflow_lvgl_update_focus(int score, const char *tag);
void mindflow_lvgl_update_emotion(const char *emotion, int pressure, int fatigue);
void mindflow_lvgl_push_message(const char *icon, const char *line1, const char *line2);

#ifdef __cplusplus
}
#endif
#endif /* __MINDFLOW_LVGL_H */
