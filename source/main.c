/*
 * This example is created form the gs_project_template
 * See the LICENSE file for details
 */

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_GUI_IMPL
#include <gs/util/gs_gui.h>

typedef struct app_t 
{
        gs_command_buffer_t cb;
        gs_immediate_draw_t gsi;
        gs_gui_context_t gui;
} app_t;

#define GS_DDT_IMPL
#include "gs_ddt/gs_ddt.h"

static void
echo(int argc, char** argv)
{
        for (int i = 1; i < argc; i++)
                gs_ddt_printf("%s ", argv[i]);
        gs_ddt_printf("\n");
}

static void
spam(int argc, char** argv)
{
        if (argc != 3) goto spam_invalid_command;
        int count  = atoi(argv[2]);
        if (!count) goto spam_invalid_command;
        while (count--) gs_ddt_printf("%s\n", argv[1]);
        return;
spam_invalid_command:
        gs_ddt_printf("[spam]: invalid usage. It should be 'spam word [int count]''\n");
}

static void help(int argc, char** argv);

static int bg;
static void toggle_bg(int argc, char** argv)
{
        gs_ddt_printf("Background turned %s\n", (bg = !bg) ? "on" : "off");
}


gs_ddt_command_t commands[] = {
        {
                .func = echo,
                .name = "echo",
                .desc = "repeat what was entered",
        },
        {
                .func = spam,
                .name = "spam",
                .desc = "send the word arg1, arg2 amount of times",
        },
        {
                .func = help,
                .name = "help",
                .desc = "sends a list of commands",
        },
        {
                .func = toggle_bg,
                .name = "bg",
                .desc = "toggles background",
        },
};

static void
help(int argc, char** argv)
{
        for (int i = 0; i < gs_array_size(commands); i++) {
                if (commands[i].name) gs_ddt_printf("* Command: %s\n", commands[i].name);
                if (commands[i].desc) gs_ddt_printf("- desc: %s\n", commands[i].desc);
        }
}

void app_init() 
{
        app_t* app = gs_user_data(app_t);
        app->cb = gs_command_buffer_new();
        app->gsi = gs_immediate_draw_new(gs_platform_main_window());
        gs_gui_init(&app->gui, gs_platform_main_window());
}

void app_update()
{
        app_t* app = gs_user_data(app_t);
        gs_command_buffer_t* cb = &app->cb;
        gs_immediate_draw_t* gsi = &app->gsi;
        gs_gui_context_t* gui = &app->gui;

        gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
        const float t = gs_platform_elapsed_time() * 0.0001f;

        static int open, autoscroll = 1;
        if (gs_platform_key_pressed(GS_KEYCODE_ESC)) {
                open = !open;
        } else if (gs_platform_key_pressed(GS_KEYCODE_TAB) && open) {
                autoscroll = !autoscroll;
        }

        if (bg) {
                gsi_camera3D(gsi, fbs.x, fbs.y);
                gsi_rotatev(gsi, gs_deg2rad(90.f), GS_ZAXIS); gsi_rotatev(gsi, t, GS_YAXIS);
                gsi_sphere(gsi, 0.f, 0.f, 0.f, 1.f, 50, 150, 200, 50, GS_GRAPHICS_PRIMITIVE_LINES);
        }
        gsi_camera2D(gsi, fbs.x, fbs.y);
        gsi_text(gsi, fbs.x * 0.5f - 198.f, fbs.y * 0.5f, "ESC to open term, TAB toggle autoscroll, help for more", NULL, false, 255, 255, 255, 255);
        gsi_renderpass_submit(gsi, cb, fbs.x, fbs.y, gs_color(10, 10, 10, 255));

        // Render gui
        gs_gui_begin(gui, NULL);

        gs_ddt_update(gui, NULL, open, autoscroll, 0.4f, 0.2f, 0.3f, commands, gs_array_size(commands));

        gs_gui_end(gui);
        gs_gui_render(gui, cb);

        // Submit command buffer for GPU
        gs_graphics_command_buffer_submit(cb);
}

void app_shutdown()
{
        app_t* app = gs_user_data(app_t);
        gs_immediate_draw_free(&app->gsi);
        gs_command_buffer_free(&app->cb);
        gs_gui_free(&app->gui);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
        return (gs_app_desc_t) {
                .user_data = gs_malloc_init(app_t),
                .window_width = 800,
                .window_height = 600,
                .init = app_init,
                .update = app_update,
                .shutdown = app_shutdown
        };
}
