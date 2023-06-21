//------------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <libqu.h>

//------------------------------------------------------------------------------

#define NUM_OBJECTS 24

struct object
{
    qu_color color;
    qu_vec2f position;
    float scale;
    float rotation;
    float speed;
    bool growing;
};

static float prev_time;
static struct object objects[NUM_OBJECTS];
static struct object alpha;
static struct object beta;
static struct object gamma;

//------------------------------------------------------------------------------

static void init_object(struct object *object)
{
    object->color = QU_COLOR(rand() % 256, rand() % 256, rand() % 256);
    object->position.x = (float) (rand() % 512);
    object->position.y = (float) (rand() % 512);
    object->scale = 0.5f + (float) (rand() % 1000) / 1000.f;
    object->speed = 0.5f + (float) (rand() % 1000) / 1000.f;
}

static void update_object(struct object *object, float dt)
{
    object->rotation += 180.f * object->speed * dt;

    if (object->growing) {
        object->scale += object->speed * dt;

        if (object->scale > 1.f) {
            object->growing = false;
        }
    } else {
        object->scale -= object->speed * dt;

        if (object->scale < 0.25f) {
            object->growing = true;
        }
    }
}

static void draw_object(struct object *object)
{
    qu_push_matrix();
    qu_translate(object->position.x, object->position.y);
    qu_rotate(object->rotation);
    qu_scale(object->scale, object->scale);
    qu_draw_rectangle(-32.f, -32.f, 64.f, 64.f, object->color, 0);
    qu_draw_rectangle(-24.f, -24.f, 48.f, 48.f, QU_COLOR(255, 255, 255), 0);
    qu_pop_matrix();
}

static bool loop(void)
{
    float current_time = (float) qu_get_time_highp();
    float dt = current_time - prev_time;
    prev_time = current_time;

    qu_clear(QU_COLOR(0, 0, 0));

    for (int i = 0; i < NUM_OBJECTS; i++) {
        update_object(&objects[i], dt);
        draw_object(&objects[i]);
    }

    qu_present();

    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .title = "[libqu] hello-transform",
        .display_width = 512,
        .display_height = 512,
    });

    srand((unsigned int) (qu_get_time_mediump() * 1000.f));

    for (int i = 0; i < NUM_OBJECTS; i++) {
        init_object(&objects[i]);
    }

    qu_execute(loop);
}
