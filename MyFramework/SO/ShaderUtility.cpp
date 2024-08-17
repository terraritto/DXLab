#include "ShaderUtility.h"
#include <filesystem>
#include <dxcapi.h>
#include <d3dcompiler.h>
#include <d3d12shader.h>
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "d3dcompiler.lib")

bool LoadFile(std::vector<char>& out, const std::wstring& fileName)
{
    std::ifstream infile(fileName, std::ios::binary);
    if (!infile) {
        return false;
    }

    out.resize(infile.seekg(0, std::ios::end).tellg());
    infile.seekg(0, std::ios::beg).read(out.data(), out.size());
    return true;
}

D3D12_SHADER_BYTECODE LoadShaderForRaster(std::wstring filename, std::vector<char>& outdata)
{
    // 削除しておく
    outdata.clear();

    if (!LoadFile(outdata, filename))
    {
        throw std::runtime_error("shader file not found");
    }

    D3D12_SHADER_BYTECODE result;
    result = CD3DX12_SHADER_BYTECODE(outdata.data(), outdata.size());
    return result;
}

bool CompileShader(const std::wstring fileName, const std::wstring profile, Microsoft::WRL::ComPtr<IDxcBlob>& byte, Microsoft::WRL::ComPtr<IDxcBlob>& errorBlob)
{
    using namespace std::filesystem;

    HRESULT hr;

    // reference
    // https://simoncoenen.com/blog/programming/graphics/DxcCompiling
    ComPtr<IDxcUtils> utils;
    hr = ::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

    ComPtr<IDxcBlobEncoding> source;
    utils->LoadFile(fileName.c_str(), nullptr, &source);

    if (source == NULL)
    {
        throw std::runtime_error("shader not found");
    }

    ComPtr<IDxcCompiler3> compiler;
    hr = ::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    
    // argumentの構築
    std::vector<LPCWSTR> arguments;
    // エントリーポイント
    arguments.push_back(L"-E");
    arguments.push_back(L"main");

    // プロファイル(ps_6_0とかそういうの)
    arguments.push_back(L"-T");
    arguments.push_back(profile.c_str());

    arguments.push_back(DXC_ARG_DEBUG); //-Zi
    arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL0); // -O0

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = source->GetBufferPointer();
    sourceBuffer.Size = source->GetBufferSize();
    sourceBuffer.Encoding = 0;

    ComPtr<IDxcResult> compileResult;

    // デフォルトのinclude handler
    ComPtr<IDxcIncludeHandler> includeHandler;
    hr = utils->CreateDefaultIncludeHandler(&includeHandler);

    hr = compiler->Compile(&sourceBuffer, arguments.data(), (UINT)arguments.size(), *includeHandler.GetAddressOf(), IID_PPV_ARGS(compileResult.GetAddressOf()));

    // エラー処理
    ComPtr<IDxcBlobUtf8> errors{};
    ComPtr<IDxcBlobUtf16> outputName{};
    compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr);
    if (errors && errors->GetStringLength() > 0)
    {
        const char* errorText = static_cast<const char*>(errors->GetBufferPointer());
        OutputDebugStringA(errorText);
        return false;
    }

    compileResult->GetStatus(&hr);

    if (SUCCEEDED(hr))
    {
        compileResult->GetResult(&byte);
    }
    else
    {
        compileResult->GetErrorBuffer(
            reinterpret_cast<IDxcBlobEncoding**>(errorBlob.GetAddressOf())
        );
    }

    return SUCCEEDED(hr);
}

bool CompileShaderFromLibrary(const std::wstring fileName, const std::wstring profile, Microsoft::WRL::ComPtr<ID3DBlob>& byte, Microsoft::WRL::ComPtr<ID3DBlob>& errorBlob)
{
    using namespace std::filesystem;

    path filePath(fileName);
    std::ifstream infile(filePath, std::ifstream::binary);
    std::vector<char> srcData;
    if (!infile)
    {
        throw std::runtime_error("shader not found");
    }

    // 読み込み
    srcData.resize(uint32_t(infile.seekg(0, infile.end).tellg()));
    infile.seekg(0, infile.beg).read(srcData.data(), srcData.size());

    // DXC によるコンパイル処理
    ComPtr<ID3DBlob> shaderBlob;

    ComPtr<IDxcLibrary> library;
    ComPtr<IDxcCompiler> compiler;
    ComPtr<IDxcBlobEncoding> source;
    ComPtr<IDxcOperationResult> dxcResult;

    DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
    library->CreateBlobWithEncodingFromPinned(srcData.data(), UINT(srcData.size()), CP_ACP, &source);
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

    LPCWSTR compilerFlags[] = {
    #if _DEBUG
        L"/Zi", L"/O0"
    #else
        L"/O2" // リリースビルドでは最適化
    #endif
    };

    HRESULT hr;

    compiler->Compile(source.Get(), filePath.wstring().c_str(),
        L"main", profile.c_str(),
        compilerFlags, _countof(compilerFlags),
        nullptr, 0,
        nullptr,
        &dxcResult);

    dxcResult->GetStatus(&hr);
    if (SUCCEEDED(hr))
    {
        dxcResult->GetResult(
            reinterpret_cast<IDxcBlob**>(byte.GetAddressOf())
        );
    }
    else
    {
        dxcResult->GetErrorBuffer(
            reinterpret_cast<IDxcBlobEncoding**>(errorBlob.GetAddressOf())
        );
    }

    return SUCCEEDED(hr);
}

bool CompileD3DShader(const std::wstring fileName, const std::string profile, Microsoft::WRL::ComPtr<ID3DBlob>& byte, Microsoft::WRL::ComPtr<ID3DBlob>& errorBlob)
{
    using namespace std::filesystem;

    path filePath(fileName);
    std::ifstream infile(filePath, std::ifstream::binary);
    std::vector<char> srcData;
    if (!infile)
    {
        throw std::runtime_error("shader not found");
    }

    // 読み込み
    srcData.resize(uint32_t(infile.seekg(0, infile.end).tellg()));
    infile.seekg(0, infile.beg).read(srcData.data(), srcData.size());

    UINT compileFlags = 0;
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    
    // ファイルからコンパイル
    HRESULT hr = D3DCompileFromFile(
        filePath.wstring().c_str(), nullptr, nullptr,
        "main", profile.c_str(),
        compileFlags, 0, byte.GetAddressOf(), errorBlob.GetAddressOf());

    return SUCCEEDED(hr);
}
