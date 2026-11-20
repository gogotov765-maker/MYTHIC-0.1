/* MYTHIC kernel 0.1 - VGA text, keyboard, command line */

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#define VERSION "0.1"

/* ---------- порты ---------- */
static inline void outb(u16 port, u8 val) {
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}
static inline u8 inb(u16 port) {
    u8 r;
    __asm__ volatile ("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

/* ---------- экран ---------- */
#define VGA_W 80
#define VGA_H 25
static volatile u16 *const vga = (volatile u16 *)0xB8000;
static int cx = 0, cy = 0;
static u8 color = 0x0F; /* белый на чёрном */

static void cursor_update(void) {
    u16 pos = cy * VGA_W + cx;
    outb(0x3D4, 14); outb(0x3D5, pos >> 8);
    outb(0x3D4, 15); outb(0x3D5, pos & 0xFF);
}

static void scroll(void) {
    for (int i = 0; i < VGA_W * (VGA_H - 1); i++)
        vga[i] = vga[i + VGA_W];
    for (int i = 0; i < VGA_W; i++)
        vga[VGA_W * (VGA_H - 1) + i] = (color << 8) | ' ';
    cy = VGA_H - 1;
}

static void putc(char c) {
    if (c == '\n') { cx = 0; cy++; }
    else if (c == '\b') {
        if (cx > 0) { cx--; vga[cy * VGA_W + cx] = (color << 8) | ' '; }
    } else {
        vga[cy * VGA_W + cx] = (color << 8) | (u8)c;
        if (++cx >= VGA_W) { cx = 0; cy++; }
    }
    if (cy >= VGA_H) scroll();
    cursor_update();
}

static void puts(const char *s) { while (*s) putc(*s++); }

static void clear(void) {
    for (int i = 0; i < VGA_W * VGA_H; i++) vga[i] = (color << 8) | ' ';
    cx = cy = 0;
    cursor_update();
}

/* ---------- строки ---------- */
static int streq(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}
static int startswith(const char *s, const char *p) {
    while (*p) if (*s++ != *p++) return 0;
    return 1;
}

/* ---------- клавиатура (PS/2, опрос) ---------- */
static const char map_lo[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' '
};
static const char map_hi[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' '
};

static int shift = 0, caps = 0;

static char getchar(void) {
    for (;;) {
        if (!(inb(0x64) & 1)) continue;
        u8 sc = inb(0x60);
        if (sc == 0x2A || sc == 0x36) { shift = 1; continue; }
        if (sc == 0xAA || sc == 0xB6) { shift = 0; continue; }
        if (sc == 0x3A) { caps = !caps; continue; }
        if (sc & 0x80) continue;             /* отпускание клавиши */
        char c = shift ? map_hi[sc] : map_lo[sc];
        if (caps && c >= 'a' && c <= 'z') c -= 32;
        else if (caps && c >= 'A' && c <= 'Z' && shift) c += 32;
        if (c) return c;
    }
}

/* ---------- команды ---------- */
static void reboot(void) {
    while (inb(0x64) & 2) {}
    outb(0x64, 0xFE);
    for (;;) __asm__ volatile ("hlt");
}

static u8 hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static void run(char *cmd) {
    while (*cmd == ' ') cmd++;
    if (!*cmd) return;

    if (streq(cmd, "help")) {
        puts("Commands:\n");
        puts("  help          - list commands\n");
        puts("  ver           - kernel version\n");
        puts("  clear         - clear screen\n");
        puts("  echo <text>   - print text\n");
        puts("  color <HH>    - set colors, e.g. color 1F (bg+fg, hex)\n");
        puts("  reboot        - restart\n");
        puts("  halt          - stop the CPU\n");
    } else if (streq(cmd, "ver")) {
        puts("MYTHIC kernel " VERSION "\n");
    } else if (streq(cmd, "clear")) {
        clear();
    } else if (startswith(cmd, "echo ")) {
        puts(cmd + 5); putc('\n');
    } else if (startswith(cmd, "color ") && cmd[6] && cmd[7]) {
        color = (hexval(cmd[6]) << 4) | hexval(cmd[7]);
        clear();
    } else if (streq(cmd, "reboot")) {
        reboot();
    } else if (streq(cmd, "halt")) {
        puts("System halted.\n");
        for (;;) __asm__ volatile ("cli; hlt");
    } else {
        puts("Unknown command: "); puts(cmd);
        puts("\nType help\n");
    }
}

/* ---------- вход ---------- */
void kernel_main(void) {
    clear();
    color = 0x0B; /* голубой */
    puts("  __  ____   _______ _   _ ___ ____ \n");
    puts(" |  \\/  \\ \\ / /_   _| | | |_ _/ ___|\n");
    puts(" | |\\/| |\\ V /  | | | |_| || | |    \n");
    puts(" | |  | | | |   | | |  _  || | |___ \n");
    puts(" |_|  |_| |_|   |_| |_| |_|___\\____|\n");
    color = 0x0F;
    puts("\nMYTHIC kernel " VERSION " loaded. Type help\n\n");

    char buf[128];
    for (;;) {
        color = 0x0A; puts("mythic> "); color = 0x0F;
        int n = 0;
        for (;;) {
            char c = getchar();
            if (c == '\n') { putc('\n'); break; }
            if (c == '\b') { if (n > 0) { n--; putc('\b'); } continue; }
            if (c == '\t' || c == 27) continue;
            if (n < (int)sizeof(buf) - 1) { buf[n++] = c; putc(c); }
        }
        buf[n] = 0;
        run(buf);
    }
}
