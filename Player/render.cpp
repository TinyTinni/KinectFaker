#include "render.h"

#include "reader.h"

#include <cstring> //strlen
#include <memory>

#include <d3dcompiler.h>

constexpr char* vertex_shader = R"(
float4 main(float3 pos : POSITION): SV_POSITION
{
    return float4(pos,1.0);
}

)";

constexpr char* pixel_shader = R"(
float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
    return float4(0.0, 1.0, 0.0, 1.0);
}

)";


const auto safe_rel = [](auto p) {if (p) p->Release(); };

ID3D11VertexShader* g_vertex_shader = nullptr;
ID3D11PixelShader*  g_pixel_shader  = nullptr;
ID3D11InputLayout*  g_shader_layout = nullptr;
ID3D11Buffer*       g_v_buffer      = nullptr;
std::unique_ptr<Reader> g_kinect;

void Alloc_Ressources(ID3D11Device * device, ID3D11DeviceContext * context)
{
    HRESULT hr;
    ID3D10Blob* shaderblob;
    ID3D10Blob* errorblob;

    // create vertex shader
    {
        hr = D3DCompile(vertex_shader, strlen(vertex_shader), "VS", nullptr, nullptr, "main", "vs_4_0", 0, 0, &shaderblob, &errorblob);

        if (FAILED(hr))
            OutputDebugStringA((char*)errorblob->GetBufferPointer());

        hr = device->CreateVertexShader(shaderblob->GetBufferPointer(), shaderblob->GetBufferSize(), nullptr, &g_vertex_shader);

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        UINT numElements = ARRAYSIZE(layout);

        // Create the input layout
        hr = device->CreateInputLayout(layout, numElements, shaderblob->GetBufferPointer(), shaderblob->GetBufferSize(), &g_shader_layout);
        context->IASetInputLayout(g_shader_layout);

        if (FAILED(hr))
            OutputDebugStringA((char*)errorblob->GetBufferPointer());


        safe_rel(shaderblob);
        safe_rel(errorblob);

    }

    // create pixel shader
    {
        hr = D3DCompile(pixel_shader, strlen(pixel_shader), "PS", nullptr, nullptr, "main", "ps_4_0", 0, 0, &shaderblob, &errorblob);

        if (FAILED(hr))
            OutputDebugStringA((char*)errorblob->GetBufferPointer());

        hr = device->CreatePixelShader(shaderblob->GetBufferPointer(), shaderblob->GetBufferSize(), nullptr, &g_pixel_shader);
        if (FAILED(hr))
            OutputDebugStringA((char*)errorblob->GetBufferPointer());

        safe_rel(shaderblob);
        safe_rel(errorblob);

    }

    // create vertex buffer
    constexpr int buffersize = 2*3 * skeleton_joints;
    constexpr float vertex_pos[buffersize] = {
        0.f,0.f,0.f,
        -0.0f,-0.5f,0.f,
        1.f,1.f,1.f,
        0.f,0.f,0.f
        //...
    };

    D3D11_BUFFER_DESC buff_desc;
    ZeroMemory(&buff_desc, sizeof(D3D11_BUFFER_DESC));
    buff_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buff_desc.Usage = D3D11_USAGE_DYNAMIC;
    buff_desc.ByteWidth = buffersize*sizeof(float);
    buff_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA data;
    ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
    data.pSysMem = vertex_pos;
    device->CreateBuffer(&buff_desc, &data, &g_v_buffer);

    g_kinect.reset(new Reader());
}

bool Render(ID3D11Device * device, ID3D11DeviceContext * context)
{

    context->IASetInputLayout(g_shader_layout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);


    D3D11_MAPPED_SUBRESOURCE resource;
    context->Map(g_v_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    
    const bool newframe = g_kinect->get_skeleton_position((float*)resource.pData);
    context->Unmap(g_v_buffer, 0);
    if (!newframe)
        return false;

    unsigned stride = 3 * sizeof(float), offset = 0;
    context->IASetVertexBuffers(0, 1, &g_v_buffer, &stride, &offset);

    context->VSSetShader(g_vertex_shader, nullptr, 0);
    context->PSSetShader(g_pixel_shader, nullptr, 0);

    context->Draw(2*skeleton_joints, 0);
    return true;
}


void Finish()
{
    safe_rel(g_pixel_shader);
    safe_rel(g_vertex_shader);
    safe_rel(g_shader_layout);
    safe_rel(g_v_buffer);
}
