#pragma once
#include "Engine/Input/KeyState.hpp"
#include "Engine/Math/Vector2.hpp"

struct _XINPUT_STATE;
typedef _XINPUT_STATE XINPUT_STATE;
enum XboxKey {
	XBOX_KEY_DPAD_UP,
	XBOX_KEY_DPAD_DOWN,
	XBOX_KEY_DPAD_LEFT,
	XBOX_KEY_DPAD_RIGHT,
	XBOX_KEY_START,
	XBOX_KEY_BACK,
	XBOX_KEY_LEFT_THUMB,
	XBOX_KEY_RIGHT_THUMB,
	XBOX_KEY_LEFT_SHOULDER,
	XBOX_KEY_RIGHT_SHOULDER,
	XBOX_KEY_A,
	XBOX_KEY_B,
	XBOX_KEY_X,
	XBOX_KEY_Y,
	NUM_XBOX_KEYS
};

enum XBoxThumb {
	XOBX_THUMB_LEFT,
	XOBX_THUMB_RIGHT,
	NUM_XBOX_THUMB
};

static constexpr float THUMB_INNER_DEADZONE_FRACTION = 5200.f / 32768.f; // 23%
static constexpr float THUMB_OUTER_DEADZONE_FRACTION = 31000.f / 32768.f; // 90%

struct XboxThumbState {
	Vector2 position = Vector2(0.f, 0.f);
	float magnitude = 0;
	float angleDegree = 0;
};

static constexpr float TRIGGER_INNER_DEADZONE_FRACTION = 0.2f; // 23%
static constexpr float TRIGGER_OUTER_DEADZONE_FRACTION = 0.95f; // 90%
enum XboxTrigger {
	XBOX_TRIGGER_LEFT,
	XBOX_TRIGGER_RIGHT,
	NUM_XBOX_TRIGGER
};

enum XboxControllerID {
	XBOX_CONTROLLER_0,
	XBOX_CONTROLLER_1,
	XBOX_CONTROLLER_2,
	XBOX_CONTROLLER_3,
	NUM_XBOXCONTROLLER
};
class XboxController {
public:
	XboxController(XboxControllerID id);
	~XboxController() {};

	void updateControllerState();
	bool isKeyDown(XboxKey keyName);
	bool isKeyUp(XboxKey keyName);
	bool isKeyJustDown(XboxKey keyName);
	bool isKeyJustUp(XboxKey keyName);
	XboxThumbState getThumbState(XBoxThumb thumbId);
	float getTriggerState(XboxTrigger triggerId);
protected:
	KeyState m_keyStates[NUM_XBOX_KEYS];
	XboxThumbState m_thumbStates[NUM_XBOX_THUMB];
	float m_triggerStates[NUM_XBOX_TRIGGER];

private:
	XboxControllerID m_cotrollerID;
	void updateKeys(const XINPUT_STATE& xboxControllerState);
	void updateThumbs(const XINPUT_STATE& xboxControllerState);
	void updateTriggers(const XINPUT_STATE& xboxControllerState);
};
