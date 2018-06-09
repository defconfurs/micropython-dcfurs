
#ifndef DCFURS_H_INCLUDED
#define DCFURS_H_INCLUDED

#define DCF_DIMMING_STEPS   16
#define DCF_TOTAL_COLS      18
#define DCF_TOTAL_ROWS      7

mp_obj_t dcfurs_matrix_init(void);
mp_obj_t dcfurs_matrix_loop(size_t n_args, const mp_obj_t *args);
mp_obj_t dcfurs_set_pixel(mp_obj_t xobj, mp_obj_t yobj, mp_obj_t vobj);
mp_obj_t dcfurs_set_row(mp_obj_t yobj, mp_obj_t rowdata);
mp_obj_t dcfurs_set_frame(mp_obj_t fbuf);
mp_obj_t dcfurs_has_pixel(mp_obj_t xobj, mp_obj_t yobj);
mp_obj_t dcfurs_clear(void);

mp_obj_t dcfurs_credits(void);

#endif /* DCFURS_H_INCLUDED */
