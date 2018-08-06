
#ifndef DCFURS_H_INCLUDED
#define DCFURS_H_INCLUDED

#define DCF_DIMMING_STEPS   64
#define DCF_TOTAL_COLS      18
#define DCF_TOTAL_ROWS      7

mp_obj_t dcfurs_init(mp_obj_t timer);
mp_obj_t dcfurs_set_pixel(mp_obj_t xobj, mp_obj_t yobj, mp_obj_t vobj);
mp_obj_t dcfurs_set_row(mp_obj_t yobj, mp_obj_t rowdata);
mp_obj_t dcfurs_set_frame(mp_obj_t fbuf);
mp_obj_t dcfurs_has_pixel(mp_obj_t xobj, mp_obj_t yobj);
mp_obj_t dcfurs_clear(void);

mp_obj_t dcfurs_credits(void);
mp_obj_t dcfurs_login(void);
mp_obj_t dcfurs_beep(void);
mp_obj_t dcfurs_boop(void);
mp_obj_t dcfurs_eula(void);
mp_obj_t dcfurs_ctznos(void);
mp_obj_t dcfurs_awoo(void);

/* Draw an emote by making an external call to emote.py */
void dcfurs_emote(const char *str);

/* Simple PRNG. */
unsigned int dcfurs_rand(void);

#endif /* DCFURS_H_INCLUDED */
