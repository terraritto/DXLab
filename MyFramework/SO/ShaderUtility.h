#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <wrl.h>
#include "../../include/d3dx12.h"

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

struct IDxcBlob;

// ファイルの読み込み
bool LoadFile(std::vector<char>& out, const std::wstring& fileName);

// 通常のShader読み込み(DXRはx)
D3D12_SHADER_BYTECODE LoadShaderForRaster(std::wstring filename, std::vector<char>& outdata);

// Shaderをコンパイルする
bool CompileShader(const std::wstring fileName, const std::wstring profile, Microsoft::WRL::ComPtr<IDxcBlob>& byte, Microsoft::WRL::ComPtr<IDxcBlob>& errorBlob);
bool CompileShaderFromLibrary(const std::wstring fileName, const std::wstring profile, Microsoft::WRL::ComPtr<ID3DBlob>& byte, Microsoft::WRL::ComPtr<ID3DBlob>& errorBlob);
bool CompileD3DShader(const std::wstring fileName, const std::string profile, Microsoft::WRL::ComPtr<ID3DBlob>& byte, Microsoft::WRL::ComPtr<ID3DBlob>& errorBlob);
