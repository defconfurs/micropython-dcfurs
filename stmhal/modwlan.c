#include <string.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"
#include "mpconfig.h"
#include "nlr.h"
#include "misc.h"
#include "qstr.h"
#include "obj.h"
#include "objtuple.h"
#include "runtime.h"
#include "portmodules.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "spi.h"

#include "hci.h"
#include "ccspi.h"
#include "wlan.h"
#include "nvmem.h"
#include "netapp.h"
#include "patch_prog.h"

#define IS_WLAN_SEC(sec) \
    (sec>WLAN_SEC_UNSEC && sec<=WLAN_SEC_WPA2)
#define MAX_PACKET_LENGTH (1024)

STATIC volatile int fd_state=0;
STATIC volatile int ip_obtained = 0;
STATIC volatile int wlan_connected = 0;

int mod_wlan_get_fd_state(int fd) {
    return (fd_state & (1<<fd));
}

void mod_wlan_clear_fd_state(int fd) {
    // reset socket state
    fd_state &= ~(1<<fd);
}

STATIC void sWlanCallback(long lEventType, char * data, unsigned char length) {
    switch (lEventType) {
        case HCI_EVNT_WLAN_UNSOL_CONNECT:
            wlan_connected = 1;
            break;
        case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
            // link down
            wlan_connected = 0;
            break;
        case HCI_EVNT_WLAN_UNSOL_DHCP:
            ip_obtained = 1;
            break;
        case HCI_EVNT_BSD_TCP_CLOSE_WAIT:
            // mark socket for closure
            fd_state |= (1<<((uint8_t)data[0]));
            break;
    }
}

/// \function init(spi, pin_cs, pin_en, pin_irq)
/// Initialise the CC3000 using the given SPI bus and pins.
// Note: pins were originally hard-coded to:
//      PYBv1.0: init(pyb.SPI(2), pyb.Pin.board.Y5, pyb.Pin.board.Y4, pyb.Pin.board.Y3)
//        [SPI on Y position; Y6=B13=SCK, Y7=B14=MISO, Y8=B15=MOSI]
//
//      STM32F4DISC: init(pyb.SPI(2), pyb.Pin.cpu.A15, pyb.Pin.cpu.B10, pyb.Pin.cpu.B11)
STATIC mp_obj_t modwlan_init(mp_uint_t n_args, const mp_obj_t *args) {
    // set the pins to use
    SpiInit(
        spi_get_handle(args[0]),
        pin_find(args[1]),
        pin_find(args[2]),
        pin_find(args[3])

    );

    // initialize and start the module
    wlan_init(sWlanCallback, NULL, NULL, NULL,
            ReadWlanInterruptPin, SpiResumeSpi, SpiPauseSpi, WriteWlanPin);

    if (wlan_start(0) != 0) {
        nlr_raise(mp_obj_new_exception_msg(
                    &mp_type_OSError, "Failed to init wlan module"));
    }

    // set connection policy. this should be called explicitly by the user
    // wlan_ioctl_set_connection_policy(0, 0, 0);

    // Mask out all non-required events from the CC3000
    wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|
                        HCI_EVNT_WLAN_UNSOL_INIT|
                        HCI_EVNT_WLAN_ASYNC_PING_REPORT|
                        HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE);

    return mp_const_none;
}

STATIC mp_obj_t modwlan_connect(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    int ssid_len =0;
    const char *ssid = NULL;
    const char *bssid = NULL;

    int key_len =0;
    int sec = WLAN_SEC_UNSEC;
    const char *key = NULL;

    mp_map_elem_t *kw_key, *kw_sec, *kw_bssid;

    ssid = mp_obj_str_get_str(args[0]);
    ssid_len = strlen(ssid);

    // get KW args
    kw_key = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("key")), MP_MAP_LOOKUP);
    kw_sec = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("sec")), MP_MAP_LOOKUP);
    kw_bssid = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("bssid")), MP_MAP_LOOKUP);

    // get key and sec
    if (kw_key && kw_sec) {
        key = mp_obj_str_get_str(kw_key->value);
        key_len = strlen(key);

        sec = mp_obj_get_int(kw_sec->value);
        if (!IS_WLAN_SEC(sec)) {
            nlr_raise(mp_obj_new_exception_msg(
                        &mp_type_ValueError, "Invalid security mode"));
        }

    }

    // get bssid
    if (kw_bssid != NULL) {
        bssid = mp_obj_str_get_str(kw_bssid->value);
    }

    // connect to AP
    if (wlan_connect(sec, (char*) ssid, ssid_len, (uint8_t*)bssid, (uint8_t*)key, key_len) != 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "could not connect to ssid=%s, sec=%d, key=%s\n", ssid, sec, key));
    }

    return mp_const_none;
}

STATIC mp_obj_t modwlan_disconnect(void) {
    int ret = wlan_disconnect();
    return mp_obj_new_int(ret);
}

STATIC mp_obj_t modwlan_is_connected(void) {
    if (wlan_connected && ip_obtained) {
        return mp_const_true;
    }
    return mp_const_false;
}

STATIC mp_obj_t modwlan_ifconfig(void) {
    tNetappIpconfigRetArgs ipconfig={{0}};

    uint8_t *ip = &ipconfig.aucIP[0];
    uint8_t *mask= &ipconfig.aucSubnetMask[0];
    uint8_t *gw= &ipconfig.aucDefaultGateway[0];
    uint8_t *dhcp= &ipconfig.aucDHCPServer[0];
    uint8_t *dns= &ipconfig.aucDNSServer[0];
    uint8_t *mac= &ipconfig.uaMacAddr[0];
    uint8_t *ssid= &ipconfig.uaSSID[0];

    netapp_ipconfig(&ipconfig);

    printf ("IP:%d.%d.%d.%d\n"  \
            "Mask:%d.%d.%d.%d\n"\
            "GW:%d.%d.%d.%d\n"  \
            "DHCP:%d.%d.%d.%d\n"\
            "DNS:%d.%d.%d.%d\n" \
            "MAC:%02X:%02X:%02X:%02X:%02X:%02X\n"\
            "SSID: %s\n",
            ip[3], ip[2], ip[1], ip[0],
            mask[3], mask[2], mask[1], mask[0],
            gw[3], gw[2], gw[1], gw[0],
            dhcp[3], dhcp[2], dhcp[1], dhcp[0],
            dns[3], dns[2], dns[1], dns[0],
            mac[5], mac[4], mac[3], mac[2], mac[1], mac[0], ssid);
    return mp_const_none;
}

STATIC mp_obj_t modwlan_patch_version(void) {
    uint8_t pver[2];
    mp_obj_tuple_t *t_pver;

    nvmem_read_sp_version(pver);
    t_pver = mp_obj_new_tuple(2, NULL);
    t_pver->items[0] = mp_obj_new_int(pver[0]);
    t_pver->items[1] = mp_obj_new_int(pver[1]);
    return t_pver;
}

STATIC mp_obj_t modwlan_patch_program(void) {
    //patch_prog_start();
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(modwlan_init_obj, 4, 4, modwlan_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (modwlan_ifconfig_obj,     modwlan_ifconfig);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(modwlan_connect_obj, 1,   modwlan_connect);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (modwlan_disconnect_obj,   modwlan_disconnect);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (modwlan_is_connected_obj, modwlan_is_connected);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (modwlan_patch_version_obj,modwlan_patch_version);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (modwlan_patch_program_obj,modwlan_patch_program);

STATIC const mp_map_elem_t wlan_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_wlan) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&modwlan_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connect), (mp_obj_t)&modwlan_connect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disconnect), (mp_obj_t)&modwlan_disconnect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_connected), (mp_obj_t)&modwlan_is_connected_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ifconfig), (mp_obj_t)&modwlan_ifconfig_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_patch_version), (mp_obj_t)&modwlan_patch_version_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_patch_program), (mp_obj_t)&modwlan_patch_program_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_WEP), MP_OBJ_NEW_SMALL_INT(WLAN_SEC_WEP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WPA), MP_OBJ_NEW_SMALL_INT(WLAN_SEC_WPA) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WPA2), MP_OBJ_NEW_SMALL_INT(WLAN_SEC_WPA2) },
};

STATIC const mp_obj_dict_t wlan_module_globals = {
    .base = {&mp_type_dict},
    .map = {
        .all_keys_are_qstrs = 1,
        .table_is_fixed_array = 1,
        .used = MP_ARRAY_SIZE(wlan_module_globals_table),
        .alloc = MP_ARRAY_SIZE(wlan_module_globals_table),
        .table = (mp_map_elem_t*)wlan_module_globals_table,
    },
};

const mp_obj_module_t wlan_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_wlan,
    .globals = (mp_obj_dict_t*)&wlan_module_globals,
};
