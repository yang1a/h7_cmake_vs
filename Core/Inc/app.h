/**
 * @file app.h
 * @brief 应用层主控逻辑接口
 */

#ifndef APP_H
#define APP_H

/**
 * @brief 初始化应用层
 */
void app_init(void);

/**
 * @brief 运行一次主控流程
 */
void app_run_once(void);

#endif // APP_H
