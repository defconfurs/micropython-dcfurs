#include <stdio.h>
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

    printf("\nStay fuzzy, and happy hacking!\n");
    return mp_const_none;
}
