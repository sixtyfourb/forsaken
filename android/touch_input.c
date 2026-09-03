/*
 * Touch and gamepad input for the Android port.
 *
 * The only file on the engine side that knows what a SHIPCONTROL is. Everything
 * the OpenTouch glue needs is in touch_input.h, which mentions no engine type,
 * so the glue never includes a Forsaken header.
 *
 * Input is applied through the engine's own DoShipAction() rather than by
 * writing SHIPCONTROL fields directly. That matters: DoShipAction knows that a
 * rotation becomes a slide when slide_mode is held, and a roll when roll_mode
 * is, so touch and pad inherit those modes for free instead of reimplementing
 * them and drifting apart.
 */

#include "touch_input.h"

#include "main.h"
#include "controls.h"

extern BYTE MyGameStatus;

/* Absolute axes: a stick, held where the finger left it. */
static float axis_pitch;
static float axis_yaw;
static float axis_roll;
static float axis_forward;
static float axis_sideways;
static float axis_vertical;

/* Relative axes: a swipe or the gyro, consumed by the frame that reads them. */
static float delta_pitch;
static float delta_yaw;

/* Digital actions. */
static int act_fire_primary;
static int act_fire_secondary;
static int act_drop_mine;
static int act_turbo;
static int act_slide_mode;
static int act_roll_mode;
static int act_next_primary;
static int act_prev_primary;
static int act_next_secondary;
static int act_prev_secondary;
static int act_cruise_up;
static int act_cruise_down;

void fsk_touch_axis_pitch(float v)    { axis_pitch = v; }
void fsk_touch_axis_yaw(float v)      { axis_yaw = v; }
void fsk_touch_axis_roll(float v)     { axis_roll = v; }
void fsk_touch_axis_forward(float v)  { axis_forward = v; }
void fsk_touch_axis_sideways(float v) { axis_sideways = v; }
void fsk_touch_axis_vertical(float v) { axis_vertical = v; }

void fsk_touch_delta_pitch(float v) { delta_pitch += v; }
void fsk_touch_delta_yaw(float v)   { delta_yaw += v; }

void fsk_touch_action(int action, int state)
{
	switch (action)
	{
	case FSK_TOUCH_FIRE_PRIMARY:   act_fire_primary   = state; break;
	case FSK_TOUCH_FIRE_SECONDARY: act_fire_secondary = state; break;
	case FSK_TOUCH_DROP_MINE:      act_drop_mine      = state; break;
	case FSK_TOUCH_TURBO:          act_turbo          = state; break;
	case FSK_TOUCH_SLIDE_MODE:     act_slide_mode     = state; break;
	case FSK_TOUCH_ROLL_MODE:      act_roll_mode      = state; break;
	case FSK_TOUCH_NEXT_PRIMARY:   act_next_primary   = state; break;
	case FSK_TOUCH_PREV_PRIMARY:   act_prev_primary   = state; break;
	case FSK_TOUCH_NEXT_SECONDARY: act_next_secondary = state; break;
	case FSK_TOUCH_PREV_SECONDARY: act_prev_secondary = state; break;
	case FSK_TOUCH_CRUISE_UP:      act_cruise_up      = state; break;
	case FSK_TOUCH_CRUISE_DOWN:    act_cruise_down    = state; break;
	default: break;
	}
}

void fsk_touch_apply_controls(void *ctrl_, float framelag)
{
	SHIPCONTROL *ctrl = (SHIPCONTROL *) ctrl_;

	if (!ctrl)
		return;

	/*
	 * The modes go on first, because DoShipAction reads them: with slide_mode
	 * held a rotation becomes a slide, and with roll_mode a roll. Setting them
	 * after the axes would apply this frame's stick to last frame's mode.
	 */
	if (act_slide_mode) ctrl->slide_mode = 1;
	if (act_roll_mode)  ctrl->roll_mode = 1;
	if (act_turbo)      ctrl->turbo = 1;

	/*
	 * amount is in units of framelag. DoShipAction multiplies by
	 * TurnAccell * MaxTurnSpeed and control_ship clamps to that times framelag,
	 * so a stick at full deflection contributing framelag lands exactly on the
	 * clamp - the same arrangement the joystick path uses, which passes
	 * framelag * axis * sensitivity.
	 */
	if (axis_pitch != 0.0f)
		DoShipAction(ctrl, axis_pitch > 0.0f ? SHIPACTION_RotateUp : SHIPACTION_RotateDown,
		             framelag * (axis_pitch > 0.0f ? axis_pitch : -axis_pitch));

	if (axis_yaw != 0.0f)
		DoShipAction(ctrl, axis_yaw > 0.0f ? SHIPACTION_RotateRight : SHIPACTION_RotateLeft,
		             framelag * (axis_yaw > 0.0f ? axis_yaw : -axis_yaw));

	if (axis_roll != 0.0f)
		DoShipAction(ctrl, axis_roll > 0.0f ? SHIPACTION_RollRight : SHIPACTION_RollLeft,
		             framelag * (axis_roll > 0.0f ? axis_roll : -axis_roll));

	if (axis_forward != 0.0f)
		DoShipAction(ctrl, axis_forward > 0.0f ? SHIPACTION_MoveForward : SHIPACTION_MoveBack,
		             framelag * (axis_forward > 0.0f ? axis_forward : -axis_forward));

	if (axis_sideways != 0.0f)
		DoShipAction(ctrl, axis_sideways > 0.0f ? SHIPACTION_SlideRight : SHIPACTION_SlideLeft,
		             framelag * (axis_sideways > 0.0f ? axis_sideways : -axis_sideways));

	if (axis_vertical != 0.0f)
		DoShipAction(ctrl, axis_vertical > 0.0f ? SHIPACTION_SlideUp : SHIPACTION_SlideDown,
		             framelag * (axis_vertical > 0.0f ? axis_vertical : -axis_vertical));

	/*
	 * Relative axes are a distance already travelled, not a rate, so they do not
	 * scale by framelag - a swipe of a given length should turn the ship by the
	 * same amount however long the frame took. Consumed here so a finger lifted
	 * mid-drag does not leave the ship turning.
	 */
	if (delta_pitch != 0.0f)
	{
		DoShipAction(ctrl, delta_pitch > 0.0f ? SHIPACTION_RotateUp : SHIPACTION_RotateDown,
		             delta_pitch > 0.0f ? delta_pitch : -delta_pitch);
		delta_pitch = 0.0f;
	}

	if (delta_yaw != 0.0f)
	{
		DoShipAction(ctrl, delta_yaw > 0.0f ? SHIPACTION_RotateRight : SHIPACTION_RotateLeft,
		             delta_yaw > 0.0f ? delta_yaw : -delta_yaw);
		delta_yaw = 0.0f;
	}

	/* Weapons and the rest are flags the engine reads once a frame. */
	if (act_fire_primary)   ctrl->fire_primary = 1;
	if (act_fire_secondary) ctrl->fire_secondary = 1;
	if (act_drop_mine)      ctrl->fire_mine = 1;

	if (act_next_primary)   ctrl->select_next_primary = 1;
	if (act_prev_primary)   ctrl->select_prev_primary = 1;
	if (act_next_secondary) ctrl->select_next_secondary = 1;
	if (act_prev_secondary) ctrl->select_prev_secondary = 1;

	if (act_cruise_up)   DoShipAction(ctrl, SHIPACTION_CruiseIncrease, framelag);
	if (act_cruise_down) DoShipAction(ctrl, SHIPACTION_CruiseDecrease, framelag);
}

int fsk_touch_screen_mode(void)
{
	/*
	 * Flight controls only while actually flying. Everything else - the title
	 * screen, the multiplayer lobby, loading a level, viewing the score - wants
	 * the menu layout, which is a pointer and a few keys.
	 */
	switch (MyGameStatus)
	{
	case STATUS_Normal:
	case STATUS_SinglePlayer:
	case STATUS_PlayingDemo:
		return 1;   /* in game */
	default:
		return 0;   /* menu */
	}
}
