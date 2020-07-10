
/*
 * Copyright (C) Zhu Jiashun
 * Copyright (C) Zaver
 */
/*Nginx把timer实现成了rbtree，这就很奇怪，
timer模块需要频繁找最小的key（最早超时的事件）然后处理后删除，
这个场景下难道不是最小化堆是最好的数据结构么？
然后通过搜索得知阿里的Tengine将timer的实现了4-heap（四叉最小堆）。
四叉堆是二叉堆的变种，比二叉堆有更浅的深度和更好的CPU Cache命中率。
Tengine团队声称用最小堆性能提升比较明显。在Zaver中为了简化实现，使用了二叉堆来实现timer的功能。
*/

#ifndef ZV_TIMER_H
#define ZV_TIMER_H

#include "priority_queue.h"
#include "http_request.h"

#define ZV_TIMER_INFINITE -1
#define TIMEOUT_DEFAULT 500     /* ms */

typedef int (*timer_handler_pt)(zv_http_request_t *rq);

typedef struct zv_timer_node_s{
    size_t key;     /* http连接超时时刻 */
    int deleted;    /* if remote client close the socket first, set deleted to 1 */
    timer_handler_pt handler;
    zv_http_request_t *rq;
} zv_timer_node;

int zv_timer_init();
int zv_find_timer();
void zv_handle_expire_timers();

extern zv_pq_t zv_timer;
extern size_t zv_current_msec;

void zv_add_timer(zv_http_request_t *rq, size_t timeout, timer_handler_pt handler);
void zv_del_timer(zv_http_request_t *rq);

#endif
