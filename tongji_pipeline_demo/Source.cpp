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

private:
	void BuildGeometryBuffers();
	void BuildShaders();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;
	ID3D11Buffer* mCB;

	ID3D11VertexShader*		mVS;
	ID3D11PixelShader*		mPS;
	ID3D11GeometryShader*	mGS;
	ID3DBlob* mCompiledVS;
	ID3DBlob* mCompiledPS;
	ID3DBlob* mCompiledGS;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

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
}

void PipelineDemo::BuildShaders()
{
	const std::string shaderFilePath("..\\data\\shader.hlsl");


	mCompiledVS = GenerateShader(shaderFilePath, "VSMain", "vs_5_0", NULL);
	auto result = md3dDevice->CreateVertexShader(
		mCompiledVS->GetBufferPointer(),
		mCompiledVS->GetBufferSize(),
		0, &mVS);

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

	D3D11_BUFFER_DESC descCB;
	ZeroMemory(&descCB, sizeof(descCB));
	descCB.ByteWidth = sizeof(CBufferType);
	descCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	descCB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	descCB.Usage = D3D11_USAGE_DYNAMIC;
	result = md3dDevice->CreateBuffer(&descCB, NULL, &mCB);
	md3dImmediateContext->GSSetConstantBuffers(0, 1, &mCB);

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

PipelineDemo::PipelineDemo(HINSTANCE hInstance)
	: D3DApp(hInstance)
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
	ReleaseCOM(mCompiledGS);
	ReleaseCOM(mCompiledPS);
	ReleaseCOM(mVS);
	ReleaseCOM(mGS);
	ReleaseCOM(mPS);
	ReleaseCOM(mCB);

	ReleaseCOM(mInputLayout);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
}

bool PipelineDemo::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildShaders();
	BuildVertexLayout();

	D3D11_RASTERIZER_DESC rs;
	rs.FillMode = D3D11_FILL_WIREFRAME;
	rs.CullMode = D3D11_CULL_NONE;
	rs.FrontCounterClockwise = false;
	rs.DepthBias = 0;
	rs.SlopeScaledDepthBias = 0.0f;
	rs.DepthBiasClamp = 0.0f;
	rs.DepthClipEnable = true;
	rs.ScissorEnable = false;
	rs.MultisampleEnable = false;
	rs.AntialiasedLineEnable = false;

	ID3D11RasterizerState* pState;
	md3dDevice->CreateRasterizerState(&rs, &pState);
	md3dImmediateContext->RSSetState(pState);

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

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world*view*proj;

	D3D11_MAPPED_SUBRESOURCE p;
	CBufferType* pBuffer = 0;
	auto hr = md3dImmediateContext->Map(mCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &p);
	if (SUCCEEDED(hr)) 
	{
		pBuffer = (CBufferType*)p.pData;
		pBuffer->mat = worldViewProj;
		pBuffer->distance = XMFLOAT4(0.0f, 0.2f, 0.5f, 1.0f);
		md3dImmediateContext->Unmap(mCB, 0);
	}

	md3dImmediateContext->DrawIndexed(36, 0, 0);

	HR(mSwapChain->Present(0, 0));
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