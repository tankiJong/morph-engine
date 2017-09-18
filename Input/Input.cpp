#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

#include "Input.hpp"

void runMessagePump() {
	MSG queuedMessage;
	for (;; ) {
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent) {
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage);
	}
}

void Input::onKeyDown(unsigned char keyCode) {
	if (!keyStates[keyCode].m_isDown) {
		keyStates[keyCode].m_justPressed = 1;
	}
	keyStates[keyCode].m_isDown = 1;
}

void Input::onKeyUp(unsigned char keyCode) {
	keyStates[keyCode].m_isDown = 0;
	keyStates[keyCode].m_justReleased = 1;
}

bool Input::isKeyDown(unsigned char keyCode) {
	return keyStates[keyCode].m_isDown;
}

bool Input::isKeyUp(unsigned char keyCode) {
	return !keyStates[keyCode].m_isDown;
}

bool Input::isKeyJustDown(unsigned char keyCode) {
	return keyStates[keyCode].m_justPressed;
}

bool Input::isKeyJustUp(unsigned char keyCode) {
	return keyStates[keyCode].m_justReleased;
}

void Input::beforeFrame() {
	updateKeyboard();
	updateControllers();
	runMessagePump();
}

void Input::afterFrame() {
	
}

void Input::updateKeyboard() {
	for (int i = 0; i < NUM_KEY; i++) {
		keyStates[i].m_justPressed = 0;
		keyStates[i].m_justReleased = 0;
	}
}
             
void Input::updateControllers() {}
