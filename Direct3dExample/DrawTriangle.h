#pragma once

#include <d3dcompiler.h>
#include "D3DFramework.h"

#pragma comment (lib, "d3dcompiler.lib")

class DrawTriangle : public D3DFramework
{
	struct VERTEX
	{
		FLOAT X, Y, Z;
		FLOAT color[4];
	};

	Microsoft::WRL::ComPtr<ID3D11InputLayout> mspInputLayout;
	Microsoft::WRL::ComPtr<ID3D10Buffer>      mspVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mspVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mspPixelShader;

public : 
	void Initialize(HINSTANCE hInstance, int width = 800, int hight = 600) override;
	void Destroy() override;

private: 
	void InitTriangle();
	void InitPipeline();

protected:
	void Render() override;
};

