
#include "Input.h"
#include <algorithm>
#include "SceneFractal.h"
#include "SceneLSystem.h"
#include "SceneProcedural.h"
#include "SceneField.h"

enum Scenes {
	FRACTAL,
	LSYSTEM,
	PROCEDURAL,
	FIELD,
	MAX,

	START = FRACTAL
};
int g_scene = FRACTAL;

SceneFractal g_fractal;
SceneLSystem g_lsystem;
SceneProcedural g_procedural;
SceneField g_field;

HRESULT InitScene()
{
	HRESULT hr = E_FAIL;
	switch (g_scene) {
	default:
	case FRACTAL: hr = g_fractal.Init(); break;
	case LSYSTEM: hr = g_lsystem.Init(); break;
	case PROCEDURAL: hr = g_procedural.Init(); break;
	case FIELD: hr = g_field.Init(); break;
	}
	return hr;
}
void UninitScene()
{
	switch (g_scene) {
	default:
	case FRACTAL: g_fractal.Uninit(); break;
	case LSYSTEM: g_lsystem.Uninit(); break;
	case PROCEDURAL: g_procedural.Uninit(); break;
	case FIELD: g_field.Uninit(); break;
	}
}
void DrawScene()
{
	// ÉLÅ[ì¸óÕ
	int preScene = g_scene;
	if (IsKeyTrigger(VK_LEFT))
		g_scene = max(START, g_scene--);
	if (IsKeyTrigger(VK_RIGHT))
		g_scene = min(MAX, g_scene++);
	if (g_scene != preScene) {
		int newScene = g_scene;
		g_scene = preScene;
		UninitScene();
		g_scene = newScene;
		InitScene();
	}

	// ï`âÊ
	switch (g_scene) {
	default:
	case FRACTAL: g_fractal.Draw(); break;
	case LSYSTEM: g_lsystem.Draw(); break;
	case PROCEDURAL: g_procedural.Draw(); break;
	case FIELD: g_field.Draw(); break;
	}

}