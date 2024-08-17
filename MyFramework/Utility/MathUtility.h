#pragma once
#include <wrl.h>
#include <DirectXMath.h>

namespace Utility
{
	using Microsoft::WRL::ComPtr;
	
	constexpr float PI = DirectX::XM_PI;

	// align‚ð‘µ‚¦‚é
	inline UINT RoundUp(size_t size, UINT align)
	{
		return UINT(size + align - 1) & ~(align - 1);
	}
}