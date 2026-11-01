#include <level.h>
#include <platform/utils.h>

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSMN_STATIC
#define JSMN_STRICT
#include <jsmn.h>

#define LEVEL_PATH_SIZE 64
#define LEVEL_TEXTURE_PATH_SIZE 512
#define LEVEL_ERROR_SIZE 256
#define LEVEL_MIN_SIZE 3

typedef struct {
    const char *path;
    const char *json;
    const jsmntok_t *tokens;
} LevelFile;

static int level_error(const LevelFile *file, const char *fmt, ...) {
    char reason[LEVEL_ERROR_SIZE];
    va_list args;

    va_start(args, fmt);
    vsnprintf(reason, sizeof(reason), fmt, args);
    va_end(args);

    utils_log(LOG_ERROR, "Couldn't load the level %s: %s", file->path, reason);

    return 1;
}

/**
 *
 * JSON TOKEN HELPERS
 *
 */

static int token_skip(const jsmntok_t *tokens, int index) {
    int children = tokens[index].size;

    index++;

    for (int i = 0; i < children; i++) {
        index = token_skip(tokens, index);
    }

    return index;
}

static int object_find(const LevelFile *file, int object, const char *key) {
    const jsmntok_t *tokens = file->tokens;
    size_t key_len = strlen(key);
    int index = object + 1;

    for (int i = 0; i < tokens[object].size; i++) {
        const jsmntok_t *current_key = &tokens[index];

        if (current_key->type == JSMN_STRING && (size_t)(current_key->end - current_key->start) == key_len &&
            memcmp(file->json + current_key->start, key, key_len) == 0) {
            return index + 1;
        }

        index = token_skip(tokens, index);
    }

    return -1;
}

static int object_find_typed(const LevelFile *file, int object, const char *key, jsmntype_t type, int *value) {
    *value = object_find(file, object, key);

    if (*value == -1)
        return level_error(file, "the key \"%s\" is missing", key);

    if (file->tokens[*value].type != type)
        return level_error(file, "the key \"%s\" has the wrong type", key);

    return 0;
}

static int token_read_double(const LevelFile *file, int index, double *value) {
    const jsmntok_t *token = &file->tokens[index];
    char *end = NULL;

    if (token->type != JSMN_PRIMITIVE)
        return 1;

    *value = strtod(file->json + token->start, &end);

    return end != file->json + token->end;
}

static int token_read_int(const LevelFile *file, int index, int *value) {
    double number;

    if (token_read_double(file, index, &number) != 0 || number < INT_MIN || number > INT_MAX || number != (int)number)
        return 1;

    *value = (int)number;

    return 0;
}

static int object_read_double(const LevelFile *file, int object, const char *key, double *value) {
    int index = 0;

    if (object_find_typed(file, object, key, JSMN_PRIMITIVE, &index) != 0)
        return 1;

    if (token_read_double(file, index, value) != 0)
        return level_error(file, "the key \"%s\" isn't a number", key);

    return 0;
}

static int object_read_int(const LevelFile *file, int object, const char *key, int *value) {
    int index = 0;

    if (object_find_typed(file, object, key, JSMN_PRIMITIVE, &index) != 0)
        return 1;

    if (token_read_int(file, index, value) != 0)
        return level_error(file, "the key \"%s\" isn't a whole number", key);

    return 0;
}

/**
 *
 * LEVEL SECTIONS
 *
 */

static int position_is_valid(double x, double y) {
    return x >= 0.0 && x <= 1.0 && y >= 0.0 && y <= 1.0;
}

static int level_parse_player(Level *level, const LevelFile *file, int root) {
    int player = 0;

    if (object_find_typed(file, root, "player", JSMN_OBJECT, &player) != 0)
        return 1;

    if (object_read_double(file, player, "x", &level->player_position.x) != 0 ||
        object_read_double(file, player, "y", &level->player_position.y) != 0 ||
        object_read_double(file, player, "look_at", &level->player_look_at) != 0)
        return 1;

    if (!position_is_valid(level->player_position.x, level->player_position.y))
        return level_error(file, "the player's position is outside of the 0.0 to 1.0 range");

    return 0;
}

static int level_parse_textures(Level *level, const LevelFile *file, int root) {
    const jsmntok_t *tokens = file->tokens;
    int textures = 0;

    if (object_find_typed(file, root, "textures", JSMN_ARRAY, &textures) != 0)
        return 1;

    int textures_count = tokens[textures].size;
    int index = textures + 1;

    if (textures_count == 0)
        return level_error(file, "the key \"textures\" is empty");

    level->textures = calloc(textures_count, sizeof(Texture));
    level->textures_count = textures_count;

    for (int i = 0; i < textures_count; i++) {
        const jsmntok_t *token = &tokens[index];
        int path_len = token->end - token->start;
        char texture_path[LEVEL_TEXTURE_PATH_SIZE];
        const char *failure = NULL;

        if (token->type != JSMN_STRING || path_len >= LEVEL_TEXTURE_PATH_SIZE)
            return level_error(file, "the texture number %d isn't a valid path", i + 1);

        memcpy(texture_path, file->json + token->start, path_len);
        texture_path[path_len] = '\0';

        if (texture_load(&level->textures[i], texture_path, &failure) != 0)
            return level_error(file, "couldn't load the texture %s: %s", texture_path, failure);

        index = token_skip(tokens, index);
    }

    return 0;
}

static int level_parse_lights(Level *level, const LevelFile *file, int root) {
    const jsmntok_t *tokens = file->tokens;
    int lights = 0;

    if (object_find_typed(file, root, "light_textures", JSMN_ARRAY, &lights) != 0)
        return 1;

    int index = lights + 1;

    level->lights_count = tokens[lights].size;
    level->lights = calloc(level->lights_count, sizeof(LevelLight));

    for (int i = 0; i < level->lights_count; i++) {
        LevelLight *light = &level->lights[i];

        if (tokens[index].type != JSMN_OBJECT)
            return level_error(file, "the light number %d isn't an object", i + 1);

        if (object_read_int(file, index, "texture", &light->texture) != 0 ||
            object_read_double(file, index, "radius", &light->radius) != 0 ||
            object_read_double(file, index, "intensity", &light->intensity) != 0)
            return 1;

        if (light->texture < 1 || light->texture > level->textures_count)
            return level_error(file, "the light number %d uses the unknown texture %d", i + 1, light->texture);

        index = token_skip(tokens, index);
    }

    return 0;
}

static int level_parse_sprites(Level *level, const LevelFile *file, int root) {
    const jsmntok_t *tokens = file->tokens;
    int sprites = 0;

    if (object_find_typed(file, root, "sprites", JSMN_ARRAY, &sprites) != 0)
        return 1;

    int index = sprites + 1;

    level->sprites_count = tokens[sprites].size;
    level->sprites = calloc(level->sprites_count, sizeof(Sprite));

    for (int i = 0; i < level->sprites_count; i++) {
        Sprite *sprite = &level->sprites[i];
        int texture = 0;

        if (tokens[index].type != JSMN_OBJECT)
            return level_error(file, "the sprite number %d isn't an object", i + 1);

        if (object_read_double(file, index, "x", &sprite->position.x) != 0 ||
            object_read_double(file, index, "y", &sprite->position.y) != 0 ||
            object_read_int(file, index, "texture", &texture) != 0)
            return 1;

        if (!position_is_valid(sprite->position.x, sprite->position.y))
            return level_error(file, "the sprite number %d is outside of the 0.0 to 1.0 range", i + 1);

        if (texture < 1 || texture > level->textures_count)
            return level_error(file, "the sprite number %d uses the unknown texture %d", i + 1, texture);

        sprite->texture = &level->textures[texture - 1];

        index = token_skip(tokens, index);
    }

    return 0;
}

static int level_parse_map(Level *level, const LevelFile *file, int root, const char *key, Map *map) {
    const jsmntok_t *tokens = file->tokens;
    int rows = 0;

    if (object_find_typed(file, root, key, JSMN_ARRAY, &rows) != 0)
        return 1;

    if (tokens[rows].size != level->size)
        return level_error(file, "\"%s\" has %d rows instead of %d", key, tokens[rows].size, level->size);

    *map = (Map) {
        .width = level->size,
        .height = level->size,
        .map_2d = malloc(sizeof(CellType) * level->size * level->size),
        .wall_empty = 0
    };

    int index = rows + 1;

    for (int y = 0; y < level->size; y++) {
        if (tokens[index].type != JSMN_ARRAY || tokens[index].size != level->size)
            return level_error(file, "the row %d of \"%s\" doesn't have %d cells", y, key, level->size);

        for (int x = 0; x < level->size; x++) {
            int cell_index = index + 1 + x;
            int cell = 0;

            if (token_read_int(file, cell_index, &cell) != 0)
                return level_error(file, "the cell %d,%d of \"%s\" isn't a whole number", x, y, key);

            if (cell < 0 || cell > level->textures_count)
                return level_error(file, "the cell %d,%d of \"%s\" uses the unknown texture %d", x, y, key, cell);

            map->map_2d[y * level->size + x] = cell;
        }

        index = token_skip(tokens, index);
    }

    return 0;
}

// The raycaster doesn't bounds check the map, it relies on rays always hitting a wall
static int level_check_walls(Level *level, const LevelFile *file) {
    Map *walls_map = &level->walls_map;
    int last = level->size - 1;

    for (int i = 0; i < level->size; i++) {
        if (map_check_intersection(walls_map, i, 0) == walls_map->wall_empty ||
            map_check_intersection(walls_map, i, last) == walls_map->wall_empty ||
            map_check_intersection(walls_map, 0, i) == walls_map->wall_empty ||
            map_check_intersection(walls_map, last, i) == walls_map->wall_empty)
            return level_error(file, "the outer border of \"walls\" has a gap");
    }

    int player_x = (int)(level->player_position.x * level->size);
    int player_y = (int)(level->player_position.y * level->size);

    if (player_x >= level->size || player_y >= level->size ||
        map_check_intersection(walls_map, player_x, player_y) != walls_map->wall_empty)
        return level_error(file, "the player starts inside of a wall");

    return 0;
}

static int level_parse(Level *level, const LevelFile *file) {
    if (file->tokens[0].type != JSMN_OBJECT)
        return level_error(file, "the file isn't a JSON object");

    if (object_read_int(file, 0, "size", &level->size) != 0)
        return 1;

    if (level->size < LEVEL_MIN_SIZE)
        return level_error(file, "the size has to be at least %d", LEVEL_MIN_SIZE);

    if (level_parse_player(level, file, 0) != 0 ||
        level_parse_textures(level, file, 0) != 0 ||
        level_parse_lights(level, file, 0) != 0 ||
        level_parse_sprites(level, file, 0) != 0 ||
        level_parse_map(level, file, 0, "walls", &level->walls_map) != 0 ||
        level_parse_map(level, file, 0, "floor", &level->floor_map) != 0 ||
        level_parse_map(level, file, 0, "ceiling", &level->ceiling_map) != 0)
        return 1;

    return level_check_walls(level, file);
}

static char *file_read(FILE *stream, long *size) {
    if (fseek(stream, 0, SEEK_END) != 0 || (*size = ftell(stream)) < 0 || fseek(stream, 0, SEEK_SET) != 0)
        return NULL;

    char *contents = malloc(*size + 1);

    if (!contents)
        return NULL;

    if (fread(contents, 1, *size, stream) != (size_t)*size) {
        free(contents);
        return NULL;
    }

    contents[*size] = '\0';

    return contents;
}

static int level_load(Level *level, const char *path, FILE *stream) {
    LevelFile file = { .path = path };
    long size = 0;
    int result = 1;

    char *json = file_read(stream, &size);

    if (!json)
        return level_error(&file, "the file can't be read");

    jsmn_parser parser;

    jsmn_init(&parser);
    int tokens_count = jsmn_parse(&parser, json, size, NULL, 0);

    if (tokens_count <= 0) {
        free(json);
        return level_error(&file, "the file isn't valid JSON");
    }

    jsmntok_t *tokens = malloc(sizeof(jsmntok_t) * tokens_count);

    jsmn_init(&parser);

    if (jsmn_parse(&parser, json, size, tokens, tokens_count) <= 0) {
        level_error(&file, "the file isn't valid JSON");
    } else {
        file.json = json;
        file.tokens = tokens;
        result = level_parse(level, &file);
    }

    free(tokens);
    free(json);

    return result;
}

static void level_free(Level *level) {
    for (int i = 0; i < level->textures_count; i++) {
        texture_destroy(&level->textures[i]);
    }

    free(level->textures);
    free(level->sprites);
    free(level->lights);
    free(level->walls_map.map_2d);
    free(level->floor_map.map_2d);
    free(level->ceiling_map.map_2d);
}

static int level_get_light_properties(Level *level, CellType cell, double *radius, double *intensity) {
    for (int i = 0; i < level->lights_count; i++) {
        if (level->lights[i].texture == cell) {
            *radius = level->lights[i].radius;
            *intensity = level->lights[i].intensity;
            return 1;
        }
    }

    return 0;
}

void level_get_world_static_lights(Level *level, Map *map, Light **static_lights, int *lights_count, Vec3 light_offset) {
    static int light_sources_capacity = 10;

    if (!(*static_lights))
        *static_lights = malloc(sizeof(Light) * light_sources_capacity);

    for (int x = 0; x < map->width; x++) {
        for (int y = 0; y < map->height; y++) {
            Vec3 light_pos = {
                .x = (double)x + 0.5,
                .y = (double)y + 0.5,
                .z = 0.0
            };
            vec3_add_vec3(&light_pos, &light_offset);

            double radius, intensity;

            if (level_get_light_properties(level, map_check_intersection(map, x, y), &radius, &intensity)) {
                (*static_lights)[*lights_count] = (Light) {
                    .position = light_pos,
                    .intensity = intensity,
                    .radius = radius
                };

                (*lights_count)++;
            }

            if (*lights_count == light_sources_capacity) {
                light_sources_capacity += 10;
                *static_lights = realloc(*static_lights, sizeof(Light) * light_sources_capacity);
            }
        }
    }
}

void levels_free(Level *levels, int levels_count) {
    for (int i = 0; i < levels_count; i++) {
        level_free(&levels[i]);
    }

    free(levels);
}

Level *levels_load(int *levels_count) {
    Level *levels = NULL;
    int count = 0;

    for (int number = 1; ; number++) {
        char path[LEVEL_PATH_SIZE];

        snprintf(path, sizeof(path), "./levels/%02d.json", number);

        FILE *stream = fopen(path, "rb");

        if (!stream)
            break;

        levels = realloc(levels, sizeof(Level) * (count + 1));
        levels[count] = (Level) {};
        count++;

        int result = level_load(&levels[count - 1], path, stream);

        fclose(stream);

        if (result != 0) {
            levels_free(levels, count);
            return NULL;
        }
    }

    if (count == 0) {
        utils_log(LOG_ERROR, "Couldn't find any levels: ./levels/01.json is missing");
        return NULL;
    }

    *levels_count = count;

    return levels;
}
