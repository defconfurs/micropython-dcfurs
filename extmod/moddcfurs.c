
#include <stdio.h>
#include <string.h>

#include "py/runtime.h"

#if MICROPY_PY_DCFURS

#define DCF_DIMMING_STEPS   16
#define DCF_TOTAL_ROWS      7

#define DCF_PIN_ROW_BANKB   0xF403
#define DCF_PIN_COL_BANKA   0x01FE
#define DCF_PIN_COL_BANKC   0x03FF

struct dcf_rowdata {
    uint32_t rowstrobe; /* Port B register value to activate the row. */   
    uint32_t pxdata[DCF_DIMMING_STEPS];
};

struct dcf_framebuf {
    unsigned int current_row;
    unsigned int current_step;
    struct dcf_rowdata rows[DCF_TOTAL_ROWS];
};

static struct dcf_framebuf dcf_fb = {0};

STATIC mp_obj_t dcfurs_matrix_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Prepare row driver outputs */
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Alternate = 0;
    GPIO_InitStructure.Pin = DCF_PIN_ROW_BANKB;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIOB->BSRR = DCF_PIN_ROW_BANKB;

    /* Prepare column driver outputs (group A) */
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Alternate = 0;
    GPIO_InitStructure.Pin = DCF_PIN_COL_BANKA;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIOA->BSRR = DCF_PIN_COL_BANKA;

    /* Prepare column driver outputs (group A) */
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Alternate = 0;
    GPIO_InitStructure.Pin = DCF_PIN_COL_BANKC;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIOC->BSRR = DCF_PIN_COL_BANKC;
    
    /* Setup the row driver data. */
    dcf_fb.rows[0].rowstrobe = (GPIO_PIN_0 << 16);
    dcf_fb.rows[1].rowstrobe = (GPIO_PIN_1 << 16);
    dcf_fb.rows[2].rowstrobe = (GPIO_PIN_10 << 16);
    dcf_fb.rows[3].rowstrobe = (GPIO_PIN_12 << 16);
    dcf_fb.rows[4].rowstrobe = (GPIO_PIN_13 << 16);
    dcf_fb.rows[5].rowstrobe = (GPIO_PIN_14 << 16);
    dcf_fb.rows[6].rowstrobe = (GPIO_PIN_15 << 16);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(dcfurs_matrix_init_obj, dcfurs_matrix_init);

STATIC mp_obj_t dcfurs_matrix_loop(size_t n_args, const mp_obj_t *args)
{
    /* Increment step, and blit the next dimming values */
    dcf_fb.current_step++;
    if (dcf_fb.current_step >= DCF_DIMMING_STEPS) {
        struct dcf_rowdata *row;

        /* Increment row data. */
        dcf_fb.current_row++;
        dcf_fb.current_step = 0;
        if (dcf_fb.current_row >= DCF_TOTAL_ROWS) dcf_fb.current_row = 0;

        /* Blit the starting pixels in the row. */
        row = &dcf_fb.rows[dcf_fb.current_row];
        GPIOB->BSRR = DCF_PIN_ROW_BANKB;
        GPIOA->BSRR = DCF_PIN_COL_BANKA;
        GPIOC->BSRR = DCF_PIN_COL_BANKC;
        GPIOA->BSRR = (row->pxdata[0] & 0x3fc00) << 7;
        GPIOC->BSRR = (row->pxdata[0] & 0x3ff) << 16;
        GPIOB->BSRR = row->rowstrobe;
    }
    /* Switch pixels off when they reach their dimming level. */
    else {
        uint32_t pxdata = dcf_fb.rows[dcf_fb.current_row].pxdata[dcf_fb.current_step];
        GPIOA->BSRR = (pxdata & 0x3fc00) >> 9;
        GPIOC->BSRR = (pxdata & 0x3ff) >> 0;
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(dcfurs_matrix_loop_obj, 0, dcfurs_matrix_loop);

STATIC mp_obj_t dcfurs_set_row(mp_obj_t yobj, mp_obj_t bitmap)
{
    struct dcf_rowdata *row;
    int y = mp_obj_get_int(yobj);
    int step;
    if ((y < 0) || (y >= DCF_TOTAL_ROWS)) {
        return mp_const_none; /* TODO: Throw a range error or something? */
    }
    
    /* Set the row's pixels */
    row = &dcf_fb.rows[y];
    for (step = 0; step < DCF_DIMMING_STEPS; step++) {
        row->pxdata[step] = 0;
    }
    row->pxdata[0] = mp_obj_get_int(bitmap);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(dcfurs_set_row_obj, dcfurs_set_row);

STATIC mp_obj_t dcfurs_set_pixel(mp_obj_t xobj, mp_obj_t yobj, mp_obj_t vobj)
{
    struct dcf_rowdata *row;
    int x = mp_obj_get_int(xobj);
    int y = mp_obj_get_int(yobj);
    int step;
    if ((y < 0) || (y >= DCF_TOTAL_ROWS)) {
        return mp_const_none; /* TODO: Throw a range error or something? */
    }

    /* Update the pixel dimming value. */
    row = &dcf_fb.rows[y];
    for (step = 0; step < DCF_DIMMING_STEPS; step++) {
        row->pxdata[step] &= ~(1 << x); /* Disable the pixel as we update it. */
    }
    if (mp_obj_is_true(vobj)) {
        row->pxdata[0] |= (1 << x);     /* Enable full intensity. */
    }

    /* Set a dimming intensity by configuring at which step we turn the pixel off. */
    if (mp_obj_is_integer(vobj)) {
        step = (mp_obj_get_int(vobj) * DCF_DIMMING_STEPS + 128) / 256;
        if (step == 0) step = 1;
        if (step < DCF_DIMMING_STEPS) {
            row->pxdata[step] |= (1 << x);
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(dcfurs_set_pixel_obj, dcfurs_set_pixel);

STATIC mp_obj_t dcfurs_clear(void)
{
    int i;
    for (i = 0; i < DCF_TOTAL_ROWS; i++) {
        memset(dcf_fb.rows[i].pxdata, 0, sizeof(dcf_fb.rows[i].pxdata));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(dcfurs_clear_obj, dcfurs_clear);

STATIC const mp_rom_map_elem_t mp_module_dcfurs_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_dcfurs) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_matrix_init), (mp_obj_t)&dcfurs_matrix_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_matrix_loop), (mp_obj_t)&dcfurs_matrix_loop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_row), (mp_obj_t)&dcfurs_set_row_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pixel), (mp_obj_t)&dcfurs_set_pixel_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_clear), (mp_obj_t)&dcfurs_clear_obj },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_dcfurs_globals, mp_module_dcfurs_globals_table);

const mp_obj_module_t mp_module_dcfurs = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_dcfurs_globals,
};

#endif // MICROPY_PY_DCFURS
