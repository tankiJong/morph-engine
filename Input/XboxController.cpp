#include <Windows.h>
#include <Xinput.h> // include the Xinput API
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; use 9_1_0 explicitly for broadest compatibility

#include "Engine/Math/MathUtils.hpp"
#include "XboxController.hpp"

XboxController::XboxController(XboxControllerID id)
              : m_cotrollerID(id) {}

void XboxController::updateControllerState() {
	XINPUT_STATE xboxControllerState;
	memset(&xboxControllerState, 0, sizeof(xboxControllerState));
	//DWORD errorStatus = XInputGetState(m_cotrollerID, &xboxControllerState);
	XInputGetState(m_cotrollerID, &xboxControllerState); //QA: error handler?
	updateKeys(xboxControllerState);
	updateThumbs(xboxControllerState);
	updateTriggers(xboxControllerState);
}

bool XboxController::isKeyDown(XBOX_KEY keyName) {
	return m_keyStates[keyName].m_isDown;
}

bool XboxController::isKeyUp(XBOX_KEY keyName) {
	return !m_keyStates[keyName].m_isDown;
}

bool XboxController::isKeyJustDown(XBOX_KEY keyName) {
	return m_keyStates[keyName].m_justPressed;
}

bool XboxController::isKeyJustUp(XBOX_KEY keyName) {
	return m_keyStates[keyName].m_justReleased;
}

XBOX_THUMB_STATE XboxController::getThumbState(XBOX_THUMB thumbId) {
	return m_thumbStates[thumbId];
}

float XboxController::getTriggerState(XBOX_TRIGGER triggerId) {
	return m_triggerStates[triggerId];
}

void XboxController::updateKeys(const XINPUT_STATE& xboxControllerState) {
	unsigned short mask = 0x1;
	for (int i = 0; i < NUM_XBOX_KEYS; i++) {
		m_keyStates[i].m_justPressed = 0;
		m_keyStates[i].m_justReleased = 0;
		if ((xboxControllerState.Gamepad.wButtons & mask) == mask) {
			if (!m_keyStates[i].m_isDown) {
				m_keyStates[i].m_justPressed = 1;
			}
			m_keyStates[i].m_isDown = 1;
		} else {
			m_keyStates[i].m_isDown = 0;
			m_keyStates[i].m_justReleased = 1;
		}
		mask <<= 1;
		if (mask == 0x0400) mask = 0x1000; // there is a gap, the 11, 12 bits are not used
	}
}

void XboxController::updateThumbs(const XINPUT_STATE& xboxControllerState) {
	{
		// Left
		float x = (float) xboxControllerState.Gamepad.sThumbLX,
			y = (float) xboxControllerState.Gamepad.sThumbLY;

		x = rangeMapf(x, -32768.f, 32767.f, -1.f, 1.f);
		y = rangeMapf(y, -32768.f, 32767.f, -1.f, 1.f);

		float magnitude = sqrtf(x*x + y*y), theta = atan2Degree(y, x);
		magnitude = rangeMapf(magnitude
							  , THUMB_INNER_DEADZONE_FRACTION
							  , THUMB_OUTER_DEADZONE_FRACTION
							  , 0.f, 1.f);
		m_thumbStates[XOBX_THUMB_LEFT].magnitude = clampf01(magnitude);
		m_thumbStates[XOBX_THUMB_LEFT].angleDegree = theta;
		m_thumbStates[XOBX_THUMB_LEFT].position = Vector2(cosDegrees(theta) * magnitude, sinDegrees(theta) * magnitude);
	} {
		// Right
		float x = (float) xboxControllerState.Gamepad.sThumbRX,
			y = (float) xboxControllerState.Gamepad.sThumbRY;

		x = rangeMapf(x, -32768.f, 32767.f, -1.f, 1.f);
		y = rangeMapf(y, -32768.f, 32767.f, -1.f, 1.f);

		float magnitude = sqrtf(x*x + y*y), theta = atan2Degree(y, x);
		magnitude = rangeMapf(magnitude
							  , THUMB_INNER_DEADZONE_FRACTION
							  , THUMB_OUTER_DEADZONE_FRACTION
							  , 0.f, 1.f);
		m_thumbStates[XOBX_THUMB_RIGHT].magnitude = clampf01(magnitude);
		m_thumbStates[XOBX_THUMB_RIGHT].angleDegree = theta;
		m_thumbStates[XOBX_THUMB_RIGHT].position = Vector2(cosDegrees(theta) * magnitude, sinDegrees(theta) * magnitude);
	}
	
}

void XboxController::updateTriggers(const XINPUT_STATE& xboxControllerState) {
	{
		// Left
		float trigger = xboxControllerState.Gamepad.bLeftTrigger;
		m_triggerStates[XBOX_TRIGGER_LEFT] = rangeMapf(trigger, 0.f, 255.f, 0.f, 1.f);
		//m_triggerStates[XBOX_TRIGGER_LEFT] = rangeMapf(normalizedTrigger
		//								   , TRIGGER_INNER_DEADZONE_FRACTION
		//								   , TRIGGER_OUTER_DEADZONE_FRACTION
		//								   , 0.f, 1.f);

	}
	{
		// Right
		float trigger = xboxControllerState.Gamepad.bRightTrigger;
		m_triggerStates[XBOX_TRIGGER_RIGHT] = rangeMapf(trigger, 0.f, 255.f, 0.f, 1.f);
		//m_triggerStates[XBOX_TRIGGER_RIGHT] = rangeMapf(normalizedTrigger
		//											 , TRIGGER_INNER_DEADZONE_FRACTION
		//											 , TRIGGER_OUTER_DEADZONE_FRACTION
		//											 , 0.f, 1.f);

	}
}
