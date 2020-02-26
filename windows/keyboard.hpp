#pragma once

// we use VK_SNAPSHOT as a sentinel because libui will never support the print screen key; that key belongs to the user
struct extkeymap {
	WPARAM vk;
	uiExtKey extkey;
};

// all mappings come from GLFW - https://github.com/glfw/glfw/blob/master/src/win32_window.c#L152
static const struct extkeymap numpadExtKeys[] = {
	{ VK_HOME, uiExtKeyN7 },
	{ VK_UP, uiExtKeyN8 },
	{ VK_PRIOR, uiExtKeyN9 },
	{ VK_LEFT, uiExtKeyN4 },
	{ VK_CLEAR, uiExtKeyN5 },
	{ VK_RIGHT, uiExtKeyN6 },
	{ VK_END, uiExtKeyN1 },
	{ VK_DOWN, uiExtKeyN2 },
	{ VK_NEXT, uiExtKeyN3 },
	{ VK_INSERT, uiExtKeyN0 },
	{ VK_DELETE, uiExtKeyNDot },
	{ VK_SNAPSHOT, 0 },
};

static const struct extkeymap extKeys[] = {
	{ VK_ESCAPE, uiExtKeyEscape },
	{ VK_INSERT, uiExtKeyInsert },
	{ VK_DELETE, uiExtKeyDelete },
	{ VK_HOME, uiExtKeyHome },
	{ VK_END, uiExtKeyEnd },
	{ VK_PRIOR, uiExtKeyPageUp },
	{ VK_NEXT, uiExtKeyPageDown },
	{ VK_UP, uiExtKeyUp },
	{ VK_DOWN, uiExtKeyDown },
	{ VK_LEFT, uiExtKeyLeft },
	{ VK_RIGHT, uiExtKeyRight },
	{ VK_F1, uiExtKeyF1 },
	{ VK_F2, uiExtKeyF2 },
	{ VK_F3, uiExtKeyF3 },
	{ VK_F4, uiExtKeyF4 },
	{ VK_F5, uiExtKeyF5 },
	{ VK_F6, uiExtKeyF6 },
	{ VK_F7, uiExtKeyF7 },
	{ VK_F8, uiExtKeyF8 },
	{ VK_F9, uiExtKeyF9 },
	{ VK_F10, uiExtKeyF10 },
	{ VK_F11, uiExtKeyF11 },
	{ VK_F12, uiExtKeyF12 },
	// numpad numeric keys and . are handled in common/areaevents.c
	// numpad enter is handled in code below
	{ VK_ADD, uiExtKeyNAdd },
	{ VK_SUBTRACT, uiExtKeyNSubtract },
	{ VK_MULTIPLY, uiExtKeyNMultiply },
	{ VK_DIVIDE, uiExtKeyNDivide },
	{ VK_SNAPSHOT, 0 },
};

static const struct {
	WPARAM vk;
	uiModifiers mod;
} modKeys[] = {
	// even if the separate left/right aren't necessary, have them here anyway, just to be safe
	{ VK_CONTROL, uiModifierCtrl },
	{ VK_LCONTROL, uiModifierCtrl },
	{ VK_RCONTROL, uiModifierCtrl },
	{ VK_MENU, uiModifierAlt },
	{ VK_LMENU, uiModifierAlt },
	{ VK_RMENU, uiModifierAlt },
	{ VK_SHIFT, uiModifierShift },
	{ VK_LSHIFT, uiModifierShift },
	{ VK_RSHIFT, uiModifierShift },
	// there's no combined Windows key virtual-key code as there is with the others
	{ VK_LWIN, uiModifierSuper },
	{ VK_RWIN, uiModifierSuper },
	{ VK_SNAPSHOT, 0 },
};
