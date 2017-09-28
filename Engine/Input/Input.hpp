#pragma once
#include "Engine/Input/KeyState.hpp"
#include "Engine/Input/XboxController.hpp"

void runMessagePump();
class XboxController;
class Input {
public:
	Input();
	~Input();
	static constexpr int NUM_KEY = 0xff;
	void onKeyDown(unsigned char keyCode);
	void onKeyUp(unsigned char keyCode);

	bool isKeyDown(unsigned char keyCode);
	bool isKeyUp(unsigned char keyCode);
	bool isKeyJustDown(unsigned char keyCode);
	bool isKeyJustUp(unsigned char keyCode);

	void beforeFrame();
	void afterFrame();
	XboxController* getController(XboxControllerID controllerId);

protected:
	void updateKeyboard();
	void updateXboxControllers();

protected:
	KeyState m_keyStates[NUM_KEY];
	XboxController* m_xboxControllers[NUM_XBOXCONTROLLER]{nullptr};

};


/*
 * Virtual Keys, Standard Set
 */
#define KEYBOARD_LBUTTON        0x01
#define KEYBOARD_RBUTTON        0x02
#define KEYBOARD_CANCEL         0x03
#define KEYBOARD_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

#if(_WIN32_WINNT >= 0x0500)
#define KEYBOARD_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define KEYBOARD_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */
#endif /* _WIN32_WINNT >= 0x0500 */

/*
 * 0x07 : reserved
 */


#define KEYBOARD_BACK           0x08
#define KEYBOARD_TAB            0x09

/*
 * 0x0A - 0x0B : reserved
 */

#define KEYBOARD_CLEAR          0x0C
#define KEYBOARD_RETURN         0x0D

/*
 * 0x0E - 0x0F : unassigned
 */

#define KEYBOARD_SHIFT          0x10
#define KEYBOARD_CONTROL        0x11
#define KEYBOARD_MENU           0x12
#define KEYBOARD_PAUSE          0x13
#define KEYBOARD_CAPITAL        0x14

#define KEYBOARD_KANA           0x15
#define KEYBOARD_HANGEUL        0x15  /* old name - should be here for compatibility */
#define KEYBOARD_HANGUL         0x15

/*
 * 0x16 : unassigned
 */

#define KEYBOARD_JUNJA          0x17
#define KEYBOARD_FINAL          0x18
#define KEYBOARD_HANJA          0x19
#define KEYBOARD_KANJI          0x19

/*
 * 0x1A : unassigned
 */

#define KEYBOARD_ESCAPE         0x1B

#define KEYBOARD_CONVERT        0x1C
#define KEYBOARD_NONCONVERT     0x1D
#define KEYBOARD_ACCEPT         0x1E
#define KEYBOARD_MODECHANGE     0x1F

#define KEYBOARD_SPACE          0x20
#define KEYBOARD_PRIOR          0x21
#define KEYBOARD_NEXT           0x22
#define KEYBOARD_END            0x23
#define KEYBOARD_HOME           0x24
#define KEYBOARD_LEFT           0x25
#define KEYBOARD_UP             0x26
#define KEYBOARD_RIGHT          0x27
#define KEYBOARD_DOWN           0x28
#define KEYBOARD_SELECT         0x29
#define KEYBOARD_PRINT          0x2A
#define KEYBOARD_EXECUTE        0x2B
#define KEYBOARD_SNAPSHOT       0x2C
#define KEYBOARD_INSERT         0x2D
#define KEYBOARD_DELETE         0x2E
#define KEYBOARD_HELP           0x2F

/*
 * KEYBOARD_0 - KEYBOARD_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x3A - 0x40 : unassigned
 * KEYBOARD_A - KEYBOARD_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */

#define KEYBOARD_LWIN           0x5B
#define KEYBOARD_RWIN           0x5C
#define KEYBOARD_APPS           0x5D

/*
 * 0x5E : reserved
 */

#define KEYBOARD_SLEEP          0x5F

#define KEYBOARD_NUMPAD0        0x60
#define KEYBOARD_NUMPAD1        0x61
#define KEYBOARD_NUMPAD2        0x62
#define KEYBOARD_NUMPAD3        0x63
#define KEYBOARD_NUMPAD4        0x64
#define KEYBOARD_NUMPAD5        0x65
#define KEYBOARD_NUMPAD6        0x66
#define KEYBOARD_NUMPAD7        0x67
#define KEYBOARD_NUMPAD8        0x68
#define KEYBOARD_NUMPAD9        0x69
#define KEYBOARD_MULTIPLY       0x6A
#define KEYBOARD_ADD            0x6B
#define KEYBOARD_SEPARATOR      0x6C
#define KEYBOARD_SUBTRACT       0x6D
#define KEYBOARD_DECIMAL        0x6E
#define KEYBOARD_DIVIDE         0x6F
#define KEYBOARD_F1             0x70
#define KEYBOARD_F2             0x71
#define KEYBOARD_F3             0x72
#define KEYBOARD_F4             0x73
#define KEYBOARD_F5             0x74
#define KEYBOARD_F6             0x75
#define KEYBOARD_F7             0x76
#define KEYBOARD_F8             0x77
#define KEYBOARD_F9             0x78
#define KEYBOARD_F10            0x79
#define KEYBOARD_F11            0x7A
#define KEYBOARD_F12            0x7B
#define KEYBOARD_F13            0x7C
#define KEYBOARD_F14            0x7D
#define KEYBOARD_F15            0x7E
#define KEYBOARD_F16            0x7F
#define KEYBOARD_F17            0x80
#define KEYBOARD_F18            0x81
#define KEYBOARD_F19            0x82
#define KEYBOARD_F20            0x83
#define KEYBOARD_F21            0x84
#define KEYBOARD_F22            0x85
#define KEYBOARD_F23            0x86
#define KEYBOARD_F24            0x87
