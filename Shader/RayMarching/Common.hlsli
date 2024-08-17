#ifndef __COMMON_RAYMARCHING__
#define __COMMON_RAYMARCHING__

struct Camera
{
	float3 position;
	float3 direction;
	float3 up;

	float3 GetRay(const float2 uv)
	{
		const float3 side = cross(direction, up);
		return normalize(side * uv.x + up * uv.y + direction);
	}
};

#endif