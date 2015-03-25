/*
 * internal mehstation button codes.
 */

#pragma once

/* Input */

#define MEH_INPUT_BUTTON_UP 0
#define MEH_INPUT_BUTTON_DOWN 1
#define MEH_INPUT_BUTTON_LEFT 2
#define MEH_INPUT_BUTTON_RIGHT 3
#define MEH_INPUT_BUTTON_A 4
#define MEH_INPUT_BUTTON_B 5
#define MEH_INPUT_BUTTON_L 6
#define MEH_INPUT_BUTTON_R 7
#define MEH_INPUT_BUTTON_START 8
#define MEH_INPUT_BUTTON_SELECT 9

#define MEH_INPUT_SPECIAL_ESCAPE 10
#define MEH_INPUT_SPECIAL_UNKNOWN 11

#define MEH_INPUT_END 12

/* Event */

#define MEH_MSG_BUTTON_PRESSED 0
#define MEH_MSG_UPDATE 1
#define MEH_MSG_RENDER 2
#define MEH_MSG_END 3

/*
 * We fake a resolution while drawing into a Screen
 * for simple representation.
 */

#define MEH_FAKE_WIDTH 1280.0f
#define MEH_FAKE_HEIGHT 720.0f

