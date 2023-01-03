#include "lvgl/lvgl.h"

#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_drivers/indev/keyboard.h"

#include "lv_demos/lv_demo.h"

#include <unistd.h>

#include <stdio.h>
#include <windows.h>

//#define DISP_BUF_SIZE (272 * LV_HOR_RES_MAX)

static int tick_thread(void * data);

int main(int argc, char *argv[])
{
    /*LittlevGL init*/
    lv_init();

    monitor_init();

    uint32_t rotated = LV_DISP_ROT_NONE;

    static lv_color_t *buf;
    buf = (lv_color_t*) malloc(LV_HOR_RES_MAX * LV_VER_RES_MAX * sizeof (lv_color_t));

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);


    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb = monitor_flush;
    disp_drv.hor_res    = LV_HOR_RES_MAX;
    disp_drv.ver_res    = LV_VER_RES_MAX;
    disp_drv.rotated    = rotated;
    lv_disp_drv_register(&disp_drv);

    //鼠标输入
    mouse_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);

    //键盘输入
    keyboard_init();
    lv_indev_drv_t sdl_kb_drv;
    lv_indev_drv_init(&sdl_kb_drv);
    sdl_kb_drv.type = LV_INDEV_TYPE_KEYPAD;
    //sdl_kb_drv.read_cb = keyboard_read; //每个gui时间片轮询都会调用此回调函数
    lv_indev_t * sdl_kb_indev = lv_indev_drv_register(&sdl_kb_drv);
    lv_indev_data_t kbDate;

    /*Create a Demo*/
    lv_demo_widgets();

    SDL_CreateThread(tick_thread, "tick", NULL);

    while(1) {

        keyboard_read(&sdl_kb_drv, &kbDate);


        if (kbDate.key != 0) {
            //1-49 2-50 ...
            printf("%d\r\n", kbDate.key);
        }

        lv_task_handler();
        Sleep(5);

#ifdef SDL_APPLE
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
#if USE_MOUSE != 0
            mouse_handler(&event);
#endif
#if USE_KEYBOARD
            keyboard_handler(&event);
#endif
        }
#endif
    }

    return 0;
}

static int tick_thread(void * data) {
    (void)data;

    while(1) {
        SDL_Delay(5);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }
    return 0;
}
