#include "Input.hpp"


void Input::onKeyDown(unsigned char keyCode) {
	keyStates[keyCode].m_isDown = 1;
}

void Input::onKeyUp(unsigned char keyCode) {
	keyStates[keyCode].m_isDown = 0;
}

bool Input::isKeyDown(unsigned char keyCode) {
	return keyStates[keyCode].m_isDown;
}

bool Input::isKeyUp(unsigned char keyCode) {
	return !keyStates[keyCode].m_isDown;
}

bool Input::isKeyJustDown(unsigned char keyCode) {
	return keyStates[keyCode].m_isDown && !keyStates[keyCode].m_isPreviousDown;
}

bool Input::isKeyJustUp(unsigned char keyCode) {
	return keyStates[keyCode].m_isPreviousDown && !keyStates[keyCode].m_isDown;
}

void Input::beforeFrame() {}

void Input::afterFrame() {
	for (int i = 0; i < NUM_KEY; i++) {
		keyStates[i].m_isPreviousDown = keyStates[i].m_isDown;
	}
}
