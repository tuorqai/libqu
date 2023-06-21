
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <libqu.h>

//------------------------------------------------------------------------------
// Constants

#define TICKS_PER_SECOND        40
#define TICK_DURATION           (1.0 / TICKS_PER_SECOND)

#define STATE_NORMAL            0
#define STATE_TITLE             1
#define STATE_GAMEOVER          2

#define INPUT_UP                0x01
#define INPUT_DOWN              0x02
#define INPUT_LEFT              0x04
#define INPUT_RIGHT             0x08
#define INPUT_ATTACK            0x10

#define ANIMATION_IDLE          0
#define ANIMATION_RUN           1
#define ANIMATION_ATTACK        2
#define ANIMATION_DYING         3
#define ANIMATION_DEAD          4

#define TOTAL_ENEMIES           8
#define TOTAL_BULLETS           4
#define TOTAL_PARTICLES         256
#define TOTAL_POPUPS            2

#define TEXTURE_CHARACTER       0
#define TEXTURE_ZOMBIESKEL      1
#define TEXTURE_BGCITY          2
#define TOTAL_TEXTURES          3

#define SOUND_SHOT1             0
#define SOUND_SHOT2             1
#define SOUND_DAMAGE1           2
#define SOUND_DAMAGE2           3
#define SOUND_DAMAGE3           4
#define SOUND_DAMAGE4           5
#define SOUND_DAMAGE5           6
#define SOUND_DAMAGE6           7
#define SOUND_DAMAGE7           8
#define SOUND_DAMAGE8           9
#define SOUND_DAMAGE9           10
#define SOUND_DAMAGE10          11
#define SOUND_FANFARE           12
#define SOUND_NEGATIVE          13
#define TOTAL_SOUNDS            14

#define MUSIC_29                0
#define MUSIC_BALDA             1
#define TOTAL_TRACKS            2

#define FONT_PIXEL              0
#define FONT_PIXEL_22           1
#define TOTAL_FONTS             2

//------------------------------------------------------------------------------
// Typedefs

struct player
{
    struct scene *scene;

    qu_vec2f position;
    qu_vec2f velocity;
    int direction;

    unsigned int input;
    float attack_time;

    int animation_sequence;
    int animation_frame;
    int animation_length;
    bool animation_loop;
    float animation_time;
    float animation_rate;

    float health;
};

struct enemy
{
    struct scene *scene;
    bool active;

    qu_vec2f position;
    qu_vec2f velocity;
    int direction;

    int animation_frame;
    float animation_time;

    float health;
};

struct bullet
{
    struct scene *scene;
    bool active;

    qu_vec2f position;
    qu_vec2f velocity;
};

struct particle
{
    bool active;

    qu_vec2f position;
    qu_vec2f velocity;
    qu_vec2f size;
    qu_color outline;
    qu_color fill;

    float start_time;
};

struct popup
{
    struct scene *scene;
    bool active;

    qu_vec2f position;

    qu_color main;
    qu_color shadow;

    float start_time;
    float duration;

    char buffer[32];
};

struct scene
{
    struct game *game;

    struct player player;
    struct enemy enemies[TOTAL_ENEMIES];
    struct bullet bullets[TOTAL_BULLETS];
    struct particle particles[TOTAL_PARTICLES];
    struct popup popups[TOTAL_POPUPS];

    float enemy_spawn_time;
    int current_bullet;
    int current_particle;
    int current_popup;
};

struct resources
{
    qu_texture textures[TOTAL_TEXTURES];
    qu_font fonts[TOTAL_FONTS];
    qu_sound sounds[TOTAL_SOUNDS];
    qu_music tracks[TOTAL_TRACKS];
};

struct game
{
    struct resources resources;
    struct scene scene;

    double frame_time;
    double lag;

    int state;
    int score;

    float state_time;
    int next_state;

    qu_stream ambient;
};

//------------------------------------------------------------------------------
// Function declarations and macros

void game_init(struct game *game);
void game_loop(struct game *game);
void game_switch_state(struct game *game, int next_state, float after);

void resources_init(struct resources *resources);

void scene_init(struct scene *scene, struct game *game);
void scene_update(struct scene *scene);
void scene_draw(struct scene *scene, double lag_offset);
void scene_spawn_bullet(struct scene *scene, float x, float y, float dx, float dy);
void scene_spawn_particle(struct scene *scene, float x, float y, float dx, float dy,
    float w, float h, qu_color outline, qu_color fill);
void scene_spawn_popup(struct scene *scene, float x, float y, float duration, int number);
void scene_emit_blood(struct scene *scene, float x, float y, float dx, float dy);
void scene_emit_splatter(struct scene *scene, float x, float y);

void player_init(struct player *player, struct scene *scene);
void player_update(struct player *player);
void player_draw(struct player *player, double lag_offset);
void player_damage(struct player *player, float amount, int direction);

void enemy_init(struct enemy *enemy, struct scene *scene);
void enemy_update(struct enemy *enemy);
void enemy_draw(struct enemy *enemy, double lag_offset);
void enemy_damage(struct enemy *enemy, float amount);

void bullet_init(struct bullet *bullet, struct scene *scene);
void bullet_update(struct bullet *bullet);
void bullet_draw(struct bullet *bullet, double lag_offset);

void particle_init(struct particle *particle);
void particle_update(struct particle *particle);
void particle_draw(struct particle *particle, double lag_offset);

void popup_init(struct popup *popup, struct scene *scene);
void popup_update(struct popup *popup);
void popup_draw(struct popup *popup);

void ui_draw(struct game *game);

#define UI_DRAW_TEXT(Font, X, Y, Main, Shadow, ...) \
    do { \
        qu_draw_text_fmt(Font, (X) + 2.f, (Y) + 2.f, Shadow, __VA_ARGS__); \
        qu_draw_text_fmt(Font, X, Y, Main, __VA_ARGS__); \
    } while (0);

//------------------------------------------------------------------------------
// Utility functions

static float vlen(qu_vec2f vec)
{
    return sqrtf((vec.x * vec.x) + (vec.y * vec.y));
}

//------------------------------------------------------------------------------
// Game

void game_init(struct game *game)
{
    *game = (struct game) {
        .state = STATE_TITLE,
        .next_state = -1,
    };

    resources_init(&game->resources);
    scene_init(&game->scene, game);
}

void game_loop(struct game *game)
{
    double current_time = qu_get_time_highp();
    double elapsed_time = current_time - game->frame_time;

    game->frame_time = current_time;
    game->lag += elapsed_time;

    while (game->lag >= TICK_DURATION) {
        if (game->next_state >= 0) {
            if (game->state_time < qu_get_time_mediump()) {
                game->state = game->next_state;
                game->next_state = -1;

                qu_stop_stream(game->ambient);

                if (game->state != STATE_GAMEOVER) {
                    scene_init(&game->scene, game);
                    game->score = 0;
                }

                switch (game->state) {
                case STATE_NORMAL:
                    game->ambient = qu_loop_music(game->resources.tracks[MUSIC_BALDA]);
                    qu_play_sound(game->resources.sounds[SOUND_FANFARE]);
                    break;
                case STATE_GAMEOVER:
                    qu_play_sound(game->resources.sounds[SOUND_NEGATIVE]);
                    break;
                default:
                    break;
                }
            }
        }

        if (game->state == STATE_TITLE) {
            if (qu_is_key_pressed(QU_KEY_SPACE)) {
                game_switch_state(game, STATE_NORMAL, 0.25f);
            }
        } else {
            if (qu_is_key_pressed(QU_KEY_ESCAPE)) {
                game_switch_state(game, STATE_TITLE, 0.25f);
            }

            scene_update(&game->scene);
        }

        game->lag -= TICK_DURATION;
    }

    double lag_offset = game->lag / TICK_DURATION;

    qu_clear(0xFF000000);
    scene_draw(&game->scene, lag_offset);
    ui_draw(game);
    qu_present();
}

void game_switch_state(struct game *game, int next_state, float after)
{
    if (game->next_state != -1) {
        return;
    }

    game->next_state = next_state;
    game->state_time = qu_get_time_mediump() + after;
}

//------------------------------------------------------------------------------
// Resources

void resources_init(struct resources *resources)
{
    char const *texture_paths[TOTAL_TEXTURES] = {
        [TEXTURE_CHARACTER] = "assets/character.png",
        [TEXTURE_ZOMBIESKEL] = "assets/zombie_n_skeleton2.png",
        [TEXTURE_BGCITY] = "assets/bgCity2.png",
    };

    char const *font_paths[TOTAL_FONTS] = {
        [FONT_PIXEL] = "assets/LanaPixel.ttf",
        [FONT_PIXEL_22] = "assets/LanaPixel.ttf",
    };

    float font_sizes[TOTAL_FONTS] = {
        [FONT_PIXEL] = 33.f,
        [FONT_PIXEL_22] = 22.f,
    };

    char const *sound_paths[TOTAL_SOUNDS] = {
        [SOUND_SHOT1] = "assets/shot1.wav",
        [SOUND_SHOT2] = "assets/shot2.wav",
        [SOUND_DAMAGE1] = "assets/sfx_damage_hit1.wav",
        [SOUND_DAMAGE2] = "assets/sfx_damage_hit2.wav",
        [SOUND_DAMAGE3] = "assets/sfx_damage_hit3.wav",
        [SOUND_DAMAGE4] = "assets/sfx_damage_hit4.wav",
        [SOUND_DAMAGE5] = "assets/sfx_damage_hit5.wav",
        [SOUND_DAMAGE6] = "assets/sfx_damage_hit6.wav",
        [SOUND_DAMAGE7] = "assets/sfx_damage_hit7.wav",
        [SOUND_DAMAGE8] = "assets/sfx_damage_hit8.wav",
        [SOUND_DAMAGE9] = "assets/sfx_damage_hit9.wav",
        [SOUND_DAMAGE10] = "assets/sfx_damage_hit10.wav",
        [SOUND_FANFARE] = "assets/fanfare.wav",
        [SOUND_NEGATIVE] = "assets/sfx_sounds_negative1.wav",
    };

    char const *music_paths[TOTAL_TRACKS] = {
        [MUSIC_29] = "assets/29.wav",
        [MUSIC_BALDA] = "assets/balda.wav",
    };

    for (int i = 0; i < TOTAL_TEXTURES; i++) {
        resources->textures[i] = qu_load_texture(texture_paths[i]);

        if (!resources->textures[i].id) {
            fprintf(stderr, "Failed to load texture %s\n", texture_paths[i]);
            abort();
        }
    }

    for (int i = 0; i < TOTAL_FONTS; i++) {
        resources->fonts[i] = qu_load_font(font_paths[i], font_sizes[i]);

        if (!resources->fonts[i].id) {
            fprintf(stderr, "Failed to load font %s (%.2fpt)\n", font_paths[i], font_sizes[i]);
            abort();
        }
    }

    for (int i = 0; i < TOTAL_SOUNDS; i++) {
        resources->sounds[i] = qu_load_sound(sound_paths[i]);

        if (!resources->sounds[i].id) {
            fprintf(stderr, "Failed to load sound %s\n", sound_paths[i]);
            abort();
        }
    }

    for (int i = 0; i < TOTAL_TRACKS; i++) {
        resources->tracks[i] = qu_open_music(music_paths[i]);
    }
}

//------------------------------------------------------------------------------
// Scene

void scene_init(struct scene *scene, struct game *game)
{
    *scene = (struct scene) {
        .game = game,
        .enemy_spawn_time = qu_get_time_mediump() + 5.f,
    };

    player_init(&scene->player, scene);
}

void scene_update(struct scene *scene)
{
    player_update(&scene->player);

    for (int i = 0; i < TOTAL_ENEMIES; i++) {
        if (scene->enemies[i].active) {
            enemy_update(&scene->enemies[i]);
        } else {
            if (scene->enemy_spawn_time < qu_get_time_mediump()) {
                enemy_init(&scene->enemies[i], scene);

                if (rand() % 2 == 0) {
                    scene->enemies[i].position.x = -32.f;
                } else {
                    scene->enemies[i].position.x = 752.f;
                }

                scene->enemies[i].position.y = 250.f + (rand() % 200);

                scene->enemy_spawn_time = qu_get_time_mediump() + 1.f + (rand() % 3);
            }
        }
    }

    for (int i = 0; i < TOTAL_BULLETS; i++) {
        if (scene->bullets[i].active) {
            bullet_update(&scene->bullets[i]);
        }
    }

    for (int i = 0; i < TOTAL_PARTICLES; i++) {
        if (scene->particles[i].active) {
            particle_update(&scene->particles[i]);
        }
    }

    for (int i = 0; i < TOTAL_POPUPS; i++) {
        if (scene->popups[i].active) {
            popup_update(&scene->popups[i]);
        }
    }
}

void scene_draw(struct scene *scene, double lag_offset)
{
    qu_texture bg = scene->game->resources.textures[TEXTURE_BGCITY];
    qu_draw_texture(bg, 0.f, 0.f, 720.f, 480.f);

    for (int i = 0; i < TOTAL_ENEMIES; i++) {
        if (scene->enemies[i].active) {
            enemy_draw(&scene->enemies[i], lag_offset);
        }
    }

    for (int i = 0; i < TOTAL_BULLETS; i++) {
        if (scene->bullets[i].active) {
            bullet_draw(&scene->bullets[i], lag_offset);
        }
    }

    player_draw(&scene->player, lag_offset);

    for (int i = 0; i < TOTAL_PARTICLES; i++) {
        if (scene->particles[i].active) {
            particle_draw(&scene->particles[i], lag_offset);
        }
    }

    for (int i = 0; i < TOTAL_POPUPS; i++) {
        if (scene->popups[i].active) {
            popup_draw(&scene->popups[i]);
        }
    }
}

void scene_spawn_bullet(struct scene *scene, float x, float y, float dx, float dy)
{
    struct bullet *bullet = &scene->bullets[scene->current_bullet];

    bullet_init(bullet, scene);

    bullet->position.x = x;
    bullet->position.y = y;
    bullet->velocity.x = dx;
    bullet->velocity.y = dy;

    scene->current_bullet = (scene->current_bullet + 1) % TOTAL_BULLETS;
}

void scene_spawn_particle(struct scene *scene, float x, float y, float dx, float dy,
    float w, float h, qu_color outline, qu_color fill)
{
    struct particle *particle = &scene->particles[scene->current_particle];

    particle_init(particle);

    particle->position.x = x;
    particle->position.y = y;
    particle->velocity.x = dx;
    particle->velocity.y = dy;
    particle->size.x = w;
    particle->size.y = h;
    particle->outline = outline;
    particle->fill = fill;

    scene->current_particle = (scene->current_particle + 1) % TOTAL_PARTICLES;
}

void scene_spawn_popup(struct scene *scene, float x, float y, float duration,
    int number)
{
    struct popup *popup = &scene->popups[scene->current_popup];

    popup_init(popup, scene);

    popup->position.x = x;
    popup->position.y = y;
    popup->duration = duration;
    snprintf(popup->buffer, sizeof(popup->buffer), "%d", number);

    scene->current_popup = (scene->current_popup + 1) % TOTAL_POPUPS;
}

void scene_emit_blood(struct scene *scene, float x, float y, float dx, float dy)
{
    for (int i = 0; i < 32; i++) {
        float a = rand() % 24 - 12;
        float b = rand() % 24 - 12;

        scene_spawn_particle(scene, x + a, y + b, dx + b, dy + a, 4.f, 4.f,
            0, QU_COLOR(200 + rand() % 50, 0, 0));
    }
}

void scene_emit_splatter(struct scene *scene, float x, float y)
{
    for (int i = 0; i < 64; i++) {
        float a = rand() % 24 - 12;
        float b = rand() % 24 - 12;
        float c = 4.f + (rand() % 12);

        float dx = (rand() % 4) - 2.f;
        float dy = -(rand() % 16);

        scene_spawn_particle(scene, x + a, y + b, dx, dy, c, c,
            0, QU_COLOR(200 + rand() % 50, 0, 0));
    }
}

//------------------------------------------------------------------------------
// Player

void player_init(struct player *player, struct scene *scene)
{
    *player = (struct player) {
        .scene = scene,
        .position.x = 360.f,
        .position.y = 400.f,
        .direction = -1,
        .attack_time = qu_get_time_mediump() + 0.5f,
        .health = 1.f,
    };
}

void player_set_animation(struct player *player, int sequence)
{
    player->animation_sequence = sequence;
    player->animation_frame = 0;
    player->animation_time = 0.f;

    switch (sequence) {
    case ANIMATION_IDLE:
    case ANIMATION_ATTACK:
        player->animation_length = 5;
        break;
    case ANIMATION_RUN:
    case ANIMATION_DYING:
        player->animation_length = 8;
        break;
    default:
        player->animation_length = 1;
        break;
    }

    switch (sequence) {
    case ANIMATION_IDLE:
    case ANIMATION_RUN:
    case ANIMATION_DEAD:
        player->animation_loop = true;
        break;
    default:
        player->animation_loop = false;
        break;
    }

    switch (sequence) {
    case ANIMATION_IDLE:
        player->animation_rate = 1.f / 3;
        break;
    default:
        player->animation_rate = 0.1f;
        break;
    }
}

void player_update(struct player *player)
{
    if (player->health <= 0.f) {
        return;
    }

    player->input = 0;

    if (qu_is_key_pressed(QU_KEY_W)) {
        player->input |= INPUT_UP;
    }

    if (qu_is_key_pressed(QU_KEY_S)) {
        player->input |= INPUT_DOWN;
    }

    if (qu_is_key_pressed(QU_KEY_A)) {
        player->input |= INPUT_LEFT;

        if (player->animation_sequence != ANIMATION_ATTACK) {
            player->direction = -1;
        }
    }

    if (qu_is_key_pressed(QU_KEY_D)) {
        player->input |= INPUT_RIGHT;

        if (player->animation_sequence != ANIMATION_ATTACK) {
            player->direction = +1;
        }
    }

    if (qu_is_key_pressed(QU_KEY_SPACE)) {
        player->input |= INPUT_ATTACK;
    }

    player->velocity.x /= 2.f;
    player->velocity.y /= 2.f;

    if (player->animation_sequence != ANIMATION_ATTACK) {
        if (player->input & INPUT_UP) {
            player->velocity.y -= 4.f;
        }

        if (player->input & INPUT_DOWN) {
            player->velocity.y += 4.f;
        }

        if (player->input & INPUT_LEFT) {
            player->velocity.x -= 4.f;
        }

        if (player->input & INPUT_RIGHT) {
            player->velocity.x += 4.f;
        }

        if (vlen(player->velocity) < 0.01f) {
            if (player->animation_sequence == ANIMATION_RUN) {
                player_set_animation(player, ANIMATION_IDLE);
            }
        } else {
            if (player->animation_sequence != ANIMATION_RUN) {
                player_set_animation(player, ANIMATION_RUN);
            }
        }
    }

    player->position.x += player->velocity.x;
    player->position.y += player->velocity.y;

    if (player->position.y < 260.f) {
        player->position.y = 260.f;
        player->velocity.y = 0.f;
    }

    if (player->position.y > 480.f) {
        player->position.y = 480.f;
        player->velocity.y = 0.f;
    }

    if (player->attack_time < qu_get_time_mediump() && (player->input & INPUT_ATTACK)) {
        player_set_animation(player, ANIMATION_ATTACK);
        player->attack_time = qu_get_time_mediump() + 0.22f;

        float x = player->position.x + 12.f * 3 * player->direction;
        float y = player->position.y - 24.f * 3;
        float dx = 24.f * player->direction;
        float dy = -4.f + (rand() % 8);

        scene_spawn_bullet(player->scene, x, y, dx, dy);

        struct resources *resources = &player->scene->game->resources;
        qu_play_sound(resources->sounds[SOUND_SHOT1 + (rand() % 2)]);
    }
}

void player_draw(struct player *player, double lag_offset)
{
    struct game *game = player->scene->game;
    qu_texture texture = game->resources.textures[TEXTURE_CHARACTER];

    float w = 48.f;
    float h = 48.f;
    float sw = w * 3;
    float sh = h * 3;

    float x = player->position.x + player->velocity.x * lag_offset - sw / 2.f;
    float y = player->position.y + player->velocity.y * lag_offset - sh + 24.f;

    float s = player->animation_frame * 48.f;
    float t = 0.f;

    if (player->animation_sequence == ANIMATION_DEAD) {
        s = 336.f;
    }

    switch (player->animation_sequence) {
    case ANIMATION_RUN:
        t = 96.f;
        break;
    case ANIMATION_ATTACK:
        t = 192.f;
        break;
    case ANIMATION_DYING:
    case ANIMATION_DEAD:
        t = 288.f;
    default:
        break;
    }

    if (player->direction > 0) {
        t += 48.f;
    }

    qu_draw_subtexture(texture, x, y, sw, sh, s, t, w, h);

    if (player->animation_time < qu_get_time_mediump()) {
        player->animation_frame++;

        if (player->animation_frame >= player->animation_length) {
            if (player->animation_loop) {
                player->animation_frame = 0;
            } else {
                if (player->animation_sequence == ANIMATION_DYING) {
                    player_set_animation(player, ANIMATION_DEAD);
                } else {
                    player_set_animation(player, ANIMATION_IDLE);
                }
            }
        }

        player->animation_time = qu_get_time_mediump() + player->animation_rate;
    }
}

void player_damage(struct player *player, float amount, int direction)
{
    player->health -= amount;
    player->velocity.x += amount * 1000.f * direction;

    if (player->health <= 0.f) {
        player_set_animation(player, ANIMATION_DYING);

        player->velocity.x = 0.f;
        player->velocity.y = 0.f;

        player->health = 0.f;
        game_switch_state(player->scene->game, STATE_GAMEOVER, 1.5f);
    }
}

//------------------------------------------------------------------------------
// Enemy

void enemy_init(struct enemy *enemy, struct scene *scene)
{
    *enemy = (struct enemy) {
        .scene = scene,
        .active = true,
        .direction = -1,
        .health = 1.f,
    };
}

void enemy_update(struct enemy *enemy)
{
    if (enemy->health <= 0.f) {
        float x = enemy->position.x;
        float y = enemy->position.y - 24.f;

        enemy->scene->game->score += 1000;

        scene_spawn_popup(enemy->scene, x, y, 1.5f, 1000);
        scene_emit_splatter(enemy->scene, x, y);

        enemy->active = false;
        return;
    }

    struct scene *scene = enemy->scene;
    struct player *player = &scene->player;
    struct bullet *bullets = scene->bullets;

    float x_delta = player->position.x - enemy->position.x;
    float y_delta = player->position.y - enemy->position.y;
    int collision = 0;

    if (x_delta < -48.f) {
        enemy->velocity.x = -4.f;
        enemy->direction = -1;
    } else if (x_delta > 48.f) {
        enemy->velocity.x = 4.f;
        enemy->direction = +1;
    } else {
        enemy->velocity.x = 0.f;
        collision |= 0x01;
    }

    if (y_delta < -32.f) {
        enemy->velocity.y = -2.f;
    } else if (y_delta > 32.f) {
        enemy->velocity.y = 2.f;
    } else {
        enemy->velocity.y = 0.f;
        collision |= 0x02;
    }

    enemy->position.x += enemy->velocity.x;
    enemy->position.y += enemy->velocity.y;

    if ((collision & 0x03) == 0x03 && player->health > 0.f) {
        float x = enemy->position.x + 32.f * enemy->direction;
        float y = enemy->position.y - 32.f;

        float dx = 7.f * enemy->direction;
        float dy = -10.f;

        scene_emit_blood(scene, x, y, dx, dy);
        player_damage(player, 0.1f, enemy->direction);

        struct resources *resources = &enemy->scene->game->resources;
        qu_play_sound(resources->sounds[SOUND_DAMAGE1 + (rand() % 10)]);
    }
}

void enemy_draw(struct enemy *enemy, double lag_offset)
{
    struct game *game = enemy->scene->game;
    qu_texture texture = game->resources.textures[TEXTURE_ZOMBIESKEL];

    float w = 32.f;
    float h = 48.f;
    float sw = w * 2;
    float sh = h * 2;

    float x = enemy->position.x + enemy->velocity.x * lag_offset - sw / 2.f;
    float y = enemy->position.y + enemy->velocity.y * lag_offset - sh;

    float s = 32.f * enemy->animation_frame;
    float t = (enemy->direction < 0) ? 80.f : 144.f;

    qu_draw_subtexture(texture, x, y, sw, sh, s, t, w, h);

    if (enemy->animation_time < qu_get_time_mediump()) {
        enemy->animation_frame++;

        if (enemy->animation_frame >= 3) {
            enemy->animation_frame = 0;
        }

        enemy->animation_time = qu_get_time_mediump() + 0.1f;
    }
}

void enemy_damage(struct enemy *enemy, float amount)
{
    enemy->health -= amount;
}

//------------------------------------------------------------------------------
// Bullet

void bullet_init(struct bullet *bullet, struct scene *scene)
{
    *bullet = (struct bullet) {
        .scene = scene,
        .active = true,
    };
}

void bullet_update(struct bullet *bullet)
{
    bullet->position.x += bullet->velocity.x;
    bullet->position.y += bullet->velocity.y;

    if (bullet->position.x < -16.f || bullet->position.x > 736.f) {
        bullet->active = false;
    }

    struct enemy *enemies = bullet->scene->enemies;

    for (int i = 0; i < TOTAL_ENEMIES; i++) {
        if (!enemies[i].active) {
            continue;
        }

        float l = enemies[i].position.x - 32.f * 2;
        float r = enemies[i].position.x + 32.f * 2;
        float t = enemies[i].position.y - 48.f * 2;
        float b = enemies[i].position.y;

        float x = bullet->position.x;
        float y = bullet->position.y;

        if (x < l || x > r || y < t || y > b) {
            continue;
        }

        float d = (enemies[i].position.y - y) / 96.f;

        enemy_damage(&enemies[i], 0.75f * d);
        scene_emit_blood(bullet->scene, x, y, bullet->velocity.x / 2.f, 0.f);

        struct resources *resources = &bullet->scene->game->resources;
        qu_play_sound(resources->sounds[SOUND_DAMAGE1 + (rand() % 10)]);
    
        int bonus = 10 * (int) (50 * d);

        bullet->scene->game->score += bonus;
        scene_spawn_popup(bullet->scene, x, y, 0.75f, bonus);

        bullet->active = false;
    }
}

void bullet_draw(struct bullet *bullet, double lag_offset)
{
    float x = bullet->position.x + bullet->velocity.x * lag_offset;
    float y = bullet->position.y + bullet->velocity.y * lag_offset;

    qu_draw_rectangle(x, y, 8.f, 4.f, 0, 0xFFFFFF00);
}

//------------------------------------------------------------------------------
// Particles

void particle_init(struct particle *particle)
{
    *particle = (struct particle) {
        .active = true,
        .start_time = qu_get_time_mediump(),
    };
}

void particle_update(struct particle *particle)
{
    particle->velocity.y += 1.f;

    particle->position.x += particle->velocity.x;
    particle->position.y += particle->velocity.y;

    if (particle->position.x < -16.f || particle->position.x > 736.f) {
        particle->active = false;
    }

    if (particle->position.y < -16.f || particle->position.y > 496.f) {
        particle->active = false;
    }

    particle->size.x -= 0.33f;
    particle->size.y -= 0.33f;

    if (particle->size.x <= 0.f || particle->size.y <= 0.f) {
        particle->active = false;
    }
}

void particle_draw(struct particle *particle, double lag_offset)
{
    float x = particle->position.x + particle->velocity.x * lag_offset;
    float y = particle->position.y + particle->velocity.y * lag_offset;

    qu_draw_rectangle(x, y,
        particle->size.x, particle->size.y,
        particle->outline, particle->fill);
}

//------------------------------------------------------------------------------
// Pop-ups

void popup_init(struct popup *popup, struct scene *scene)
{
    *popup = (struct popup) {
        .scene = scene,
        .active = true,
        .main = 0xFFFFFFFF,
        .shadow = 0xFFE00000,
        .start_time = qu_get_time_mediump(),
        .duration = 0.75f,
    };
}

void popup_update(struct popup *popup)
{
    float age = qu_get_time_mediump() - popup->start_time;

    if (age < 0.25f) {
        popup->position.y -= 8.f;
    } else if (age > popup->duration) {
        popup->active = false;
    }
}

void popup_draw(struct popup *popup)
{
    struct resources *resources = &popup->scene->game->resources;
    qu_font font = resources->fonts[FONT_PIXEL_22];

    UI_DRAW_TEXT(font, popup->position.x, popup->position.y,
        popup->main, popup->shadow,
        "%s", popup->buffer);
}

//------------------------------------------------------------------------------
// UI

void ui_draw(struct game *game)
{
    qu_font font = game->resources.fonts[FONT_PIXEL];
    qu_color main = 0xFFFFFFFF;
    qu_color shadow = 0xFFE00000;

    if (game->state != STATE_NORMAL) {
        qu_draw_rectangle(0.f, 0.f, 720.f, 480.f, 0, 0x80000000);
    }

    if (game->state != STATE_TITLE) {
        UI_DRAW_TEXT(font, 16.f, 16.f, main, shadow, "%08d", game->score);
    }

    if (game->state == STATE_NORMAL) {
        qu_draw_rectangle(18.f, 450.f, 144.f, 8.f, 0, shadow);
        qu_draw_rectangle(16.f, 448.f, 144.f * game->scene.player.health,
            8.f, 0, main);
    }

    if (game->state == STATE_TITLE) {
        UI_DRAW_TEXT(font, 220.f, 250.f, main, shadow, "press SPACE to start");
    } else if (game->state == STATE_GAMEOVER) {
        UI_DRAW_TEXT(font, 270.f, 250.f, main, shadow, "game over");
    }
}

//------------------------------------------------------------------------------

struct game game = { 0 };

bool main_loop(void)
{
    game_loop(&game);
    return true;
}

int main(int argc, char *argv[])
{
    qu_initialize(&(qu_params) {
        .display_width = 720,
        .display_height = 480,
        .title = "[libqu] surrounded",
        .enable_canvas = true,
    });

    game_init(&game);
    qu_execute(main_loop);
}

//------------------------------------------------------------------------------
