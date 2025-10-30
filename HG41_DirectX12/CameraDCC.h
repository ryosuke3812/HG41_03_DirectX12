#ifndef __CAMERA_DCC_H__
#define __CAMERA_DCC_H__

#include <DirectXMath.h>
#include <Windows.h>

class CameraDCC
{
private:
	struct Argument
	{
		DirectX::XMFLOAT2 mouseMove;
		DirectX::XMVECTOR vCamFront;
		DirectX::XMVECTOR vCamSide;
		DirectX::XMVECTOR vCamUp;
		DirectX::XMVECTOR vCamPos;
		DirectX::XMVECTOR vCamLook;
		float focus;
	};
public:
	CameraDCC();
	~CameraDCC();
	void Update();

	DirectX::XMFLOAT3 GetPos();
	DirectX::XMFLOAT4X4 GetView(bool transpose = true);
	DirectX::XMFLOAT4X4 GetProj(bool transpose = true);

private:
	void UpdateState();
	void UpdateOrbit(Argument& arg);
	void UpdateTrack(Argument& arg);
	void UpdateDolly(Argument& arg);
	void UpdateFlight(Argument& arg);

private:
	int m_state;
	POINT m_oldPos;
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_look;
	DirectX::XMFLOAT3 m_up;
	float m_fovy;
	float m_aspect;
	float m_near;
	float m_far;
};

#endif // __CAMERA_DCC_H__