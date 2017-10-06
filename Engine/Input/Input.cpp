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

Input::Input() {}

Input::~Input() {
	for (int i = 0; i < NUM_XBOXCONTROLLER; i++) {
		if (m_xboxControllers[i] != nullptr) {
			delete m_xboxControllers[i];
			m_xboxControllers[i] = nullptr;
		}
	}
}

void Input::onKeyDown(unsigned char keyCode) {
	if (!m_keyStates[keyCode].m_isDown) {
		m_keyStates[keyCode].m_justPressed = 1;
	}
	m_keyStates[keyCode].m_isDown = 1;
}

void Input::onKeyUp(unsigned char keyCode) {
	m_keyStates[keyCode].m_isDown = 0;
	m_keyStates[keyCode].m_justReleased = 1;
}

bool Input::isKeyDown(unsigned char keyCode) const {
	return m_keyStates[keyCode].m_isDown;
}

bool Input::isKeyUp(unsigned char keyCode) const {
	return !m_keyStates[keyCode].m_isDown;
}

bool Input::isKeyJustDown(unsigned char keyCode) const {
	return m_keyStates[keyCode].m_justPressed;
}

bool Input::isKeyJustUp(unsigned char keyCode) const {
	return m_keyStates[keyCode].m_justReleased;
}

void Input::beforeFrame() {
	updateKeyboard();
	updateXboxControllers();
	runMessagePump();
}

void Input::afterFrame() {
	
}

XboxController* Input::getController(XboxControllerID controllerId) {
	if (m_xboxControllers[controllerId] == nullptr) {
		m_xboxControllers[controllerId] = new XboxController(controllerId);
	}

	return m_xboxControllers[controllerId];
}

void Input::updateKeyboard() {
	for (int i = 0; i < NUM_KEY; i++) {
		m_keyStates[i].m_justPressed = 0;
		m_keyStates[i].m_justReleased = 0;
	}
}
             
void Input::updateXboxControllers() {
	for (int i = 0; i < NUM_XBOXCONTROLLER; i++) {
		if (m_xboxControllers[i] != nullptr) {
			m_xboxControllers[i]->updateControllerState();
		}
	}
}
