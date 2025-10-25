//TODO: :=< 16.10.25 23:00 слушаю бобра за столом и хочу сделать хоть немного функций по типу как рендер трех мерных объектов, рендеринг друх мерных объектов
//Скриптинг хотя бы на lua но лучше на c++ так как прикольнее, toolkit (внутри готовые объекты как в source), физику через bullet, редактор сцен, ну и пока что все но хочу ещё сделать минимальный анти чит
// ещё много работы наверное я заброшу проект сделав 50% максимум 01.01.26 мое последнее обновление я уже забрасываю и перехожу на unreal engine 5 мне купили 5070 ti
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <filesystem>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui/backends/imgui_impl_win32.h"
#include <string>
#include <sstream>
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <iostream>
#include <d3dcompiler.h>
#include <string>
#include <vector>
#include <wincodec.h>
#include <wrl/client.h>
#include <fstream>
#include <DirectXCollision.h>
#include <windowsx.h>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "../WindowsProject4/simpleini.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

#define gui ImGui

using namespace DirectX;
// g_
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
bool mains = true;
bool ui = false;

UINT g_windowWidth = 1280;
UINT g_windowHeight = 800;

float fps = 0.0f;
float frameTime = 0.0f;
bool show_settings = false;

enum ProjectType {
    PROJECT_NONE,
    PROJECT_2D,
    PROJECT_3D
};

ProjectType g_currentProjectType = PROJECT_NONE;
std::string g_currentProjectName = "";
bool g_showProjectCreation = false;
char g_projectNameBuffer[128] = "NewProject";

int EbankoEbychee = 0;

std::string name;
const char* eng[10] = { "File", "open", "close", "language", "save", "quit", "tools", "copy", "paste", "licensce" };
const char* rus[10] = { u8"Файл", u8"открыть", u8"закрыть", u8"язык", u8"сохранить", u8"выйти", u8"инструменты", u8"скопировать", u8"вставить", u8"лицензия" };
bool languages = false;
int counts = 1;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void DebugOutput(const std::string& message);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

UINT GetWindowWidth() { return g_windowWidth; }
UINT GetWindowHeight() { return g_windowHeight; }
float GetAspectRatio() { return static_cast<float>(g_windowWidth) / g_windowHeight; }
void HandleResize(UINT newWidth, UINT newHeight);

class RenderToTexture {
private:
    ID3D11Texture2D* m_renderTargetTexture;
    ID3D11RenderTargetView* m_renderTargetView;
    ID3D11ShaderResourceView* m_shaderResourceView;
    UINT m_width, m_height;

public:
    RenderToTexture() : m_renderTargetTexture(nullptr), m_renderTargetView(nullptr),
        m_shaderResourceView(nullptr), m_width(0), m_height(0) {
    }

    bool Initialize(ID3D11Device* device, UINT width, UINT height) {
        m_width = width;
        m_height = height;

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;

        HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &m_renderTargetTexture);
        if (FAILED(hr)) return false;

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
        renderTargetViewDesc.Format = textureDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;

        hr = device->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetView);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
        shaderResourceViewDesc.Format = textureDesc.Format;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        shaderResourceViewDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);
        if (FAILED(hr)) return false;

        return true;
    }

    void BeginRender(ID3D11DeviceContext* context, float r, float g, float b, float a) {
        context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

        float clearColor[4] = { r, g, b, a };
        context->ClearRenderTargetView(m_renderTargetView, clearColor);
    }

    void EndRender(ID3D11DeviceContext* context) {
        context->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    }

    ID3D11ShaderResourceView* GetShaderResourceView() const {
        return m_shaderResourceView;
    }

    void Shutdown() {
        if (m_shaderResourceView) m_shaderResourceView->Release();
        if (m_renderTargetView) m_renderTargetView->Release();
        if (m_renderTargetTexture) m_renderTargetTexture->Release();
    }
};


class Renderer3D {
private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;

    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_inputLayout;
    ID3D11Buffer* m_constantBuffer;

public:
    Renderer3D() : m_device(nullptr), m_context(nullptr), m_vertexBuffer(nullptr),
        m_indexBuffer(nullptr), m_vertexShader(nullptr), m_pixelShader(nullptr),
        m_inputLayout(nullptr), m_constantBuffer(nullptr) {
    }

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
        m_device = device;
        m_context = context;

        struct Vertex {
            XMFLOAT3 position;
            XMFLOAT4 color;
        };

        Vertex vertices[] = {
            { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
            { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
            { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
            { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
            { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
            { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
            { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
            { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f) }
        };

        D3D11_BUFFER_DESC vbd = {};
        vbd.Usage = D3D11_USAGE_DEFAULT;
        vbd.ByteWidth = sizeof(vertices);
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA vinitData = {};
        vinitData.pSysMem = vertices;

        if (FAILED(m_device->CreateBuffer(&vbd, &vinitData, &m_vertexBuffer))) {
            DebugOutput("Failed to create vertex buffer");
            return false;
        }

        WORD indices[] = {
            0,1,2, 0,2,3,  
            4,6,5, 4,7,6,  
            4,5,1, 4,1,0,  
            3,2,6, 3,6,7,  
            1,5,6, 1,6,2,  
            4,0,3, 4,3,7  
        };

        D3D11_BUFFER_DESC ibd = {};
        ibd.Usage = D3D11_USAGE_DEFAULT;
        ibd.ByteWidth = sizeof(indices);
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA iinitData = {};
        iinitData.pSysMem = indices;

        if (FAILED(m_device->CreateBuffer(&ibd, &iinitData, &m_indexBuffer))) {
            DebugOutput("Failed to create index buffer");
            return false;
        }

        const char* vsCode =
            "cbuffer MatrixBuffer { matrix world; matrix view; matrix projection; }"
            "struct VS_IN { float3 pos : POSITION; float4 col : COLOR; };"
            "struct VS_OUT { float4 pos : SV_POSITION; float4 col : COLOR; };"
            "VS_OUT main(VS_IN input) {"
            "    VS_OUT output;"
            "    output.pos = mul(float4(input.pos, 1.0), world);"
            "    output.pos = mul(output.pos, view);"
            "    output.pos = mul(output.pos, projection);"
            "    output.col = input.col;"
            "    return output;"
            "}";

        const char* psCode =
            "struct PS_IN { float4 pos : SV_POSITION; float4 col : COLOR; };"
            "float4 main(PS_IN input) : SV_TARGET { return input.col; }";

        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;

        if (FAILED(D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob))) {
            DebugOutput("Failed to compile vertex shader");
            if (errorBlob) {
                DebugOutput((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            return false;
        }

        if (FAILED(D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob))) {
            DebugOutput("Failed to compile pixel shader");
            if (errorBlob) {
                DebugOutput((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            return false;
        }

        if (FAILED(m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader))) {
            DebugOutput("Failed to create vertex shader");
            return false;
        }

        if (FAILED(m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader))) {
            DebugOutput("Failed to create pixel shader");
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        if (FAILED(m_device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout))) {
            DebugOutput("Failed to create input layout");
            return false;
        }

        vsBlob->Release();
        psBlob->Release();

        D3D11_BUFFER_DESC cbDesc = {};
        cbDesc.ByteWidth = sizeof(XMMATRIX) * 3;
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (FAILED(m_device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer))) {
            DebugOutput("Failed to create constant buffer");
            return false;
        }

        DebugOutput("3D Renderer initialized successfully");
        return true;
    }

    void Render() {
        if (!m_device || !m_context) return;

        m_context->VSSetShader(m_vertexShader, nullptr, 0);
        m_context->PSSetShader(m_pixelShader, nullptr, 0);
        m_context->IASetInputLayout(m_inputLayout);

        UINT stride = sizeof(float) * 7; 
        UINT offset = 0;
        m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
        m_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        static float rotation = 0.0f;
        rotation += 0.01f;

        XMMATRIX world = XMMatrixRotationY(rotation) * XMMatrixRotationX(rotation);
        XMMATRIX view = XMMatrixLookAtLH(
            XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
            XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
            XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
        );
        XMMATRIX projection = XMMatrixPerspectiveFovLH(
            XM_PIDIV4, GetAspectRatio(), 0.1f, 100.0f
        );

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        if (SUCCEEDED(m_context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
            XMMATRIX* matrices = (XMMATRIX*)mappedResource.pData;
            matrices[0] = XMMatrixTranspose(world);
            matrices[1] = XMMatrixTranspose(view);
            matrices[2] = XMMatrixTranspose(projection);
            m_context->Unmap(m_constantBuffer, 0);
        }

        m_context->VSSetConstantBuffers(0, 1, &m_constantBuffer);

        m_context->DrawIndexed(36, 0, 0);
    }

    void skibidi();

    void Shutdown() {
        if (m_vertexBuffer) m_vertexBuffer->Release();
        if (m_indexBuffer) m_indexBuffer->Release();
        if (m_vertexShader) m_vertexShader->Release();
        if (m_pixelShader) m_pixelShader->Release();
        if (m_inputLayout) m_inputLayout->Release();
        if (m_constantBuffer) m_constantBuffer->Release();
    }
};

class Renderer2D {
private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_inputLayout;

public:
    Renderer2D() : m_device(nullptr), m_context(nullptr), m_vertexBuffer(nullptr),
        m_indexBuffer(nullptr), m_vertexShader(nullptr), m_pixelShader(nullptr),
        m_inputLayout(nullptr) {
    }

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
        m_device = device;
        m_context = context;

        struct Vertex {
            XMFLOAT3 position;
            XMFLOAT4 color;
        };

        Vertex vertices[] = {
            { XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
            { XMFLOAT3(-0.5f,  0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
            { XMFLOAT3(0.5f,  0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }, 
            { XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) }  
        };

        D3D11_BUFFER_DESC vbd = {};
        vbd.Usage = D3D11_USAGE_DEFAULT;
        vbd.ByteWidth = sizeof(vertices);
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA vinitData = {};
        vinitData.pSysMem = vertices;

        if (FAILED(m_device->CreateBuffer(&vbd, &vinitData, &m_vertexBuffer))) {
            DebugOutput("Failed to create 2D vertex buffer");
            return false;
        }

        WORD indices[] = { 0,1,2, 0,2,3 };

        D3D11_BUFFER_DESC ibd = {};
        ibd.Usage = D3D11_USAGE_DEFAULT;
        ibd.ByteWidth = sizeof(indices);
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA iinitData = {};
        iinitData.pSysMem = indices;

        if (FAILED(m_device->CreateBuffer(&ibd, &iinitData, &m_indexBuffer))) {
            DebugOutput("Failed to create 2D index buffer");
            return false;
        }

        const char* vsCode =
            "struct VS_IN { float3 pos : POSITION; float4 col : COLOR; };"
            "struct VS_OUT { float4 pos : SV_POSITION; float4 col : COLOR; };"
            "VS_OUT main(VS_IN input) {"
            "    VS_OUT output;"
            "    output.pos = float4(input.pos, 1.0);" 
            "    output.col = input.col;"
            "    return output;"
            "}";

        const char* psCode =
            "struct PS_IN { float4 pos : SV_POSITION; float4 col : COLOR; };"
            "float4 main(PS_IN input) : SV_TARGET { return input.col; }";

        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;

        if (FAILED(D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob))) {
            DebugOutput("Failed to compile 2D vertex shader");
            if (errorBlob) {
                DebugOutput((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            return false;
        }

        if (FAILED(D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob))) {
            DebugOutput("Failed to compile 2D pixel shader");
            if (errorBlob) {
                DebugOutput((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            return false;
        }

        if (FAILED(m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader))) {
            DebugOutput("Failed to create 2D vertex shader");
            return false;
        }

        if (FAILED(m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader))) {
            DebugOutput("Failed to create 2D pixel shader");
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        if (FAILED(m_device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout))) {
            DebugOutput("Failed to create 2D input layout");
            return false;
        }

        vsBlob->Release();
        psBlob->Release();

        DebugOutput("2D Renderer initialized successfully");
        return true;
    }

    void Render() {
        if (!m_device || !m_context) return;

        m_context->VSSetShader(m_vertexShader, nullptr, 0);
        m_context->PSSetShader(m_pixelShader, nullptr, 0);
        m_context->IASetInputLayout(m_inputLayout);

        UINT stride = sizeof(float) * 7; 
        UINT offset = 0;
        m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
        m_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        m_context->DrawIndexed(6, 0, 0);
    }

    void Shutdown() {
        if (m_vertexBuffer) m_vertexBuffer->Release();
        if (m_indexBuffer) m_indexBuffer->Release();
        if (m_vertexShader) m_vertexShader->Release();
        if (m_pixelShader) m_pixelShader->Release();
        if (m_inputLayout) m_inputLayout->Release();
    }

};

Renderer3D g_renderer3D;
Renderer2D g_renderer2D;
RenderToTexture g_renderTexture;

void RenderToTextureScene() {
    g_renderTexture.BeginRender(g_pd3dDeviceContext, 0.2f, 0.2f, 0.2f, 1.0f);

    D3D11_VIEWPORT vp;
    vp.Width = 512.0f;
    vp.Height = 512.0f;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pd3dDeviceContext->RSSetViewports(1, &vp);

    g_renderer2D.Render();

    g_renderTexture.EndRender(g_pd3dDeviceContext);

    D3D11_VIEWPORT mainVp;
    mainVp.Width = (float)g_windowWidth;
    mainVp.Height = (float)g_windowHeight;
    mainVp.MinDepth = 0.0f;
    mainVp.MaxDepth = 1.0f;
    mainVp.TopLeftX = 0;
    mainVp.TopLeftY = 0;
    g_pd3dDeviceContext->RSSetViewports(1, &mainVp);
}

std::string floatToString(float value, int precision) {
    precision = 6;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

void LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->Clear();

    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\arial.ttf", 
        16.0f,
        nullptr,
        io.Fonts->GetGlyphRangesCyrillic()
    );

    if (!font) {
        font = io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\tahoma.ttf",
            16.0f,
            nullptr,
            io.Fonts->GetGlyphRangesCyrillic()
        );
    }

    if (!font) {
        font = io.Fonts->AddFontDefault();
    }

    io.Fonts->Build();

    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui_ImplDX11_InvalidateDeviceObjects();
        ImGui_ImplDX11_CreateDeviceObjects();
    }
}

void DisplayWindowInfo() {
    if (ImGui::Begin("Window Info")) {
        ImGui::Text("Window Size: %dx%d", GetWindowWidth(), GetWindowHeight());
        ImGui::Text("Aspect Ratio: %.3f", GetAspectRatio());
        ImGui::Text("Client Area: %d x %d pixels", GetWindowWidth(), GetWindowHeight());

        ImGui::Separator();
        ImGui::Text("Current Project: %s", g_currentProjectName.c_str());
        ImGui::Text("Project Type: %s",
            g_currentProjectType == PROJECT_2D ? "2D" :
            g_currentProjectType == PROJECT_3D ? "3D" : "None");

        if (ImGui::Button("Print Size to Console")) {
            std::cout << "Current window size: " << GetWindowWidth()
                << "x" << GetWindowHeight()
                << " (Aspect: " << GetAspectRatio() << ")" << std::endl;
        }
    }
    ImGui::End();
}

void CreateProject(const std::string& name, ProjectType type) {
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile("conf.ini");

    std::string key_name = name;
    std::string base_key = key_name;
    int counter = 1;

    while (ini.KeyExists("Projects", key_name.c_str())) {
        key_name = base_key + "_" + std::to_string(counter);
        counter++;
    }

    std::string projectType = (type == PROJECT_2D) ? "2D" : "3D";
    ini.SetValue("Projects", key_name.c_str(), projectType.c_str());

    SI_Error result = ini.SaveFile("conf.ini");

    if (result == SI_OK) {
        g_currentProjectName = key_name;
        g_currentProjectType = type;
        mains = false;
        ui = true;

        std::cout << "Project created: " << key_name << " (" << projectType << ")" << std::endl;
        DebugOutput("Project created: " + key_name + " (" + projectType + ")");
    }
}

void OpenProject(const std::string& name) {
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile("conf.ini");

    if (result == SI_OK) {
        const char* projectType = ini.GetValue("Projects", name.c_str(), "Unknown");

        g_currentProjectName = name;

        if (strcmp(projectType, "2D") == 0) {
            g_currentProjectType = PROJECT_2D;
        }
        else if (strcmp(projectType, "3D") == 0) {
            g_currentProjectType = PROJECT_3D;
        }

        mains = false;
        ui = true;

        std::cout << "Project opened: " << name << " (" << projectType << ")" << std::endl;
        DebugOutput("Project opened: " + name + " (" + projectType + ")");
    }
}

void ShowProjectCreationWindow() {
    if (g_showProjectCreation) {
        ImGui::OpenPopup("Create Project");
        g_showProjectCreation = false;
    }

    if (ImGui::BeginPopupModal("Create Project", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter project name:");
        ImGui::InputText("##projectname", g_projectNameBuffer, IM_ARRAYSIZE(g_projectNameBuffer));

        ImGui::Separator();

        if (ImGui::Button("2D Project", ImVec2(120, 0))) {
            CreateProject(g_projectNameBuffer, PROJECT_2D);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("3D Project", ImVec2(120, 0))) {
            CreateProject(g_projectNameBuffer, PROJECT_3D);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);


    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    std::cout << "=== КОНСОЛЬ АКТИВИРОВАНА ===" << std::endl;
    std::cout << "Current console CP: " << GetConsoleOutputCP() << std::endl;

    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        L"ImGui DX11 Example", nullptr
    };
    ::RegisterClassEx(&wc);

    HWND hwnd = ::CreateWindow(wc.lpszClassName, L":( :) :( :)()()(:)()(:)()(:()()()()()()( bliat",
        WS_OVERLAPPEDWINDOW, 100, 100, g_windowWidth, g_windowHeight,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = NULL;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    LoadFonts();

    if (!g_renderer3D.Initialize(g_pd3dDevice, g_pd3dDeviceContext)) {
        std::cout << "Failed to initialize 3D renderer!" << std::endl;
    }
    else {
        std::cout << "3D Renderer initialized successfully!" << std::endl;
    }

    if (!g_renderer2D.Initialize(g_pd3dDevice, g_pd3dDeviceContext)) {
        std::cout << "Failed to initialize 2D renderer!" << std::endl;
    }
    else {
        std::cout << "2D Renderer initialized successfully!" << std::endl;
    }

    if (!g_renderTexture.Initialize(g_pd3dDevice, 512, 512)) {
        std::cout << "Failed to initialize render texture!" << std::endl;
    }
    else {
        std::cout << "Render texture initialized successfully!" << std::endl;
    }

    bool show_demo_window = false;
    bool show_another_window = false;
    bool show_window_info = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float slider_value = 0.0f;
    int counter = 0;

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        static int frameCount = 0;
        if (frameCount % 60 == 0) { 
            fps = ImGui::GetIO().Framerate;
            frameTime = 1000.0f / fps;
            wchar_t windowTitle[256];
            swprintf_s(windowTitle, L"KERNEL || FPS %.1f || Frame: %.3f ms", fps, frameTime);
            SetWindowText(hwnd, windowTitle);
        }
        frameCount++;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (show_window_info) {
            DisplayWindowInfo();
        }

        ShowProjectCreationWindow();

        if (ui) {
            ImGui::Begin("TOOLS", nullptr, ImGuiWindowFlags_NoTitleBar);
            if (ImGui::Button("ShowGui")) {
                ui = false;
                mains = true;
                g_currentProjectType = PROJECT_NONE;
                g_currentProjectName = "";
            }

            ImGui::SameLine();
            ImGui::Text("Project: %s (%s)",
                g_currentProjectName.c_str(),
                g_currentProjectType == PROJECT_2D ? "2D" : "3D");

            ImGui::Text(u8"привет");
            static int selected_item = 0;
            const char* items[] = { u8"Элемент 1", u8"Элемент 2", u8"Элемент 3", u8"Элемент 4" };

            ImGui::Combo(u8"Выбери элемент", &selected_item, items, IM_ARRAYSIZE(items));

            if (ImGui::Button("Render 3D Scene")) {
                g_renderer3D.Render();
            }

            ImGui::End();
        }
        ImGui::Begin("Render View");

        RenderToTextureScene();

        ID3D11ShaderResourceView* textureView = g_renderTexture.GetShaderResourceView();
        if (textureView) {
            ImVec2 window_size = ImGui::GetContentRegionAvail();
            ImGui::Image(textureView, window_size);
        }
        else {
            ImGui::Text("Texture not available");
        }

        ImGui::End();
        {
            static int counter = 0;

            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(GetWindowWidth(), GetWindowHeight()), ImGuiCond_Always);
            if (mains) {
                ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu(languages ? rus[0] : eng[0]))
                    {
                        if (ImGui::BeginMenu("New")) {
                            static char buf[128] = "Project";

                            ImGui::PushItemWidth(200);
                            ImGui::InputText("##filename", buf, IM_ARRAYSIZE(buf));
                            ImGui::PopItemWidth();

                            if (ImGui::Button(languages ? rus[0] : eng[0])) {
                                strcpy_s(g_projectNameBuffer, buf);
                                g_showProjectCreation = true;
                                try {
                                    std::filesystem::path path = std::filesystem::path("Projects") / buf;
                                    std::filesystem::create_directories(path);
                                    std::cout << "yes sir" << std::endl;
                                }
                                catch (const std::filesystem::filesystem_error& ex) {
                                    std::cout << "Ошибка " << ex.what() << "\n";
                                }
                            }
                            if (gui::Button("hello")) {
                                try {
                                    std::filesystem::create_directory("Projects");
                                    std::cout << "Папка создана успешно!\n";
                                }
                                catch (const std::filesystem::filesystem_error& ex) {
                                    std::cout << "Ошибка: " << ex.what() << "\n";
                                }

                            }

                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu(languages ? rus[1] : eng[1], "Ctrl+O")) {
                            CSimpleIniA ini;
                            ini.SetUnicode();
                            SI_Error result = ini.LoadFile("conf.ini");
                            if (result == SI_OK) {
                                CSimpleIniA::TNamesDepend keys;
                                ini.GetAllKeys("Projects", keys);
                                int count1 = keys.size();
                                std::cout << "Digital: " << count1 << std::endl;

                                for (const auto& key : keys) {
                                    const char* projectType = ini.GetValue("Projects", key.pItem, "Unknown");
                                    std::string menuLabel = std::string(key.pItem) + " (" + projectType + ")";

                                    if (ImGui::MenuItem(menuLabel.c_str())) {
                                        OpenProject(key.pItem);
                                    }
                                }
                            }
                            else {
                                ImGui::Text("Файл conf.ini не найден");
                            }
                            ImGui::EndMenu();
                        }
                        if (ImGui::MenuItem(languages ? rus[2] : eng[2], "Ctrl+N")) {
                            counter = 0;
                        }
                        if (ImGui::MenuItem(languages ? rus[4] : eng[4], "Ctrl+S")) {
                            // Действие
                        }

                        ImGui::Separator();

                        if (ImGui::MenuItem(languages ? rus[5] : eng[5])) {
                            PostQuitMessage(0);
                        }

                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu(languages ? rus[6] : eng[6]))
                    {
                        if (ImGui::MenuItem(languages ? rus[7] : eng[7], "Ctrl+C")) { 

                        }
                        if (ImGui::MenuItem(languages ? rus[8] : eng[8], "Ctrl+V")) { 

                        }
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Debug"))
                    {
                        ImGui::MenuItem("Demo Window", NULL, &show_demo_window);
                        ImGui::MenuItem("Another Window", NULL, &show_another_window);
                        ImGui::MenuItem("Window Info", NULL, &show_window_info);
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Render"))
                    {
                        if (ImGui::MenuItem("3D Renderer")) {
                            g_renderer3D.Render();
                        }
                        if (ImGui::MenuItem("2D Renderer")) {
                            g_renderer2D.Render();
                        }
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu(languages ? rus[9] : eng[9]))
                    {
                        show_another_window = true;
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("App")) {
                        if (ImGui::BeginMenu("Settings")) {
                            show_settings = true;
                            ImGui::EndMenu();
                        }
                        ImGui::EndMenu();
                    }
                    
                    ImGui::EndMenuBar();
                }

               
                ImGui::Text(u8"Это тест ImGui с DirectX11 и 3D/2D рендерингом!");
                ImGui::Checkbox("Demo Window", &show_demo_window);
                ImGui::Checkbox("Another Window", &show_another_window);
                ImGui::Checkbox("Window Info", &show_window_info);

                ImGui::SliderFloat("float", &slider_value, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);
                if (ImGui::Button("Button"))
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Separator();
                ImGui::Text("Renderer Status:");
                ImGui::Text("Current Project: %s", g_currentProjectName.empty() ? "None" : g_currentProjectName.c_str());
                ImGui::Text("Project Type: %s",
                    g_currentProjectType == PROJECT_2D ? "2D" :
                    g_currentProjectType == PROJECT_3D ? "3D" : "None");
                ImGui::Text("Window: %dx%d", GetWindowWidth(), GetWindowHeight());

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }
        }

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        if (show_settings) {
            ImGui::SetNextWindowFocus();
            gui::Begin("Settings");
            ImGui::Text(u8"Язык:");
            if (ImGui::RadioButton("English", !languages)) {
                languages = false;
            }
            if (ImGui::RadioButton(u8"Русский", languages)) {
                languages = true;
            }
            gui::End();
        }

        if (show_another_window)
        {
            ImGui::Begin(languages ? rus[9] : eng[9], &show_another_window);
            ImGui::Text("Hello from another window!");
            ImGui::Text("Current window size: %dx%d", GetWindowWidth(), GetWindowHeight());
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        ImGui::Render();

        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);

        if (ui) {
            if (g_currentProjectType == PROJECT_3D) {
                g_renderer3D.Render();
            }
            else if (g_currentProjectType == PROJECT_2D) {
                g_renderer2D.Render();
            }
        }

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); 
    }

    g_renderer3D.Shutdown();
    g_renderer2D.Shutdown();
    g_renderTexture.Shutdown();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = g_windowWidth;
    sd.BufferDesc.Height = g_windowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd, &g_pSwapChain,
        &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer)
    {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void DebugOutput(const std::string& message) {
    OutputDebugStringA((message + "\n").c_str());
    std::cout << message << std::endl;
}

void HandleResize(UINT newWidth, UINT newHeight) {
    g_windowWidth = newWidth;
    g_windowHeight = newHeight;

    CleanupRenderTarget();
    if (g_pSwapChain) {
        g_pSwapChain->ResizeBuffers(0, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, 0);
        CreateRenderTarget();
    }

    DebugOutput("Window resized to: " + std::to_string(newWidth) + "x" + std::to_string(newHeight));
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            UINT newWidth = (UINT)LOWORD(lParam);
            UINT newHeight = (UINT)HIWORD(lParam);
            HandleResize(newWidth, newHeight);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}