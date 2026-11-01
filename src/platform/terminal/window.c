#include <linear_algebra/vec2.h>
#include <platform/utils.h>
#include <platform/window.h>

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#define PROBE_TIMEOUT_MS 1000
#define DRAIN_TIMEOUT_MS 200
#define SIZE_QUERY_INTERVAL_NS 250000000L
#define LOG_BUFFER_SIZE 65536

// Terminals without the kitty keyboard protocol never report a key release, so a
// key counts as held until a deadline. The first one has to outlast the delay
// before autorepeat starts, the next ones only the gap between repeats.
#define KEY_FIRST_HOLD_NS 600000000L
#define KEY_REPEAT_HOLD_NS 120000000L

#define KEY_COUNT (KEY_D + 1)
#define SEQUENCE_MAX 256

// The longest cell is a half block: cursor move + both colors + glyph
#define CELL_MAX_BYTES 56
#define FRAME_EXTRA_BYTES 64

#define NO_COLOR 0xFFFFFFFFu
#define TEXT_COLORS "\x1b[38;2;255;255;255;48;2;0;0;0m"

// A half block shows two pixels in a cell: the foreground paints the top one and
// the background the bottom one
#define PIXELS_PER_CELL 2
#define BLOCK_GLYPH "\xE2\x96\x80"

#define KITTY_KEY_RELEASE 3
#define KITTY_MOD_CTRL 4

#define SIZE_QUERY_CURSOR "\x1b" "7\x1b[9999;9999H\x1b[6n\x1b" "8"

// Set a color and read it back, then ask for the Tc and RGB capabilities. The
// device attributes request is last: virtually every terminal answers it, so its
// reply means that every reply that was coming has arrived.
#define PROBE_SEQUENCE                 \
    "\x1b[38;2;1;2;3m\x1bP$qm\x1b\\\x1b[0m" \
    "\x1bP+q5463;524742\x1b\\"         \
    "\x1b[?u"                          \
    "\x1b[18t"                         \
    SIZE_QUERY_CURSOR                  \
    "\x1b[c"

#define SCREEN_ENTER "\x1b[?1049h\x1b[?25l\x1b[?7l\x1b[?1004h\x1b[0m\x1b[2J"
#define SCREEN_LEAVE "\x1b[?1004l\x1b[0m\x1b[?7h\x1b[?25h\x1b[?1049l"

// Disambiguate + report event types + report all keys as escape codes
#define KITTY_KEYBOARD_PUSH "\x1b[>11u"
#define KITTY_KEYBOARD_POP "\x1b[<u"

typedef enum {
    PARSE_GROUND,
    PARSE_ESC,
    PARSE_SS3,
    PARSE_CSI,
    PARSE_DCS,
    PARSE_DCS_ESC,
    PARSE_OSC,
    PARSE_OSC_ESC
} ParseState;

struct Window {
    int width;
    int height;
    int rows;
    uint32_t *pixels;
    uint32_t *prev_pixels;
    char *text;
    char *prev_text;
    char *frame;
    int force_redraw;
    NsTime last_size_query;
    int window_should_quit;
};

// window_is_key_pressed doesn't get a window
static struct {
    struct termios saved_termios;
    int raw_mode;
    int screen_active;

    ParseState parse_state;
    char sequence[SEQUENCE_MAX];
    size_t sequence_len;

    int da1_received;
    int truecolor;
    int truecolor_denied;
    int kitty_keyboard;
    int size_from_winops;
    int pending_cursor_reports;
    int rows;
    int cols;

    int key_down[KEY_COUNT];
    NsTime key_deadline[KEY_COUNT];
    int quit_requested;
} term;

static volatile sig_atomic_t quit_signal = 0;

static void signal_handler(int signal_number) {
    (void)signal_number;
    quit_signal = 1;
}

static int term_write(const char *data, size_t len) {
    while (len > 0) {
        ssize_t written = write(STDOUT_FILENO, data, len);

        if (written < 0) {
            if (errno == EINTR)
                continue;

            if (errno == EAGAIN) {
                struct pollfd output = { .fd = STDOUT_FILENO, .events = POLLOUT };
                poll(&output, 1, -1);
                continue;
            }

            return -1;
        }

        data += written;
        len -= written;
    }

    return 0;
}

/**
 *
 * INPUT PARSING
 *
 */

static int key_from_char(int character) {
    switch (tolower(character)) {
        case 'w':
            return KEY_W;
        case 'a':
            return KEY_A;
        case 's':
            return KEY_S;
        case 'd':
            return KEY_D;
        default:
            return -1;
    }
}

static void handle_plain_byte(unsigned char byte) {
    if (byte == 0x03 || byte == 'q' || byte == 'Q') {
        term.quit_requested = 1;
        return;
    }

    int key = key_from_char(byte);

    if (key < 0)
        return;

    NsTime now = utils_get_current_time_ns();

    term.key_deadline[key] = now + (now < term.key_deadline[key] ? KEY_REPEAT_HOLD_NS : KEY_FIRST_HOLD_NS);
}

// CSI code[:alternates] ; modifiers[:event] [; text] u
static void handle_kitty_key(const char *sequence) {
    int code = atoi(sequence);
    int modifiers = 1;
    int event_type = 1;

    const char *parameter = strchr(sequence, ';');

    if (parameter) {
        char *end = NULL;
        long parsed_modifiers = strtol(parameter + 1, &end, 10);

        if (parsed_modifiers > 0)
            modifiers = (int)parsed_modifiers;
        if (*end == ':')
            event_type = atoi(end + 1);
    }

    int is_press = event_type != KITTY_KEY_RELEASE;
    int key = key_from_char(code < 128 ? code : 0);

    if (key >= 0)
        term.key_down[key] = is_press;

    if (is_press && (code == 'q' || (code == 'c' && ((modifiers - 1) & KITTY_MOD_CTRL))))
        term.quit_requested = 1;
}

static void handle_csi(char final_byte) {
    const char *sequence = term.sequence;
    int rows = 0;
    int cols = 0;

    switch (final_byte) {
        case 'c':
            if (sequence[0] == '?')
                term.da1_received = 1;
            break;
        case 't':
            if (sscanf(sequence, "8;%d;%d", &rows, &cols) == 2 && rows > 0 && cols > 0) {
                term.rows = rows;
                term.cols = cols;
                term.size_from_winops = 1;
            }
            break;
        case 'R':
            if (term.pending_cursor_reports > 0) {
                term.pending_cursor_reports--;

                if (!term.size_from_winops && sscanf(sequence, "%d;%d", &rows, &cols) == 2 && rows > 0 && cols > 0) {
                    term.rows = rows;
                    term.cols = cols;
                }
            }
            break;
        case 'u':
            if (sequence[0] == '?')
                term.kitty_keyboard = 1;
            else
                handle_kitty_key(sequence);
            break;
        case 'O':
            if (term.sequence_len == 0) {
                memset(term.key_down, 0, sizeof(term.key_down));
                memset(term.key_deadline, 0, sizeof(term.key_deadline));
            }
            break;
    }
}

// Accepts 38;2;r;g;b, 38:2:r:g:b and 38:2::r:g:b, an empty field being the color space
static int sgr_has_probe_color(const char *sgr) {
    enum { FIELDS_MAX = 16 };
    int fields[FIELDS_MAX];
    int fields_count = 0;

    while (fields_count < FIELDS_MAX) {
        char *end = NULL;
        long field = strtol(sgr, &end, 10);

        fields[fields_count++] = end == sgr ? -1 : (int)field;

        if (*end != ';' && *end != ':')
            break;

        sgr = end + 1;
    }

    for (int i = 0; i + 4 < fields_count; i++) {
        if (fields[i] != 38 || fields[i + 1] != 2)
            continue;

        for (int start = i + 2; start <= i + 3 && start + 2 < fields_count; start++) {
            if (fields[start] == 1 && fields[start + 1] == 2 && fields[start + 2] == 3)
                return 1;
        }
    }

    return 0;
}

static int capability_is_rgb(const char *capability, size_t len) {
    return (len == 4 && strncasecmp(capability, "5463", len) == 0) ||
           (len == 6 && strncasecmp(capability, "524742", len) == 0);
}

static void handle_dcs() {
    const char *sequence = term.sequence;

    if (strncmp(sequence, "1$r", 3) == 0) {
        if (sgr_has_probe_color(sequence + 3))
            term.truecolor = 1;
        else
            term.truecolor_denied = 1;
    } else if (strncmp(sequence, "1+r", 3) == 0) {
        const char *entry = sequence + 3;

        while (*entry) {
            size_t entry_len = strcspn(entry, ";");
            size_t name_len = strcspn(entry, "=;");

            if (capability_is_rgb(entry, name_len))
                term.truecolor = 1;

            entry += entry_len;
            if (*entry == ';')
                entry++;
        }
    }
}

static void sequence_append(unsigned char byte) {
    if (term.sequence_len < SEQUENCE_MAX - 1) {
        term.sequence[term.sequence_len++] = (char)byte;
        term.sequence[term.sequence_len] = '\0';
    }
}

static void sequence_begin(ParseState state) {
    term.parse_state = state;
    term.sequence_len = 0;
    term.sequence[0] = '\0';
}

static void parse_byte(unsigned char byte) {
    switch (term.parse_state) {
        case PARSE_GROUND:
            if (byte == 0x1b)
                term.parse_state = PARSE_ESC;
            else
                handle_plain_byte(byte);
            break;
        case PARSE_ESC:
            switch (byte) {
                case '[':
                    sequence_begin(PARSE_CSI);
                    break;
                case 'P':
                    sequence_begin(PARSE_DCS);
                    break;
                case ']':
                    sequence_begin(PARSE_OSC);
                    break;
                case 'O':
                    term.parse_state = PARSE_SS3;
                    break;
                case 0x1b:
                    break;
                default:
                    term.parse_state = PARSE_GROUND;
                    break;
            }
            break;
        case PARSE_SS3:
            term.parse_state = PARSE_GROUND;
            break;
        case PARSE_CSI:
            if (byte == 0x1b) {
                term.parse_state = PARSE_ESC;
            } else if (byte >= 0x40 && byte <= 0x7E) {
                term.parse_state = PARSE_GROUND;
                handle_csi((char)byte);
            } else {
                sequence_append(byte);
            }
            break;
        case PARSE_DCS:
            if (byte == 0x1b)
                term.parse_state = PARSE_DCS_ESC;
            else
                sequence_append(byte);
            break;
        case PARSE_DCS_ESC:
            if (byte == '\\') {
                term.parse_state = PARSE_GROUND;
                handle_dcs();
            } else {
                term.parse_state = PARSE_ESC;
                parse_byte(byte);
            }
            break;
        case PARSE_OSC:
            if (byte == 0x07)
                term.parse_state = PARSE_GROUND;
            else if (byte == 0x1b)
                term.parse_state = PARSE_OSC_ESC;
            break;
        case PARSE_OSC_ESC:
            if (byte == '\\') {
                term.parse_state = PARSE_GROUND;
            } else {
                term.parse_state = PARSE_ESC;
                parse_byte(byte);
            }
            break;
    }
}

static int read_input() {
    unsigned char buffer[4096];
    ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));

    if (bytes_read < 0 && errno != EINTR && errno != EAGAIN)
        return -1;

    for (ssize_t i = 0; i < bytes_read; i++) {
        parse_byte(buffer[i]);
    }

    return 0;
}

// Reads replies until the device attributes reply arrives or the time is up
static void wait_for_da1(int timeout_ms) {
    NsTime deadline = utils_get_current_time_ns() + timeout_ms * 1000000L;

    while (!term.da1_received) {
        NsTime remaining = deadline - utils_get_current_time_ns();

        if (remaining <= 0)
            break;

        struct pollfd input = { .fd = STDIN_FILENO, .events = POLLIN };

        if (poll(&input, 1, (int)(remaining / 1000000L) + 1) > 0)
            read_input();
    }
}

/**
 *
 * TERMINAL STATE
 *
 */

static void terminal_restore(void) {
    if (term.screen_active) {
        term.screen_active = 0;

        if (term.kitty_keyboard)
            term_write(KITTY_KEYBOARD_POP, strlen(KITTY_KEYBOARD_POP));

        term_write(SCREEN_LEAVE, strlen(SCREEN_LEAVE));
    }

    if (term.raw_mode) {
        term.raw_mode = 0;

        // Replies still on their way would otherwise be typed into the shell
        term.da1_received = 0;
        term_write("\x1b[c", 3);
        wait_for_da1(DRAIN_TIMEOUT_MS);

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &term.saved_termios);
    }
}

// Many terminals render truecolor without answering the queries, but announce it here
static int colorterm_has_truecolor() {
    const char *colorterm = getenv("COLORTERM");

    return colorterm && (strcmp(colorterm, "truecolor") == 0 || strcmp(colorterm, "24bit") == 0);
}

static int terminal_enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &term.saved_termios) != 0)
        return -1;

    struct termios raw = term.saved_termios;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~OPOST;
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // Not O_NONBLOCK: stdin and stdout are usually the same open file description
    // and frame writes have to keep blocking
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0)
        return -1;

    term.raw_mode = 1;

    return 0;
}

static void terminal_install_handlers() {
    struct sigaction action = { .sa_handler = signal_handler, .sa_flags = SA_RESTART };

    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
}

static int window_allocate_buffers(Window *window, int cols, int rows) {
    size_t cells = (size_t)cols * rows;
    size_t pixels = cells * PIXELS_PER_CELL;

    free(window->pixels);
    free(window->prev_pixels);
    free(window->text);
    free(window->prev_text);
    free(window->frame);

    window->pixels = calloc(pixels, sizeof(uint32_t));
    window->prev_pixels = calloc(pixels, sizeof(uint32_t));
    window->text = calloc(cells, sizeof(char));
    window->prev_text = calloc(cells, sizeof(char));
    window->frame = malloc(cells * CELL_MAX_BYTES + FRAME_EXTRA_BYTES);

    if (!window->pixels || !window->prev_pixels || !window->text || !window->prev_text || !window->frame)
        return -1;

    window->width = cols;
    window->height = rows * PIXELS_PER_CELL;
    window->rows = rows;
    window->force_redraw = 1;

    return 0;
}

static void window_free_buffers(Window *window) {
    free(window->pixels);
    free(window->prev_pixels);
    free(window->text);
    free(window->prev_text);
    free(window->frame);
    free(window);
}

Window *window_create() {
    // stderr shares the tty with the frame being drawn. Fully buffered, the logs stay
    // in memory until the program exits, which is after the terminal is restored.
    static char log_buffer[LOG_BUFFER_SIZE];
    setvbuf(stderr, log_buffer, _IOFBF, sizeof(log_buffer));

    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        utils_log(LOG_ERROR, "The terminal backend needs stdin and stdout to be a terminal");
        return NULL;
    }

    terminal_install_handlers();

    if (terminal_enable_raw_mode() != 0) {
        utils_log(LOG_ERROR, "Couldn't switch the terminal to raw mode");
        return NULL;
    }

    term.pending_cursor_reports = 1;

    if (term_write(PROBE_SEQUENCE, strlen(PROBE_SEQUENCE)) != 0) {
        terminal_restore();
        utils_log(LOG_ERROR, "Couldn't write to the terminal");
        return NULL;
    }

    wait_for_da1(PROBE_TIMEOUT_MS);

    // Anything typed during the probe isn't meant for the game
    int truecolor = term.truecolor || (!term.truecolor_denied && colorterm_has_truecolor());
    int rows = term.rows;
    int cols = term.cols;

    term.quit_requested = 0;
    memset(term.key_deadline, 0, sizeof(term.key_deadline));

    if (!truecolor) {
        terminal_restore();
        utils_log(LOG_ERROR, "This terminal didn't confirm truecolor support: reading back a color (DECRQSS), "
                             "asking for the Tc and RGB capabilities (XTGETTCAP) and COLORTERM all failed");
        return NULL;
    }

    if (rows <= 0 || cols <= 0) {
        terminal_restore();
        utils_log(LOG_ERROR, "This terminal didn't report its size");
        return NULL;
    }

    Window *window = calloc(1, sizeof(Window));

    if (!window || window_allocate_buffers(window, cols, rows) != 0) {
        terminal_restore();
        utils_log(LOG_ERROR, "Couldn't allocate the framebuffers for a %dx%d terminal", cols, rows);

        if (window)
            window_free_buffers(window);

        return NULL;
    }

    term_write(SCREEN_ENTER, strlen(SCREEN_ENTER));

    if (term.kitty_keyboard)
        term_write(KITTY_KEYBOARD_PUSH, strlen(KITTY_KEYBOARD_PUSH));

    term.screen_active = 1;

    window->last_size_query = utils_get_current_time_ns();

    return window;
}

/**
 *
 * FRAME OUTPUT
 *
 */

static size_t append_string(char *out, size_t len, const char *string, size_t string_len) {
    memcpy(out + len, string, string_len);

    return len + string_len;
}

static size_t append_uint(char *out, size_t len, unsigned int value) {
    char digits[10];
    int digits_count = 0;

    do {
        digits[digits_count++] = '0' + value % 10;
        value /= 10;
    } while (value > 0);

    while (digits_count > 0) {
        out[len++] = digits[--digits_count];
    }

    return len;
}

static size_t append_cursor_move(char *out, size_t len, int x, int y) {
    len = append_string(out, len, "\x1b[", 2);
    len = append_uint(out, len, y + 1);
    out[len++] = ';';
    len = append_uint(out, len, x + 1);
    out[len++] = 'H';

    return len;
}

static size_t append_color(char *out, size_t len, unsigned int sgr_code, uint32_t color) {
    len = append_string(out, len, "\x1b[", 2);
    len = append_uint(out, len, sgr_code);
    len = append_string(out, len, ";2;", 3);
    len = append_uint(out, len, (color >> 16) & 0xFF);
    out[len++] = ';';
    len = append_uint(out, len, (color >> 8) & 0xFF);
    out[len++] = ';';
    len = append_uint(out, len, color & 0xFF);
    out[len++] = 'm';

    return len;
}

void window_flip(Window *window) {
    char *out = window->frame;
    size_t len = 0;

    int cursor_x = -1;
    int cursor_y = -1;
    uint32_t last_foreground = NO_COLOR;
    uint32_t last_background = NO_COLOR;

    len = append_string(out, len, "\x1b[?2026h", 8);

    if (window->force_redraw)
        len = append_string(out, len, "\x1b[2J", 4);

    for (int y = 0; y < window->rows; y++) {
        for (int x = 0; x < window->width; x++) {
            size_t cell = (size_t)y * window->width + x;
            size_t top_pixel = (size_t)y * PIXELS_PER_CELL * window->width + x;
            size_t bottom_pixel = top_pixel + (PIXELS_PER_CELL - 1) * (size_t)window->width;
            uint32_t top = window->pixels[top_pixel];
            uint32_t bottom = window->pixels[bottom_pixel];
            char text = window->text[cell];

            if (!window->force_redraw && top == window->prev_pixels[top_pixel] &&
                bottom == window->prev_pixels[bottom_pixel] && text == window->prev_text[cell])
                continue;

            if (cursor_x != x || cursor_y != y)
                len = append_cursor_move(out, len, x, y);

            if (text) {
                len = append_string(out, len, TEXT_COLORS, strlen(TEXT_COLORS));
                out[len++] = text;
                len = append_string(out, len, "\x1b[49m", 5);
                last_foreground = NO_COLOR;
                last_background = NO_COLOR;
            } else {
                if (top != last_foreground) {
                    len = append_color(out, len, 38, top);
                    last_foreground = top;
                }

                if (bottom != last_background) {
                    len = append_color(out, len, 48, bottom);
                    last_background = bottom;
                }

                len = append_string(out, len, BLOCK_GLYPH, strlen(BLOCK_GLYPH));
            }

            cursor_x = x + 1;
            cursor_y = y;
        }
    }

    len = append_string(out, len, "\x1b[?2026l", 8);

    if (term_write(out, len) != 0)
        window->window_should_quit = 1;

    uint32_t *displayed_pixels = window->pixels;
    window->pixels = window->prev_pixels;
    window->prev_pixels = displayed_pixels;

    char *displayed_text = window->text;
    window->text = window->prev_text;
    window->prev_text = displayed_text;

    window->force_redraw = 0;
}

void window_clear_surface(Window *window) {
    size_t cells = (size_t)window->width * window->rows;

    memset(window->pixels, 0, cells * PIXELS_PER_CELL * sizeof(uint32_t));
    memset(window->text, 0, cells);
}

int window_resize(Window *window) {
    if (window_allocate_buffers(window, term.cols, term.rows) != 0) {
        utils_log(LOG_ERROR, "Couldn't allocate the framebuffers for a %dx%d terminal", term.cols, term.rows);
        return 1;
    }

    return 0;
}

int window_get_width(Window *window) {
    return window->width;
}

int window_get_height(Window *window) {
    return window->height;
}

int window_should_quit(Window *window) {
    return window->window_should_quit;
}

Event window_poll_event(Window *window) {
    if (read_input() != 0)
        term.quit_requested = 1;

    if (term.quit_requested || quit_signal) {
        term.quit_requested = 0;
        quit_signal = 0;

        return EVENT_QUIT_APP;
    }

    NsTime now = utils_get_current_time_ns();

    if (now - window->last_size_query >= SIZE_QUERY_INTERVAL_NS) {
        window->last_size_query = now;

        if (term.size_from_winops) {
            term_write("\x1b[18t", 5);
        } else {
            term.pending_cursor_reports++;
            term_write(SIZE_QUERY_CURSOR, strlen(SIZE_QUERY_CURSOR));
        }
    }

    if (term.cols != window->width || term.rows != window->rows)
        return EVENT_WINDOW_RESIZED;

    return EVENT_NONE;
}

int window_is_key_pressed(KeyPress key) {
    if (term.kitty_keyboard)
        return term.key_down[key];

    return utils_get_current_time_ns() < term.key_deadline[key];
}

void window_draw_line(Window *window, Vec2 norm_start, Vec2 norm_end, RGBA *colors, int colors_count) {
    int width = window->width;
    int height = window->height;

    Vec2 end = vec2_map_norm_coord_cp(&norm_end, width, height);
    Vec2 start = vec2_map_norm_coord_cp(&norm_start, width, height);

    end = (Vec2) {
        .x = round(end.x),
        .y = round(end.y)
    };
    start = (Vec2) {
        .x = round(start.x),
        .y = round(start.y)
    };

    if ((int)end.x >= width) {
        end.x = width - 1;
    }
    if ((int)end.y >= height) {
        end.y = height - 1;
    }
    if ((int)start.x < 0) {
        start.x = 0;
    }
    if ((int)start.y < 0) {
        start.y = 0;
    }

    Vec2 ray_dir = end;
    vec2_subtract_vec2(&ray_dir, &start);

    double steps = fabs(ray_dir.x) > fabs(ray_dir.y) ? fabs(ray_dir.x) : fabs(ray_dir.y);

    double x_inc = ray_dir.x / steps;
    double y_inc = ray_dir.y / steps;

    int current_color_index = 0;

    for (int i = 0; i <= steps; i++) {
        int x = (int)start.x;
        int y = (int)start.y;

        RGBA *current_color = &colors[current_color_index];

        if (current_color->a != 0x00 && x >= 0 && x < width && y >= 0 && y < height) {
            window->pixels[(size_t)y * width + x] = (uint32_t)(current_color->r & 0xFF) << 16 |
                                                    (uint32_t)(current_color->g & 0xFF) << 8 |
                                                    (uint32_t)(current_color->b & 0xFF);
        }

        start.x += x_inc;
        start.y += y_inc;

        if (current_color_index < colors_count - 1)
            current_color_index++;
    }
}

void window_draw_text(Window *window, Vec2 norm_pos, const char *font_path,
                      double size, const char *fmt, ...) {
    (void)font_path;
    (void)size;

    char formatted_string[256];
    va_list ap;

    va_start(ap, fmt);
    int formatted_string_len = vsnprintf(formatted_string, sizeof(formatted_string), fmt, ap);
    va_end(ap);

    if (formatted_string_len < 0) {
        utils_log(LOG_ERROR, "Couldn't format the provided string in %s", __FUNCTION__);
        return;
    }

    vec2_map_norm_coord(&norm_pos, window->width, window->rows);

    int x = (int)norm_pos.x;
    int y = (int)norm_pos.y;

    if (x < 0 || y < 0 || y >= window->rows)
        return;

    for (int i = 0; formatted_string[i] != '\0' && x + i < window->width; i++) {
        unsigned char character = (unsigned char)formatted_string[i];

        window->text[(size_t)y * window->width + x + i] = isprint(character) ? (char)character : '?';
    }
}

void window_close(Window *window) {
    window->window_should_quit = 1;
}

void window_destroy(Window *window) {
    terminal_restore();
    window_free_buffers(window);
}
