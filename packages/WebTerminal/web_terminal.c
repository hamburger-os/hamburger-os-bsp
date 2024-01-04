/*
 * This file is part of the Web Terminal Library.
 *
 * Copyright (c) 2017, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Initialize function and other general function.
 * Created on: 2017-08-20
 */

#include <stdbool.h>
#include <shell.h>
#include <finsh.h>
#include <rtdevice.h>
#include <mongoose.h>
#include "web_terminal.h"

#ifndef WEB_RECV_BUF_SIZE
#define WEB_RECV_BUF_SIZE              1024
#endif
#ifndef WEB_SEND_BUF_SIZE
#define WEB_SEND_BUF_SIZE              1024
#endif
#ifndef WEB_TERMINAL_DEV_NAM
#define WEB_TERMINAL_DEV_NAM           "web_term"
#endif
#ifndef WEB_TERMINAL_THREAD_NAM
#define WEB_TERMINAL_THREAD_NAM        "web_terminal"
#endif

/**
 * web_terminal �豸
 */
typedef struct {
    struct rt_device device;

    struct rt_ringbuffer recv_buf;
    struct rt_ringbuffer send_buf;

    rt_mutex_t recv_buf_lock;
    /* ������ WebSocket �ͻ��˵�������Ϣ */
    struct mg_connection *ws_clinet;
    /* �ͻ������ӷ����� */
    rt_mutex_t client_nc_lock;
    /* console �� finsh �豸�ı�����Ϣ */
    const char *console_dev_name_bak;
    const char *finsh_dev_name_bak;
    rt_uint8_t echo_mode_bak;
    /* ���б�־ */
    bool is_running;
} web_terminal, *web_terminal_t;

/* ��ʼ���ɹ���־ */
static bool init_ok = false;
/* Web Terminal �豸����ʵ�� */
static web_terminal_t terminal = NULL;

/**
 * RT-Thread �豸�ӿ�
 */
static rt_err_t device_init(rt_device_t dev) {
    return RT_EOK;
}

static rt_err_t device_open(rt_device_t dev, rt_uint16_t oflag) {
    return RT_EOK;
}

static rt_err_t device_close(rt_device_t dev) {
    return RT_EOK;
}

static rt_size_t device_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size) {
    rt_size_t result;
    /* �� WebSokcet ���������ջ�������ȡ�����ݣ������ݸ� Terminal */
    rt_mutex_take(terminal->recv_buf_lock, RT_WAITING_FOREVER);
    result = rt_ringbuffer_get(&(terminal->recv_buf), buffer, size);
    rt_mutex_release(terminal->recv_buf_lock);

    return result;
}

static rt_size_t device_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size) {
    if (terminal->ws_clinet) {
        /* �������ӵ� WebSocket �ͻ��˷��� Terminal ���͹��������� */
        rt_mutex_take(terminal->client_nc_lock, RT_WAITING_FOREVER);
        mg_send_websocket_frame(terminal->ws_clinet, WEBSOCKET_OP_TEXT, buffer, size);
        rt_mutex_release(terminal->client_nc_lock);
        return size;
    } else {
        return 0;
    }
}

static rt_err_t device_control(rt_device_t dev, rt_uint8_t cmd, void *args) {
    return RT_EOK;
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    switch (ev) {
    case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: {
        /* ���������� 1 ���ͻ��� */
        if (terminal->ws_clinet) {
            /* �Ͽ������� */
            nc->flags |= MG_F_CLOSE_IMMEDIATELY;
        }
        break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
        char addr[32];

        mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
        rt_kprintf("Web Terminal: %s ������\n", addr);

        terminal->ws_clinet = nc;
        /* ���ݵ�ǰ�Ļ���ģʽ */
        terminal->echo_mode_bak = finsh_get_echo();
        /* ���ݵ�ǰ console �� finsh �豸 */
        terminal->console_dev_name_bak = rt_console_get_device()->parent.name;
        terminal->finsh_dev_name_bak = finsh_get_device();
        /* ���� console �� finsh �豸Ϊ Web Termianl �豸 */
        rt_console_set_device(WEB_TERMINAL_DEV_NAM);
        finsh_set_device(WEB_TERMINAL_DEV_NAM);
        /* ��������ģʽ */
        finsh_set_echo(1);
        break;
    }
    case MG_EV_WEBSOCKET_FRAME: {
        struct websocket_message *wm = (struct websocket_message *) ev_data;
        /* ���� WebSokcet ���������յ����ݴ�����ջ����� */
        rt_mutex_take(terminal->recv_buf_lock, RT_WAITING_FOREVER);
        rt_ringbuffer_put(&(terminal->recv_buf), wm->data, wm->size);
        rt_mutex_release(terminal->recv_buf_lock);
        /* ִ�� Web Terminal �豸�Ľ��ջص� */
        if (terminal->device.rx_indicate && wm->size) {
            terminal->device.rx_indicate(&terminal->device, wm->size);
        }
        break;
    }
    case MG_EV_CLOSE: {
        if (nc == terminal->ws_clinet) {
            /* ��ԭ finsh ֮ǰ�Ļ���ģʽ */
            finsh_set_echo(terminal->echo_mode_bak);
            /* ��ԭ console �� finsh �豸 */
            rt_console_set_device(terminal->console_dev_name_bak);
            finsh_set_device(terminal->finsh_dev_name_bak);
            terminal->ws_clinet = NULL;
            rt_kprintf("Web Terminal: �ѶϿ�\n");
        }
        break;
    }
    }
}

static void web_terminal_thread(void* parameter) {
    struct mg_mgr mgr;
    struct mg_connection *nc;

    /* ���� WebSocket ������ */
    mg_mgr_init(&mgr, NULL);

    nc = mg_bind(&mgr, WEB_TERMINAL_PORT, ev_handler);

    if (nc) {
        mg_set_protocol_http_websocket(nc);
        rt_kprintf("Web Terminal: ������ɣ��ȴ����ӡ���\n");
        while (terminal->is_running) {
            rt_mutex_take(terminal->client_nc_lock, RT_WAITING_FOREVER);
            mg_mgr_poll(&mgr, 10);
            rt_mutex_release(terminal->client_nc_lock);
            rt_thread_delay(rt_tick_from_millisecond(10));
        }
    } else {
        rt_kprintf("Web Terminal: �󶨶˿�(%s)ʧ�ܣ������ԡ���\n", WEB_TERMINAL_PORT);
        terminal->is_running = false;
    }
    mg_mgr_free(&mgr);
}

void web_terminal_init(void) {
    if (init_ok) {
        rt_kprintf("Web Terminal: �����ظ���ʼ����\n");
        return;
    }

    terminal = rt_calloc(1, sizeof(web_terminal));
    if (terminal == NULL) {
        rt_kprintf("Web Terminal: �ڴ治�㣡\n");
        return;
    }

    rt_uint8_t *ptr = rt_malloc(WEB_RECV_BUF_SIZE);
    if (ptr) {
        rt_ringbuffer_init(&terminal->recv_buf, ptr, WEB_RECV_BUF_SIZE);
    } else {
        rt_free(terminal);
        rt_kprintf("Web Terminal: �ڴ治�㣡\n");
        return;
    }

    terminal->recv_buf_lock = rt_mutex_create("web_term_recv", RT_IPC_FLAG_FIFO);
    if (terminal->recv_buf_lock == NULL) {
        rt_free(terminal);
        rt_free(ptr);
        rt_kprintf("Web Terminal: �ڴ治�㣡\n");
        return;
    }

    terminal->client_nc_lock = rt_mutex_create("web_term_conn", RT_IPC_FLAG_FIFO);
    if (terminal->client_nc_lock == NULL) {
        rt_mutex_delete(terminal->recv_buf_lock);
        rt_free(terminal);
        rt_free(ptr);
        rt_kprintf("Web Terminal: �ڴ治�㣡\n");
        return;
    }

    /* ע�� Web Terminal �豸 */
    terminal->device.type      = RT_Device_Class_Char;
    terminal->device.init      = device_init;
    terminal->device.open      = device_open;
    terminal->device.close     = device_close;
    terminal->device.read      = device_read;
    terminal->device.write     = device_write;
    terminal->device.control   = device_control;
    terminal->device.user_data = RT_NULL;
    rt_device_register(&terminal->device, WEB_TERMINAL_DEV_NAM, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM);

    init_ok = true;

    rt_kprintf("Web Terminal: V%s ��ʼ�����\n", WT_SW_VERSION);
}

void web_terminal_start(void) {
    if (init_ok) {
        if (terminal->is_running) {
            rt_kprintf("Web Terminal: ������\n");
            return;
        }

        rt_thread_t thread = rt_thread_create(WEB_TERMINAL_THREAD_NAM, web_terminal_thread, NULL, 4096, 19, 25);
        if (thread) {
            terminal->is_running = true;
            rt_thread_startup(thread);
        }
    } else {
        rt_kprintf("Web Terminal: δ��ʼ����\n");
    }
}

void web_terminal_stop(void) {
    if (init_ok) {
        if (terminal->is_running) {
            /* �����߳� */
            terminal->is_running = false;
            /* �ȴ��߳��Լ����н�����1S ��ʱ */
            for (uint8_t i = 0; i < 10; i++) {
                if ((rt_thread_find(WEB_TERMINAL_THREAD_NAM) == NULL)) {
                    rt_kprintf("Web Terminal: ��ֹͣ\n");
                    break;
                } else {
                    rt_thread_delay(rt_tick_from_millisecond(100));
                }
            }
        } else {
            rt_kprintf("Web Terminal: δ����\n");
        }
    } else {
        rt_kprintf("Web Terminal: δ��ʼ����\n");
    }
}

static void web_term(uint8_t argc, char **argv) {
    if (argc >= 2) {
        if (!strcmp(argv[1], "init")) {
            web_terminal_init();
        } else if (!strcmp(argv[1], "start")) {
            web_terminal_start();
        } else if (!strcmp(argv[1], "stop")) {
            web_terminal_stop();
        } else {
            rt_kprintf("�����룺 `web_term <init|start|stop>`\n");
        }
    } else {
        rt_kprintf("�����룺`web_term <init|start|stop>`\n");
    }
}
MSH_CMD_EXPORT(web_term, Web Terminal 'web_term <init|start|stop>');
