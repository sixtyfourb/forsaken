/*
 * Touch and gamepad input, as a flat C API.
 *
 * This is the only file on the engine side that knows what a SHIPCONTROL is,
 * and this header deliberately mentions no engine type at all - the OpenTouch
 * glue includes it and must not need a Forsaken header to compile. The same
 * arrangement as dxx-redux's android/touch_input.h, for the same reason.
 *
 * Axes are deflections in -1..1. Absolute axes are a stick held somewhere and
 * persist until they change; relative axes are a swipe or the gyro and are
 * consumed by the frame that reads them, so a finger lifted mid-drag does not
 * leave the ship turning.
 */

#ifndef FSK_TOUCH_INPUT_H
#define FSK_TOUCH_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Digital actions. Named for what the player does, not for the key it used to
 * be, so the glue never has to know Forsaken's own action numbering. */
enum {
	FSK_TOUCH_FIRE_PRIMARY = 1,
	FSK_TOUCH_FIRE_SECONDARY,
	FSK_TOUCH_DROP_MINE,
	FSK_TOUCH_TURBO,
	FSK_TOUCH_SLIDE_MODE,
	FSK_TOUCH_ROLL_MODE,
	FSK_TOUCH_NEXT_PRIMARY,
	FSK_TOUCH_PREV_PRIMARY,
	FSK_TOUCH_NEXT_SECONDARY,
	FSK_TOUCH_PREV_SECONDARY,
	FSK_TOUCH_CRUISE_UP,
	FSK_TOUCH_CRUISE_DOWN
};

/* Absolute axes - a stick. */
void fsk_touch_axis_pitch(float v);
void fsk_touch_axis_yaw(float v);
void fsk_touch_axis_roll(float v);
void fsk_touch_axis_forward(float v);
void fsk_touch_axis_sideways(float v);
void fsk_touch_axis_vertical(float v);

/* Relative axes - a swipe or the gyro. Accumulated, then consumed. */
void fsk_touch_delta_pitch(float v);
void fsk_touch_delta_yaw(float v);

/* Digital actions: state is 1 for pressed, 0 for released. */
void fsk_touch_action(int action, int state);

/*
 * Fold the above into the ship's controls. Called from control_ship() once the
 * per-frame maxima are known and immediately before it clamps, so touch input
 * is bounded exactly as keyboard, mouse and joystick input already are.
 *
 * ctrl is a SHIPCONTROL *, passed as void * to keep this header free of engine
 * types. framelag is the engine's own frame scale: DoShipAction multiplies by
 * TurnAccell * MaxTurnSpeed, and the clamp is that times framelag, so a stick at
 * full deflection wants to contribute exactly framelag.
 */
void fsk_touch_apply_controls(void *ctrl, float framelag);

/*
 * Which control layout the overlay should show. Mirrors the touchscreemode_t the
 * glue expects: 0 menu, 1 game.
 */
int fsk_touch_screen_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* FSK_TOUCH_INPUT_H */
