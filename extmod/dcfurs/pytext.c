#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/runtime.h"
#include "dcfurs.h"

const char *dcfurs_banner =
"    ____  ____________                   ____            __\n"
"   / __ \\/ ____/ ____/_  ____________   / __ )____ _____/ /___ ____\n"
"  / / / / /   / /_  / / / / ___/ ___/  / __  / __ `/ __  / __ `/ _ \\\n"
" / /_/ / /___/ __/ / /_/ / /  _\\_ \\   / /_/ / /_/ / /_/ / /_/ /  __/\n"
"/_____/\\____/_/    \\__,_/_/  /____/  /_____/\\__,_/\\__,_/\\__, /\\___/\n" 
"                                                      /_____/\n";

const char *dcfurs_thanks =
" _______  __  __  ______  __  __  __  __\n"
"/\\_   __\\/\\ \\/\\ \\/\\  __ \\/\\ \\/\\ \\/\\ \\/\\ \\\n"
"\\/_/\\ \\_/\\ \\ \\_\\ \\ \\ \\_\\ \\ \\  \\\\ \\ \\ \\/ /\n"
"   \\ \\ \\  \\ \\  __ \\ \\  __ \\ \\     \\ \\   <\n"
"    \\ \\ \\  \\ \\ \\ \\ \\ \\ \\ \\ \\ \\ \\ \\ \\ \\ \\\\ \\\n"
"     \\ \\_\\  \\ \\_\\ \\_\\ \\_\\ \\_\\ \\_\\ \\_\\ \\_\\ \\_\\\n"
"      \\/_/   \\/_/\\/_/\\/_/\\/_/\\/_/\\/_/\\/_/\\/_/\n"
"                   __   __  ______  __  __\n"
"                  /\\ \\ /\\ \\/\\  __ \\/\\ \\ \\ \\\n"
"                  \\   \\\\/ /\\ \\ \\ \\ \\ \\ \\ \\ \\\n"
"                    \\    /  \\ \\ \\ \\ \\ \\ \\ \\ \\\n"
"                     \\ \\ \\   \\ \\ \\_\\ \\ \\ \\_\\ \\\n"
"                      \\ \\_\\   \\ \\_____\\ \\_____\\\n"
"                       \\/_/    \\/_____/\\/_____/\n";

#define ESC_NORMAL  "\e[0m"
#define ESC_BOLD    "\e[1m"

static void print_credit(const char *name, const char *detail)
{
    printf("   " ESC_BOLD "%12s" ESC_NORMAL " - %s\n", name, detail);
}

static void dcfurs_draw_bitmap(const uint32_t *buf)
{
    int i;
    for (i = 0; i < DCF_TOTAL_ROWS; i++) {
        dcfurs_set_row(MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_NEW_SMALL_INT(buf[i]));
    }
}

void dcfurs_emote(const char *str)
{
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t mod = mp_import_name(MP_QSTR_emote, mp_const_none, 0);
        mp_obj_t fun = mp_load_attr(mod, MP_QSTR_render);
        mp_call_function_1(fun, mp_obj_new_str(str, strlen(str)));
        nlr_pop();
    }
}

void dcfurs_draw_owo(void)
{
    const uint32_t owo[DCF_TOTAL_ROWS] = {
        0x1c00e, 0x22011, 0x22851, 0x22b51, 0x22491, 0x22011, 0x1c00e,
    };
    dcfurs_draw_bitmap(owo);
}

mp_obj_t dcfurs_credits(void)
{
    dcfurs_draw_owo();

    /* Show the Banner */
    printf(ESC_BOLD "%s" ESC_NORMAL, dcfurs_banner);
    printf("\n");

    printf("Brought to you by the dedication and hard work from:\n");
    print_credit("Foobar", "Electronics design and manufacturing");
    print_credit("Alofoxx", "Business, logistics and software");
    print_credit("DranoTheCat", "Graphics design");
    print_credit("Kayfox", "Software and logistics");
    print_credit("LoialOtter", "Animations and firmware");
    print_credit("Liquidthex", "Animations and software");

    printf("\nStay fuzzy, and happy hacking!\n\n");
    return mp_const_none;
}

mp_obj_t dcfurs_beep(void)
{
    printf("boop\n");
    return mp_const_none;
}

mp_obj_t dcfurs_boop(void)
{
    printf("beep\n");
    return mp_const_none;
}

mp_obj_t dcfurs_ctznos(void)
{
    printf("%s", dcfurs_thanks);
    return mp_const_none;
}

static uint32_t adler32_vstr(const vstr_t *vstr)
{
    uint32_t a = 1, b = 0;
    int i;
    for (i = 0; i < vstr->len; i++) {
        a = (a + vstr->buf[i]) % 65521;
        b = (b + a) % 66521;
    }
    return (b << 16) | a;
}

#include "py/mphal.h"
#include "lib/mp-readline/readline.h"

// A port can define mp_hal_readline if they want to use a custom function here
#ifndef mp_hal_readline
#define mp_hal_readline readline
#endif

#define DCFURS_FAKE_IPADDR      "217.65.187.257"
#define DCFURS_FAKE_HOSTNAME    "pwned.infursec.com"

/* hacked boot.py */
static const char pwned_boot_py[] =
"# boot.py -- run on boot-up\r\n"
"# can run arbitrary Python, but best to keep it minimal\r\n"
"\r\n"
"import machine\r\n"
"import pyb\r\n"
"#pyb.main('main.py') # main script to run after this one\r\n"
"#pyb.usb_mode('VCP+MSC') # act as a serial and a storage device\r\n"
"#pyb.usb_mode('VCP+HID') # act as a serial device and a mouse\r\n"
"import settings\r\n"
"settings.banner = \"UBER PWNED By Defalt\"\r\n"
;

STATIC void dcfurs_hacked(void)
{
    int i;
    unsigned int jitter = 500;
    double elapsed = 0.25;

    printf("\nBroadcast message from defalt@localhost (pts/2):\n\n");
    printf("Hah! Nice try, pup.\n\n");
    mp_hal_delay_ms(1000);

    /* Fake wget connection establishment. */
    printf("Resolving %s (%s)... ", DCFURS_FAKE_HOSTNAME, DCFURS_FAKE_HOSTNAME);
    mp_hal_delay_ms(500);
    printf("%s\n", DCFURS_FAKE_IPADDR);
    printf("Caching %s => %s\n", DCFURS_FAKE_HOSTNAME, DCFURS_FAKE_IPADDR);
    printf("Connecting to %s", DCFURS_FAKE_HOSTNAME);
    printf("(%s)|%s|:80... ", DCFURS_FAKE_HOSTNAME, DCFURS_FAKE_IPADDR);
    mp_hal_delay_ms(500);
    printf("connected.\n");
    printf("HTTP request sent, awaiting response... ");
    mp_hal_delay_ms(750);
    printf("200 OK\n");
    printf("Length: %d (%d) [text/html]\n", sizeof(pwned_boot_py), sizeof(pwned_boot_py));
    printf("Saving to: 'boot.py'\n\n");
    mp_hal_delay_ms(1000);

    /* Fake download progress bar. */
    for (i = 1; i <= 20; i++) {
        unsigned int x;
        char *progress = "===================>                    ";
        printf("boot.py             %-3d%%[%.20s] %-6d  --.-KB/s    in %.2fs\r",
                i*5, progress + 20 - i, (sizeof(pwned_boot_py)*i)/20, elapsed);

        jitter = (jitter * jitter) % 511;
        x = 200 + jitter % 50;
        mp_hal_delay_ms(x);
        elapsed += (double)x / 1000;
    }
    printf("\n\n");
}

#if 0 
STATIC void dcfurs_fake_console(void)
{
    vstr_t line;
    vstr_init(&line, 256);
    while (1) {
        int ret = mp_hal_readline(&line, "root@dcfurs:~$ ");
        if (ret == CHAR_CTRL_C) {
            nlr_raise(mp_obj_new_exception(&mp_type_KeyboardInterrupt));
        }
        if (line.len == 0 && ret == CHAR_CTRL_D) {
            nlr_raise(mp_obj_new_exception(&mp_type_EOFError));
        }
        /* Handle fake console stuff. */
        if (strcmp(vstr.buf, "help") == 0) {
            printf("There is no help, only fail.\n");
        } 
        else {
            char *cmd = strtok(vstr.buf, " \t\r\n");
            printf("%s: command not found\n");
        }
    }
}
#endif

#include "uart.h"
#include "usb.h"
#include "storage.h"
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"

STATIC void dcfurs_overwrite_bootpy(void)
{
    const char *bootpy_path = "/flash/boot.py";
    mp_vfs_mount_t *vfs = mp_vfs_lookup_path(bootpy_path, &bootpy_path);
    if (vfs == MP_VFS_NONE || vfs == MP_VFS_ROOT) {
        return;
    }
    // init the vfs object
    FIL fp;
    UINT n;
    
    f_open(&((fs_user_mount_t*)MP_OBJ_TO_PTR(vfs->obj))->fatfs, &fp, bootpy_path, FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, pwned_boot_py, sizeof(pwned_boot_py) - 1 /* don't count null terminator */, &n);
    f_sync(&fp);
    f_close(&fp);
    storage_flush();
}

STATIC void dcfurs_flush_stdin(void)
{
#if MICROPY_HW_ENABLE_USB
    byte c;
    while (usb_vcp_recv_byte(&c) != 0) { /* nop */ }
#endif
    while (MP_STATE_PORT(pyb_stdio_uart) != NULL && uart_rx_any(MP_STATE_PORT(pyb_stdio_uart))) {
        uart_rx_char(MP_STATE_PORT(pyb_stdio_uart));
    }
}

mp_obj_t dcfurs_login(void)
{
    /* Prompt the user for a password. */
    vstr_t line;
    vstr_init(&line, 64);
    int ret = mp_hal_readline(&line, "Enter Password: ");
    if (ret == CHAR_CTRL_C) {
        nlr_raise(mp_obj_new_exception(&mp_type_KeyboardInterrupt));
    }
    if (line.len == 0 && ret == CHAR_CTRL_D) {
        nlr_raise(mp_obj_new_exception(&mp_type_EOFError));
    }

    /* Check the password */
    if (adler32_vstr(&line) != 0x3c4e06ff) {
        printf("Incorrect Password!\n");
        printf("Hint: Ignorance is Strength\n");
        dcfurs_emote("X.X");
        return mp_const_none;
    }

    /* Success! */
    dcfurs_emote("^.^");
    dcfurs_flush_stdin();
    printf("root@dcfurs:~$ ");
    mp_hal_stdin_rx_chr();
    mp_hal_delay_ms(500);

    dcfurs_emote("X.X");
    dcfurs_hacked();
    dcfurs_overwrite_bootpy();

    /* Snarky reset */
    dcfurs_flush_stdin();
    printf("Pwned, press any key to continue\n");
    mp_hal_stdin_rx_chr();
    mp_hal_delay_ms(1000);
    printf("enikeyshchik...");
    mp_hal_delay_ms(1000);
    NVIC_SystemReset();
    return mp_const_none;
}
