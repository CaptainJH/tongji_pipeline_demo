#include "d3dApp.h"

using namespace DirectX;

struct CBufferType
{
	XMMATRIX mat;
	XMFLOAT4 distance;
};

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class PipelineDemo : public D3DApp
{
public:
	PipelineDemo(HINSTANCE hInstance);
	~PipelineDemo();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void BuildGeometryBuffers();
	void BuildShaders();
	void BuildVertexLayout();
	void BuildMSAARenderTarget();

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;
	ID3D11Buffer* mCB;
	ID3D11Buffer* mQuad;

	ID3D11VertexShader*		mVS;
	ID3D11VertexShader*		mVSQuad;
	ID3D11PixelShader*		mPS;
	ID3D11PixelShader*		mPSAA;
	ID3D11PixelShader*		mPSQuad;
	ID3D11GeometryShader*	mGS;
	ID3DBlob* mCompiledVS;
	ID3DBlob* mCompiledVSQuad;
	ID3DBlob* mCompiledPS;
	ID3DBlob* mCompiledPSAA;
	ID3DBlob* mCompiledPSQuad;
	ID3DBlob* mCompiledGS;

	ID3D11Texture2D*	mRTTexAA;
	ID3D11Texture2D*	mDSTexAA;
	ID3D11Texture2D*	mResolveTex;
	ID3D11RenderTargetView*	mRTV;
	ID3D11DepthStencilView* mDSV;
	ID3D11ShaderResourceView*	mSRV;
	ID3D11SamplerState*	mSampler;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	bool mDrawWireFrame;

	POINT mLastMousePos;
};

void PipelineDemo::BuildGeometryBuffers()
{
	// Create vertex buffer
	Vertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)Colors::White) },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)Colors::Black) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)Colors::Red) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)Colors::Green) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)Colors::Blue) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)Colors::Yellow) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)Colors::Cyan) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)Colors::Magenta) }
	};

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));


	// Create the index buffer

	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));


	/////////////
	///build quad
	/////////////
	Vertex quadVertices[] =
	{
		{ XMFLOAT3(-1.0f, +1.0f, 0.0f), XMFLOAT4((const float*)Colors::White) },
		{ XMFLOAT3(+1.0f, +1.0f, 0.0f), XMFLOAT4((const float*)Colors::White) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4((const float*)Colors::White) },
		{ XMFLOAT3(+1.0f, -1.0f, 0.0f), XMFLOAT4((const float*)Colors::White) }
	};

	D3D11_BUFFER_DESC quadVBd;
	quadVBd.Usage = D3D11_USAGE_IMMUTABLE;
	quadVBd.ByteWidth = sizeof(Vertex) * 4;
	quadVBd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	quadVBd.CPUAccessFlags = 0;
	quadVBd.MiscFlags = 0;
	quadVBd.StructureByteStride = 0;
	vinitData.pSysMem = quadVertices;
	HR(md3dDevice->CreateBuffer(&quadVBd, &vinitData, &mQuad));
}

void PipelineDemo::BuildShaders()
{
	const std::string shaderFilePath("..\\data\\shader.hlsl");


	mCompiledVS = GenerateShader(shaderFilePath, "VSMain", "vs_5_0", NULL);
	auto result = md3dDevice->CreateVertexShader(
		mCompiledVS->GetBufferPointer(),
		mCompiledVS->GetBufferSize(),
		0, &mVS);

	mCompiledVSQuad = GenerateShader(shaderFilePath, "VSMainQuad", "vs_5_0", NULL);
	result = md3dDevice->CreateVertexShader(
		mCompiledVSQuad->GetBufferPointer(),
		mCompiledVSQuad->GetBufferSize(),
		0, &mVSQuad);

	mCompiledGS = GenerateShader(shaderFilePath, "GSMain", "gs_5_0", NULL);
	result = md3dDevice->CreateGeometryShader(
		mCompiledGS->GetBufferPointer(),
		mCompiledGS->GetBufferSize(),
		0, &mGS);

	mCompiledPS = GenerateShader(shaderFilePath, "PSMain", "ps_5_0", NULL);
	result = md3dDevice->CreatePixelShader(
		mCompiledPS->GetBufferPointer(),
		mCompiledPS->GetBufferSize(),
		0, &mPS);

	mCompiledPSAA = GenerateShader(shaderFilePath, "PSMainAA", "ps_5_0", NULL);
	result = md3dDevice->CreatePixelShader(
		mCompiledPSAA->GetBufferPointer(),
		mCompiledPSAA->GetBufferSize(),
		0, &mPSAA);

	mCompiledPSQuad = GenerateShader(shaderFilePath, "PSMainQuad", "ps_5_0", NULL);
	result = md3dDevice->CreatePixelShader(
		mCompiledPSQuad->GetBufferPointer(),
		mCompiledPSQuad->GetBufferSize(),
		0, &mPSQuad);

	D3D11_BUFFER_DESC descCB;
	ZeroMemory(&descCB, sizeof(descCB));
	descCB.ByteWidth = sizeof(CBufferType);
	descCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	descCB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	descCB.Usage = D3D11_USAGE_DYNAMIC;
	result = md3dDevice->CreateBuffer(&descCB, NULL, &mCB);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &mCB);
	md3dImmediateContext->GSSetConstantBuffers(0, 1, &mCB);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &mCB);

	md3dImmediateContext->VSSetShader(mVS, NULL, 0);
	md3dImmediateContext->GSSetShader(mGS, NULL, 0);
	md3dImmediateContext->PSSetShader(mPS, NULL, 0);
}

void PipelineDemo::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the input layout
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, mCompiledVS->GetBufferPointer(),
		mCompiledVS->GetBufferSize(), &mInputLayout));
}

void PipelineDemo::BuildMSAARenderTarget()
{
	D3D11_TEXTURE2D_DESC descTex;
	ZeroMemory(&descTex, sizeof(descTex));
	descTex.ArraySize = 1;
	descTex.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	descTex.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descTex.Width = mClientWidth;
	descTex.Height = mClientHeight;
	descTex.Usage = D3D11_USAGE_DEFAULT;
	descTex.SampleDesc.Count = 8;
	descTex.SampleDesc.Quality = m4xMsaaQuality - 1;
	descTex.MipLevels = 1;
	HR(md3dDevice->CreateTexture2D(&descTex, NULL, &mRTTexAA));
	HR(md3dDevice->CreateRenderTargetView(mRTTexAA, NULL, &mRTV));

	descTex.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descTex.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	HR(md3dDevice->CreateTexture2D(&descTex, NULL, &mDSTexAA));
	HR(md3dDevice->CreateDepthStencilView(mDSTexAA, NULL, &mDSV));

	descTex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	descTex.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descTex.SampleDesc.Count = 1;
	descTex.SampleDesc.Quality = 0;
	HR(md3dDevice->CreateTexture2D(&descTex, NULL, &mResolveTex));
	HR(md3dDevice->CreateShaderResourceView(mResolveTex, NULL, &mSRV));

	D3D11_SAMPLER_DESC  descSamp;
	ZeroMemory(&descSamp, sizeof(descSamp));
	descSamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	descSamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	descSamp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	descSamp.MaxAnisotropy = 1;
	descSamp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	descSamp.ComparisonFunc = D3D11_COMPARISON_NEVER;
	descSamp.MaxLOD = D3D11_FLOAT32_MAX;
	md3dDevice->CreateSamplerState(&descSamp, &mSampler);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampler);

}

PipelineDemo::PipelineDemo(HINSTANCE hInstance)
	: D3DApp(hInstance)
	, mDrawWireFrame(true)
{
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = DirectX::XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.5f*MathHelper::Pi, AspectRatio(), 0.01f, 100.0f);
	XMStoreFloat4x4(&mProj, P);
}

PipelineDemo::~PipelineDemo()
{
	ReleaseCOM(mCompiledVS);
	ReleaseCOM(mCompiledVSQuad);
	ReleaseCOM(mCompiledGS);
	ReleaseCOM(mCompiledPS);
	ReleaseCOM(mCompiledPSAA);
	ReleaseCOM(mCompiledPSQuad);
	ReleaseCOM(mVS);
	ReleaseCOM(mVSQuad);
	ReleaseCOM(mGS);
	ReleaseCOM(mPS);
	ReleaseCOM(mPSAA);
	ReleaseCOM(mPSQuad);
	ReleaseCOM(mCB);

	ReleaseCOM(mInputLayout);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mQuad);

	ReleaseCOM(mRTV);
	ReleaseCOM(mDSV);
	ReleaseCOM(mSRV);
	ReleaseCOM(mRTTexAA);
	ReleaseCOM(mDSTexAA);
	ReleaseCOM(mResolveTex);

	ReleaseCOM(mSampler);
}

bool PipelineDemo::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildShaders();
	BuildVertexLayout();
	BuildMSAARenderTarget();

	D3D11_RASTERIZER_DESC rs;
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = D3D11_CULL_BACK;
	rs.FrontCounterClockwise = false;
	rs.DepthBias = 0;
	rs.SlopeScaledDepthBias = 0.0f;
	rs.DepthBiasClamp = 0.0f;
	rs.DepthClipEnable = true;
	rs.ScissorEnable = true;
	rs.MultisampleEnable = false;
	rs.AntialiasedLineEnable = false;

	ID3D11RasterizerState* pState;
	md3dDevice->CreateRasterizerState(&rs, &pState);
	md3dImmediateContext->RSSetState(pState);

	ReleaseCOM(pState);

	return true;
}

void PipelineDemo::OnResize()
{
	D3DApp::OnResize();
}

void PipelineDemo::UpdateScene(float dt)
{
	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	static DWORD dwTimeStart = 0;
	static float t = 0.0f;
	DWORD dwTimeCur = GetTickCount();
	if (dwTimeStart == 0)
		dwTimeStart = dwTimeCur;
	t = (dwTimeCur - dwTimeStart) / 1000.0f;
	XMMATRIX world = DirectX::XMMatrixRotationY(t) * DirectX::XMMatrixRotationX(t);
	XMStoreFloat4x4(&mWorld, world);
}

void PipelineDemo::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world*view*proj;

	// draw without AA
	{
		md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
		md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
		md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		md3dImmediateContext->IASetInputLayout(mInputLayout);
		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		D3D11_RECT rect = { 0, 0, 400, 600 };
		md3dImmediateContext->RSSetScissorRects(1, &rect);
		md3dImmediateContext->VSSetShader(mVS, NULL, 0);
		md3dImmediateContext->GSSetShader(mGS, NULL, 0);
		md3dImmediateContext->PSSetShader(mPS, NULL, 0);

		D3D11_MAPPED_SUBRESOURCE p;
		CBufferType* pBuffer = 0;
		auto hr = md3dImmediateContext->Map(mCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &p);
		if (SUCCEEDED(hr)) 
		{
			pBuffer = (CBufferType*)p.pData;
			pBuffer->mat = worldViewProj;
			pBuffer->distance = XMFLOAT4(1.0f, 0.0f, 0.0f, mDrawWireFrame ? 1.0f : 0.0f);
			md3dImmediateContext->Unmap(mCB, 0);
		}

		md3dImmediateContext->DrawIndexed(36, 0, 0);
	}

	// draw with AA
	{
		md3dImmediateContext->OMSetRenderTargets(1, &mRTV, mDSV);
		md3dImmediateContext->ClearRenderTargetView(mRTV, reinterpret_cast<const float*>(&Colors::Blue));
		md3dImmediateContext->ClearDepthStencilView(mDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		md3dImmediateContext->PSSetShader(mPSAA, NULL, 0);

		D3D11_RECT rect1 = { 0, 0, 800, 600 };
		md3dImmediateContext->RSSetScissorRects(1, &rect1);

		D3D11_MAPPED_SUBRESOURCE p;
		CBufferType* pBuffer = 0;
		auto hr = md3dImmediateContext->Map(mCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &p);
		if (SUCCEEDED(hr))
		{
			pBuffer = (CBufferType*)p.pData;
			pBuffer->mat = worldViewProj;
			pBuffer->distance = XMFLOAT4(0.0f, .0f, 0.0f, mDrawWireFrame ? 1.0f : 0.0f);
			md3dImmediateContext->Unmap(mCB, 0);
		}
		md3dImmediateContext->DrawIndexed(36, 0, 0);


		md3dImmediateContext->ResolveSubresource(mResolveTex, 0, mRTTexAA, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		//////////////////
		///// draw quad
		//////////////////
		md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
		md3dImmediateContext->IASetInputLayout(mInputLayout);
		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuad, &stride, &offset);

		D3D11_RECT rect2 = { 400, 0, 800, 600 };
		md3dImmediateContext->RSSetScissorRects(1, &rect2);

		md3dImmediateContext->VSSetShader(mVSQuad, NULL, 0);
		md3dImmediateContext->GSSetShader(NULL, NULL, 0);
		md3dImmediateContext->PSSetShader(mPSQuad, NULL, 0);
		md3dImmediateContext->PSSetShaderResources(0, 1, &mSRV);
		md3dImmediateContext->PSSetSamplers(0, 1, &mSampler);

		md3dImmediateContext->Draw(4, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

LRESULT PipelineDemo::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			mAppPaused = !mAppPaused;
			break;

		case VK_ESCAPE:
			PostQuitMessage(0);
			break;

		case VK_RETURN:
			mDrawWireFrame = !mDrawWireFrame;
			break;
		};
	};

	return D3DApp::MsgProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	PipelineDemo theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}