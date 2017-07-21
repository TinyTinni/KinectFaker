#pragma once

#include <Windows.h>
#include <d3d11.h>

void Alloc_Ressources(ID3D11Device* device, ID3D11DeviceContext* context);
bool Render(ID3D11Device* device, ID3D11DeviceContext* context);
void Finish();