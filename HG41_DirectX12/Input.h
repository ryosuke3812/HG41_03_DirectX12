#ifndef __INPUT_H__
#define __INPUT_H__

#include <Windows.h>

void UpdateInput();

bool IsKeyPress(BYTE key);
bool IsKeyTrigger(BYTE key);
bool IsKeyRelease(BYTE key);

#endif // __INPUT_H__