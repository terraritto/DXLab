#include "../Constant.hlsli"

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

Texture2D colorTexture : register(t0);

cbuffer param : register(b0)
{
    float2 texelSize : packoffset(c0);
    float sigma : packoffset(c0.z);
    int index;
}

SamplerState samp : register(s0);

/*
https://casual-effects.com/research/McGuire2008Median/index.html
3x3 Median
Morgan McGuire and Kyle Whitson
http://graphics.cs.williams.edu
Copyright (c) Morgan McGuire and Williams College, 2006
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Change these 2 defines to change precision,
#define vec vec3
#define toVec(x) x.rgb

//#define vec vec4
//#define toVec(x) x.rgba

#define s2(a, b)				temp = a; a = min(a, b); b = max(temp, b);
#define mn3(a, b, c)			s2(a, b); s2(a, c);
#define mx3(a, b, c)			s2(b, c); s2(a, c);

#define mnmx3(a, b, c)			mx3(a, b, c); s2(a, b);                                   // 3 exchanges
#define mnmx4(a, b, c, d)		s2(a, b); s2(c, d); s2(a, c); s2(b, d);                   // 4 exchanges
#define mnmx5(a, b, c, d, e)	s2(a, b); s2(c, d); mn3(a, c, e); mx3(b, d, e);           // 6 exchanges
#define mnmx6(a, b, c, d, e, f) s2(a, d); s2(b, e); s2(c, f); mn3(a, b, c); mx3(d, e, f); // 7 exchanges

// Gaussian
float gauss(float x, float y, float sigma)
{
    float exponent = exp(-(x * x + y * y) / (2.0f * sigma * sigma));
    float denom = 2.0 * PI * sigma * sigma;
    return exponent / denom;
}

float4 main(VSOutput In) : SV_TARGET
{
    float3 color = float3(0.0f, 0.0f, 0.0f);

    switch (index)
    {
    case 0:
        {
            // 平均フィルタ(3x3)
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    float2 uv = In.UV + texelSize * float2(i, j);
                    color += colorTexture.Sample(samp, uv).rgb;
                }
            }
            color /= 9.0f;
        }
        break;
        
    case 1:
        {
            // 中間値ブラー
            float3 v[9];

            for (int dX = -1; dX <= 1; ++dX)
            {
                for (int dY = -1; dY <= 1; ++dY)
                {
                    float2 uv = In.UV + texelSize * float2(dX, dY);
                    color += colorTexture.Sample(samp, uv).rgb;
                    v[(dX + 1) * 3 + (dY + 1)] = colorTexture.Sample(samp, uv).rgb;
                }
            }

            float3 temp;

            // Starting with a subset of size 6, remove the min and max each time
            mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
            mnmx5(v[1], v[2], v[3], v[4], v[6]);
            mnmx4(v[2], v[3], v[4], v[7]);
            mnmx3(v[3], v[4], v[8]);
            color = v[4];
        }
        break;
        
    case 2:
        {
            // ガウシアンブラー
            float total = 0.0;
            for (int i = -3; i <= 3; i++)
            {
                for (int j = -3; j <= 3; j++)
                {
                    float2 uv = In.UV + texelSize * float2(i, j);
                    float weight = gauss(i, j, sigma);
                    color += colorTexture.Sample(samp, uv).rgb * weight;
                    total += weight;
                }
            }
            color *= (1.0f / total);
        }
        break;
        
    default:
        {
            color = colorTexture.Sample(samp, In.UV).rgb;
        }
        break;
    }
        
    return float4(color, 1.0f);
}
