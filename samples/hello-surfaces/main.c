//------------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <libqu.h>

//------------------------------------------------------------------------------

#define NUM_SURFACES            3
#define SURFACE_WIDTH           128.f
#define SURFACE_HEIGHT          128.f

//------------------------------------------------------------------------------

static qu_surface surfaces[NUM_SURFACES];
static float rotation[NUM_SURFACES];
static double pt;

static bool loop(void)
{
    double ct = qu_get_time_highp();
    double dt = ct - pt;
    pt = ct;

    for (int i = 0; i < NUM_SURFACES; i++) {
        rotation[i] += 90.f * dt;
    }

    float x = SURFACE_WIDTH / 2.f;
    float y = SURFACE_HEIGHT / 2.f;

    qu_set_surface(surfaces[0]);
    qu_set_view(x, y, SURFACE_WIDTH, SURFACE_HEIGHT, rotation[0]);
    qu_clear(QU_COLOR(29, 43, 83));
    qu_draw_rectangle(32, 32, 64, 64, QU_COLOR(194, 195, 199), 0);

    qu_set_surface(surfaces[1]);
    qu_set_view(x, y, SURFACE_WIDTH, SURFACE_HEIGHT, rotation[1]);
    qu_clear(QU_COLOR(126, 37, 83));
    qu_draw_rectangle(32, 32, 64, 64, QU_COLOR(194, 195, 199), 0);

    qu_set_surface(surfaces[2]);
    qu_set_view(x, y, SURFACE_WIDTH, SURFACE_HEIGHT, rotation[2]);
    qu_clear(QU_COLOR(0, 135, 81));
    qu_draw_rectangle(32, 32, 64, 64, QU_COLOR(194, 195, 199), 0);

    qu_reset_surface();
    qu_clear(QU_COLOR(32, 32, 32));
    qu_draw_surface(surfaces[0], 32, 192, 128, 128);
    qu_draw_surface(surfaces[1], 192, 192, 128, 128);
    qu_draw_surface(surfaces[2], 352, 192, 128, 128);

    qu_present();

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "[libqu] hello-surfaces",
        .display_width = 512,
        .display_height = 512,
    });

    for (int i = 0; i < NUM_SURFACES; i++) {
        surfaces[i] = qu_create_surface(SURFACE_WIDTH, SURFACE_HEIGHT);

        if (!surfaces[i].id) {
            return -1;
        }
    }

    rotation[0] = 0.f;
    rotation[1] = 30.f;
    rotation[2] = 60.f;

    pt = 0.0;

    qu_execute(loop);
}

//------------------------------------------------------------------------------
