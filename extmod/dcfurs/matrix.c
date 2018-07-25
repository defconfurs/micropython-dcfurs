#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "ports/stm32/timer.h"

#include "dcfurs.h"

#define DCF_PIN_ROW_BANKB   0xF403
#define DCF_PIN_COL_BANKA   0x01FE
#define DCF_PIN_COL_BANKC   0x03FF

#define DCF_BITBAND_SRAM(_addr_, _bit_) \
    ((uint32_t *)SRAM1_BB_BASE)[(((unsigned long)_addr_) - SRAM1_BASE) * 8 + (_bit_)]

struct dcf_rowdata {
    uint32_t rowstrobe; /* Port B register value to activate the row. */
    uint32_t pxenable;  /* Pixels that are enabled. */
    uint16_t pxbankA[DCF_DIMMING_STEPS];
    uint16_t pxbankC[DCF_DIMMING_STEPS];
};

struct dcf_framebuf {
    unsigned int current_row;
    unsigned int current_step;
    struct dcf_rowdata rows[DCF_TOTAL_ROWS];
    DMA_HandleTypeDef dma_gpioA;
    DMA_HandleTypeDef dma_gpioC;
};

static struct dcf_framebuf dcf_fb = {0};

/* DMA Descriptors for blitting row data directly to the GPIO registers. */
static const DMA_InitTypeDef dma_init_blit_gpio = {
    .Channel             = DMA_CHANNEL_6,
    .Direction           = DMA_MEMORY_TO_PERIPH,
    .PeriphInc           = DMA_PINC_DISABLE,
    .MemInc              = DMA_MINC_ENABLE,
    .PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD,
    .MemDataAlignment    = DMA_MDATAALIGN_HALFWORD,
    .Mode                = DMA_NORMAL,
    .Priority            = DMA_PRIORITY_HIGH,
    .FIFOMode            = DMA_FIFOMODE_DISABLE,
    .FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL,
    .MemBurst            = DMA_MBURST_SINGLE,
    .PeriphBurst         = DMA_MBURST_SINGLE
};

/* Callback function to drive the matrix display. */
mp_obj_t dcfurs_matrix_loop(size_t n_args, const mp_obj_t *args)
{
    /* Increment the current row to blit. */
    struct dcf_rowdata *row;
    dcf_fb.current_row++;
    if (dcf_fb.current_row >= DCF_TOTAL_ROWS) dcf_fb.current_row = 0;
    row = &dcf_fb.rows[dcf_fb.current_row];

    /* Disable the last row and its DMA transaction. */
    GPIOB->BSRR = DCF_PIN_ROW_BANKB;
    GPIOA->BSRR = DCF_PIN_COL_BANKA;
    GPIOC->BSRR = DCF_PIN_COL_BANKC;
    HAL_DMA_Abort(&dcf_fb.dma_gpioA);
    HAL_DMA_Abort(&dcf_fb.dma_gpioC);

    /* Select the next row and start its DMA transaction. */
    GPIOA->BSRR = (row->pxenable & 0x3fc00) << 7;
    GPIOC->BSRR = (row->pxenable & 0x3ff) << 16;
    GPIOB->BSRR = row->rowstrobe;
    TIM1->DIER &= ~(TIM_DMA_CC1 | TIM_DMA_CC2);
    HAL_DMA_Start(&dcf_fb.dma_gpioA, (uint32_t)&row->pxbankA, (uint32_t)&GPIOA->BSRR, DCF_DIMMING_STEPS);
    HAL_DMA_Start(&dcf_fb.dma_gpioC, (uint32_t)&row->pxbankC, (uint32_t)&GPIOC->BSRR, DCF_DIMMING_STEPS);
    TIM1->DIER |= (TIM_DMA_CC1 | TIM_DMA_CC2);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(dcfurs_matrix_loop_obj, 0, dcfurs_matrix_loop);

/* The fastest we can do to set a single pixel PWM intensity */
static inline void
dcf_set_pwm(struct dcf_rowdata *row, int x, int pwm)
{
    int i;
    int step = ((unsigned int)pwm * DCF_DIMMING_STEPS + 128) / 256;
    for (i = DCF_DIMMING_STEPS-1; i >= 0; i--) {
        if (x < 10) {
            DCF_BITBAND_SRAM(&row->pxbankC[i], x) = (i >= step);
        } else {
            DCF_BITBAND_SRAM(&row->pxbankA[i], x-9) = (i >= step);
        }
    }
    DCF_BITBAND_SRAM(&row->pxenable, x) = (pwm != 0);
}

mp_obj_t dcfurs_init(mp_obj_t timer) {
    TIM_HandleTypeDef       *htim = pyb_timer_get_handle(timer);
    GPIO_InitTypeDef        gpio;

    mp_obj_t timer_channel_args[5] = {
        0,  /* Function object */
        0,  /* Timer object */
        0,  /* Timer channel number */
        MP_ROM_QSTR(MP_QSTR_mode), mp_load_attr(timer, MP_QSTR_OC_TIMING)
    };
    mp_obj_t timer_callback_args[3] = {
        0,  /* Function object */
        0,  /* Timer object */
        (mp_obj_t)&dcfurs_matrix_loop_obj
    };

    /* Prepare row driver outputs */
    gpio.Speed = GPIO_SPEED_FAST;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Alternate = 0;
    gpio.Pin = DCF_PIN_ROW_BANKB;
    HAL_GPIO_Init(GPIOB, &gpio);
    GPIOB->BSRR = DCF_PIN_ROW_BANKB;

    /* Prepare column driver outputs (group A) */
    gpio.Speed = GPIO_SPEED_FAST;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Alternate = 0;
    gpio.Pin = DCF_PIN_COL_BANKA;
    HAL_GPIO_Init(GPIOA, &gpio);
    GPIOA->BSRR = DCF_PIN_COL_BANKA;

    /* Prepare column driver outputs (group A) */
    gpio.Speed = GPIO_SPEED_FAST;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Alternate = 0;
    gpio.Pin = DCF_PIN_COL_BANKC;
    HAL_GPIO_Init(GPIOC, &gpio);
    GPIOC->BSRR = DCF_PIN_COL_BANKC;

    /*  Initialize the DMA channels  */
    __HAL_RCC_DMA2_CLK_ENABLE();

    dcf_fb.dma_gpioA.Instance = DMA2_Stream1;
    memcpy(&dcf_fb.dma_gpioA.Init, &dma_init_blit_gpio, sizeof(DMA_InitTypeDef));
    HAL_DMA_DeInit(&dcf_fb.dma_gpioA);
    HAL_DMA_Init(&dcf_fb.dma_gpioA);

    dcf_fb.dma_gpioC.Instance = DMA2_Stream2;
    memcpy(&dcf_fb.dma_gpioC.Init, &dma_init_blit_gpio, sizeof(DMA_InitTypeDef));
    HAL_DMA_DeInit(&dcf_fb.dma_gpioC);
    HAL_DMA_Init(&dcf_fb.dma_gpioC);

    /* Configure the timer channels. */
    timer_channel_args[2] = MP_OBJ_NEW_SMALL_INT(1);
    mp_load_method(timer, MP_QSTR_channel, timer_channel_args);
    mp_call_method_n_kw(1, 1, timer_channel_args);

    timer_channel_args[2] = MP_OBJ_NEW_SMALL_INT(2);
    mp_load_method(timer, MP_QSTR_channel, timer_channel_args);
    mp_call_method_n_kw(1, 1, timer_channel_args);

    /* Install the timer update handler to switch rows. */
    htim->Instance->RCR = DCF_DIMMING_STEPS+8;
    htim->Init.RepetitionCounter = DCF_DIMMING_STEPS+8;
    mp_load_method(timer, MP_QSTR_callback, timer_callback_args);
    mp_call_method_n_kw(1, 0, timer_callback_args);

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

mp_obj_t dcfurs_set_row(mp_obj_t yobj, mp_obj_t data)
{
    struct dcf_rowdata *row;
    int y = mp_obj_get_int(yobj);
    mp_buffer_info_t bufinfo;
    if ((y < 0) || (y >= DCF_TOTAL_ROWS)) {
        return mp_const_none; /* TODO: Throw a range error or something? */
    }
    row = &dcf_fb.rows[y];
    
    /* If an integer was provided, use it as a bitmap of pixels to switch on. */
    if (mp_obj_is_integer(data)) {
        row->pxenable = 0;
        memset(row->pxbankA, 0, sizeof(row->pxbankA));
        memset(row->pxbankC, 0, sizeof(row->pxbankC));
        row->pxenable = mp_obj_get_int(data);
    }
    /* If a bytearray was provided, use it as an array of pixel intensities. */
    else if (mp_get_buffer(data, &bufinfo, MP_BUFFER_READ)) {
        int x;
        const uint8_t *px = bufinfo.buf;
        if (bufinfo.len > DCF_TOTAL_COLS) {
            bufinfo.len = DCF_TOTAL_COLS;
        }

        //row->pxenable = 0;
        for (x = 0; x < bufinfo.len; x++) {
            dcf_set_pwm(row, x, px[x]);
        }
    }
    return mp_const_none;
}

mp_obj_t dcfurs_set_pixel(mp_obj_t xobj, mp_obj_t yobj, mp_obj_t vobj)
{
    struct dcf_rowdata *row;
    int x = mp_obj_get_int(xobj);
    int y = mp_obj_get_int(yobj);

    /* Sanity */
    if ((y < 0) || (y >= DCF_TOTAL_ROWS)) {
        return mp_const_none;
    }
    if ((x < 0) || (x >= DCF_TOTAL_COLS)) {
        return mp_const_none;
    }

    /* Update the pixel dimming value. */
    row = &dcf_fb.rows[y];
    //row->pxenable &= ~(1 << x);
    if (mp_obj_is_integer(vobj)) {
        dcf_set_pwm(row, x, mp_obj_get_int(vobj));
    }
    else if (mp_obj_is_true(vobj)) {
        dcf_set_pwm(row, x, 256);
    }
    return mp_const_none;
}

mp_obj_t dcfurs_set_frame(mp_obj_t fbobj)
{
    mp_obj_t *rowdata;
    mp_obj_get_array_fixed_n(fbobj, DCF_TOTAL_ROWS, &rowdata);
    int y;
    for (y = 0; y < DCF_TOTAL_ROWS; y++) {
        dcfurs_set_row(MP_OBJ_NEW_SMALL_INT(y), rowdata[y]);
    }
    return mp_const_none;
}

mp_obj_t dcfurs_has_pixel(mp_obj_t xobj, mp_obj_t yobj)
{
    int x = mp_obj_get_int(xobj);
    int y = mp_obj_get_int(yobj);
    if ((x < 0) || (x >= DCF_TOTAL_COLS)) {
        return mp_const_false;
    }
    if ((y < 0) || (y >= DCF_TOTAL_ROWS)) {
        return mp_const_false;
    }

    /* Corners */
    if ((x == 0) || (x == (DCF_TOTAL_COLS-1))) {
        if ((y == 0) || (y==(DCF_TOTAL_ROWS-1))) {
            return mp_const_false;
        }
    }

    /* Bridge of the nose */
    if (y == (DCF_TOTAL_ROWS-2)) {
        if ((x > 6) && (x < 11)) {
            return mp_const_false;
        }
    }
    if (y == (DCF_TOTAL_ROWS-1)) {
        if ((x > 5) && (x < 12)) {
            return mp_const_false;
        }
    }

    /* Otherwise, the pixel exists. */
    return mp_const_true;
}

mp_obj_t dcfurs_clear(void)
{
    int i;
    for (i = 0; i < DCF_TOTAL_ROWS; i++) {
        dcf_fb.rows[i].pxenable = 0;
    }
    return mp_const_none;
}
