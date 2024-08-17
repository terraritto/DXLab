#include "Shader/RayMarching/Common.hlsli"
#include "Shader/RayMarching/Intersection.hlsli"

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

#define SPHERE_SIZE 1.0f

float4 main(VSOutput In) : SV_TARGET
{
	Camera camera;
	camera.position = float3(0.0f, 0.0f, 5.0f);
	camera.up = float3(0.0f, 1.0f, 0.0f);
	camera.direction = float3(0.0f, 0.0f, -2.0f);

	float2 uv = In.UV * 2.0f - 1.0f;

	const float3 ray = camera.GetRay(uv);

	float distance = 0.0f;
	float3 rayPos = camera.position;
	float4 color = float4(0.0f,0.0f,0.0f,1.0f);

	float3 lightDir = normalize(float3(-1.0f, -1.0f, 0.0f));
	float lightIntensity = 10.0f;
	float3 ambientColor = float3(0.0f, 0.1f, 0.0f);
	float3 diffuseColor = float3(1.0f, 0.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		// Update Distance.
		distance = SDFSphere(rayPos, SPHERE_SIZE); 

		// calculate Normal and Shading
		if (distance < 0.001f)
		{
			float3 normal = GetSDFSphereNormal(rayPos, SPHERE_SIZE);
			float lambert = saturate(dot(normal, lightDir));
			color.xyz = (diffuseColor * lambert) * lightIntensity + ambientColor;
			break;
		}

		rayPos += ray * distance;
	}

	return color;
}
