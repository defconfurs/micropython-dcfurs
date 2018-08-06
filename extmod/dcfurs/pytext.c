#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/objstr.h"
#include "py/builtin.h"
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

/* Lazy implementation to generate random-ish bytes. */
unsigned int
dcfurs_rand(void) {
    static uint32_t seed = 500;
    seed *= 1103515245;
    seed += 12345;
    return seed;
}

static void print_shuffle(const char *str)
{
    unsigned int len = strlen(str);
    unsigned int i;

    /* Spit out some random characters */
    for (i = 0; i < len; i++) {
        printf("%c", 0x20 + (dcfurs_rand() % 94));
    }
    mp_hal_delay_ms(100);
    printf("%c", '\r');
    while (*str != '\0') {
        mp_hal_delay_ms(25);
        printf("%c", *str++);
    }
    printf("%c", '\n');
}

static void print_credit(const char *name, const char *detail)
{
    printf("   " ESC_BOLD "%12s" ESC_NORMAL " - %s\n", name, detail);
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

void dcfurs_bletx(const char *str)
{
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t mod = mp_import_name(MP_QSTR_badge, mp_const_none, 0);
        mp_obj_t ble = mp_load_attr(mod, MP_QSTR_ble);
        mp_obj_t ble_write_args[3] = {
            0,  /* Function object */
            0,  /* UART object */
            mp_obj_new_str(str, strlen(str))
        };
        mp_load_method(ble, MP_QSTR_write, ble_write_args);
        mp_call_method_n_kw(1, 0, ble_write_args);
        nlr_pop();
    }
}

mp_obj_t dcfurs_credits(void)
{
    dcfurs_emote("^.^");

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

#define DCFURS_FAKE_IPADDR      "217.65.287.142"
#define DCFURS_FAKE_BCAST       "217.65.287.255"
#define DCFURS_FAKE_HOSTNAME    "pwned.emptyhex.com"

mp_obj_t dcfurs_awoo(void)
{
    const char *awoonet = "howl.awoonet.com";
    double elapsed = 0.0;
    unsigned int i;

    dcfurs_bletx("tx: awoo\r\n");
    dcfurs_emote("awoo");

    printf("PING %s (%s) 56(64) bytes of data.\n", awoonet, DCFURS_FAKE_BCAST);
    for (i = 0; i < 4; i++) {
        unsigned int x = 200 + dcfurs_rand() % 50;
        unsigned int ip = (dcfurs_rand() % 253) + 1;
        elapsed += (double)x / 1000;
        mp_hal_delay_ms(x);
        printf("64 bytes from 217.65.187.%d (217.65.187.%d): awoo_seq=%d ttl=64 time=%.3f ms\n", ip, ip, i, elapsed);
    }
    printf("\n--- %s ping statistics ---\n", awoonet);
    printf("1 awoo transmitted, %d received, 0%% packet loss, time %d ms\n", i, (int)(elapsed * 1000));
    //printf("rtt min/avg/max/mdev\n");
    return mp_const_none;
}

static uint32_t adler32(const char *str)
{
    uint32_t a = 1, b = 0;
    while (*str != '\0') {
        a = (a + *str++) % 65521;
        b = (b + a) % 66521;
    }
    return (b << 16) | a;
}


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
"settings.bootanim = \"scroll\"\r\n"
;

STATIC void dcfurs_hacked(void)
{
    int i;
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

        x = 200 + dcfurs_rand() % 50;
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
    const char *prompt = "Enter Password: ";
    mp_obj_t password = mp_call_function_1(MP_OBJ_FROM_PTR(&mp_builtin_input_obj), mp_obj_new_str(prompt, strlen(prompt)));
    if (adler32(mp_obj_str_get_str(password)) != 0x3c4e06ff) {
        printf("Incorrect Password!\n");
        printf("Hint: Ignorance is Strength\n");
        dcfurs_emote("X.X");
        return mp_const_none;
    }
    print_shuffle("Access Granted");

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

mp_obj_t dcfurs_eula(void)
{
    dcfurs_emote("@.@");

    printf(ESC_BOLD "PRIVACY POLICY\n" ESC_NORMAL);
    printf("--------------\n");
    printf("We\'ve updated our privacy policy. This is purely out of the goodness of our\n");
    printf("hearts, and has nothing to do with any hypothetical unions on any particular\n");
    printf("continents. Please read every part of this policy carefully, and don\'t just\n");
    printf("skip ahead looking for challenge clues.\n");
    printf("\n");

    printf("This policy governs your interractions with this badge, herein referred to as\n");
    printf("\"the service,\" \"the badge,\" or \"the boop badge,\" and with all other badges\n");
    printf("and hardware of any kind. The enumeration in this policy, of certain rights,\n");
    printf("shall not be construed to as a license to resell this badge on eBay. By using\n");
    printf("this service, you opt-in to hosting the fursuit lounge in your home.\n");
    printf("\n");

    printf(ESC_BOLD "YOUR PERSONAL INFORMATION\n" ESC_NORMAL);
    printf("-------------------------\n");
    printf("Please don\'t send us your personal information. We do not want your personal\n");
    printf("information. We have a hard enough time keeping track of our own information,\n");
    printf("let alone yours.\n");
    printf("\n");

    printf("If you tell us your name, internet handle, fursona, or any other identifying\n");
    printf("information, we will forget it immediately. The next time we see you, we\'ll\n");
    printf("struggle to remember who you are, and try desperately to get through the\n");
    printf("conversation so we can go online and hopefully figure it out.\n");
    printf("\n");

    printf(ESC_BOLD "TRACKING PIXELS, COOKIES and BEACONS\n" ESC_NORMAL);
    printf("------------------------------------\n");
    printf("This badge uses pixels in order to form text and animations, some of which\n");
    printf("may remain in your memory after you have turned off the badge. We use cookies\n");
    printf("to encourage you to roll over, shake a paw, and go for walks. We may use\n");
    printf("beacons to start a howl.\n");
    printf("\n");

    printf(ESC_BOLD "3rd PARTY EXTENSIONS\n" ESC_NORMAL);
    printf("--------------------\n");
    printf("The Saturday night party has been extended, and will continue until midnight.\n");
    printf("\n");

    printf(ESC_BOLD "PERMISSIONS\n" ESC_NORMAL);
    printf("-----------\n");
    printf("For users who are not in the sudoers file, permission has been denied.\n");
    printf("This incident will be reported.\n");
    printf("\n");

    printf(ESC_BOLD "SCOPE AND LIMITATIONS\n" ESC_NORMAL);
    printf("---------------------\n");
    printf("This policy supersedes any applicable federal, state and local laws,\n");
    printf("regulations and ordinances, international treaties, and legal agreements that\n");
    printf("would otherwise apply. If any provision of this policy is found by a court to\n");
    printf("be unenforceable, it nevertheless remains in force.\n");
    printf("\n");

    printf("This organization is not liable and this agreement shall not be construed.\n");
    printf("These statements have not been peer reviewed. This badge is intended as a\n");
    printf("proof that P=NP.\n");
    printf("\n");

    printf("If you know anyone in DC801, please tell them we're cool.\n");
    printf("\n");
    return mp_const_none;
}
