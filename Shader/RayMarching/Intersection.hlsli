#ifndef __INTERSECTION_RAYMARCHING__
#define __INTERSECTION_RAYMARCHING__

float SDFSphere(float3 p, float s)
{
	return length(p) - s;
}

float3 GetSDFSphereNormal(float3 p, float s)
{
	float ep = 0.0001f;
	return normalize(float3(
		SDFSphere(p, s) - SDFSphere(float3(p.x - ep, p.yz), s),
		SDFSphere(p, s) - SDFSphere(float3(p.x, p.y - ep, p.z), s),
		SDFSphere(p, s) - SDFSphere(float3(p.xy, p.z - ep), s)
	));
}

float SDFBox(float3 p, float3 b)
{
	float3 q = abs(p) - b;
	return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);
}

float SDFRoundBox(float3 p, float3 b, float r)
{
	float3 q = abs(p) - b;
	return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f) - r;
}

float SDFBoxFrame(float3 p, float3 b, float e)
{
	p = abs(p) - b;
	float3 q = abs(p + e) - e;
	return min(min(
	length(max(float3(p.x,q.y,q.z), 0.0f)) + min(max(p.x, max(q.y,q.z)),0.0f),
	length(max(float3(q.x,p.y,q.z), 0.0f)) + min(max(q.x, max(p.y,q.z)),0.0f)),
	length(max(float3(q.x,q.y,p.z), 0.0f)) + min(max(q.x, max(q.y,p.z)),0.0f));
}

float SDFTorus(float3 p, float2 t)
{
	float2 q = float2(length(p.xz) - t.x, p.y);
	return length(q) - t.y;
}

float SDFCappedTorus(float3 p, float2 sc, float ra, float rb)
{
	p.x = abs(p.x);
	float k = (sc.y * p.x > sc.x * p.y) ? dot(p.xy, sc) : length(p.xy);
	return sqrt(dot(p,p) + ra * ra - 2.0f * ra * k) - rb;
}

float SDFLink(float3 p, float le, float r1, float r2)
{
	float3 q = float3(p.x, max(abs(p.y) - le, 0.0f), p.z);
	return length(float2(length(q.xy) - r1, q.z)) - r2;
}

float SDFCylinder(float3 p, float3 c)
{
	return length(p.xz - c.xy) - c.z;
}

float SDFCone(float3 p, float2 c, float h)
{
	float2 q = h * float2(c.x / c.y, -1.0f);

	float2 w = float2(length(p.xz), p.y);
	float2 a = w - q * clamp(dot(w, q) / dot(q, q), 0.0f, 1.0f);
	float2 b = w - q * float2(clamp(w.x / q.x, 0.0f, 1.0f), 1.0f);
	float k = sign(q.y);
	float d = min(dot(a, a), dot(b, b));
	float s = max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));
	return sqrt(d) * sign(s);
}

float SDFConeBound(float3 p, float2 c, float h)
{
	float q = length(p.xz);
	return max(dot(c.xy, float2(q, p.y)), -h - p.y);
}

float SDFConeInfinite(float3 p, float2 c)
{
	float2 q = float2(length(p.xz), -p.y);
	float d = length(q - c * max(dot(q, c), 0.0f));
	return d * ((q.x * c.y - q.y * c.x < 0.0f) ? -1.0f : 1.0f);
}

float SDFPlane(float3 p, float3 n, float h)
{
	return dot(p, n) + h; // n must be normalized
}

float SDFHexPrism(float3 p, float2 h)
{
	const float3 k = float3(-0.8660254, 0.5, 0.57735);
	p = abs(p);
	p.xy -= 2.0 * min(dot(k.xy, p.xy), 0.0f) * k.xy;
	float2 d = float2
	(
		length(p.xy - float2(clamp(p.x, -k.x*h.x, k.z*h.x), h.x)) * sign(p.y - h.x),
		p.z - h.y
	);
	return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f));
}

float SDFTriPrism(float3 p, float2 h)
{
	float3 q = abs(p);
	return max(q.z - h.y, max(q.x*0.866025 + p.y * 0.5, -p.y) - h.x * 0.5f);
}

float SDFCapsule(float3 p, float3 a, float3 b, float r)
{
	float3 pa = p - a;
	float3 ba = b - a;
	float h = clamp(dot(pa,pa) / dot(ba,ba), 0.0f, 1.0f);
	return length(pa - ba * h) - r;
}

float SDFVerticalCapsule(float3 p, float h, float r)
{
	p.y -= clamp(p.y, 0.0f, h);
	return length(p) - r;
}

float SDFCappedCylinder(float3 p, float h, float r)
{
	float2 d = abs(float2(length(p.xz), p.y)) - float2(r, h);
	return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f));
}

float SDFCappedCylinderExact(float3 p, float3 a, float3 b, float r)
{
	float3 ba = b - a;
	float3 pa = p - a;
	float baba = dot(ba,ba);
	float paba = dot(pa,ba);
	float x = length(pa * baba - ba * paba) - r * baba;
	float y = abs(paba - baba * 0.5f) - baba * 0.5f;
	float x2 = x * x;
	float y2 = y * y * baba;
	float d = (max(x,y) < 0.0f) ? -min(x2, y2) : (((x > 0.0f) ? x2 : 0.0f) + ((y > 0.0f) ? y2 : 0.0f));
	return sign(d) * sqrt(abs(d)) / baba;
}

float SDFRoundCylinder(float3 p, float ra, float rb, float h)
{
	float2 d = float2(length(p.xz) - 2.0f * ra + rb, abs(p.y) - h);
	return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f)) - rb;
}

float SDFCappedCone(float3 p, float h, float r1, float r2)
{
	float2 q = float2(length(p.xz), p.y);
	float2 k1 = float2(r2, h);
	float2 k2 = float2(r2-r1, 2.0f * h);
	float2 ca = float2(q.x - min(q.x, (q.y < 0.0f) ? r1 : r2), abs(q.y) - h);
	float2 cb = q - k1 + k2 * clamp(dot(k1 - q, k2) / dot(k2, k2), 0.0f, 1.0f);
	float s = (cb.x < 0.0f && ca.y < 0.0f) ? -1.0f : 1.0f;
	return s * sqrt(min(dot(ca, ca), dot(cb, cb)));
}

float SDFCappedCone(float3 p, float3 a, float3 b, float ra, float rb)
{
	float rba = rb - ra;
	float baba = dot(b - a, b - a);
	float papa = dot(p - a, p - a);
	float paba = dot(p - a, b - a) / baba;
	float x = sqrt(papa - paba * paba * baba);
	float cax = max(0.0f, x - ((paba < 0.5f) ? ra : rb));
	float cay = abs(paba - 0.5f) - 0.5f;
	float k = rba * rba + baba;
	float f = clamp((rba * (x - ra) + paba * baba) / k, 0.0f, 1.0f);
	float cbx = x - ra - f * rba;
	float cby = paba - f;
	float s = (cbx < 0.0f && cay < 0.0f) ? -1.0f : 1.0f;
	return s * sqrt(min(cax * cax + cay * cay * baba, cbx * cbx + cby * cby * baba));
}

float SDFSolidAngle(float3 p, float2 c, float ra)
{
	float2 q = float2(length(p.xz), p.y);
	float l = length(q) - ra;
	float m = length(q - c * clamp(dot(q,c), 0.0f, ra));
	return max(l, m * sign(c.y * q.x - c.x * q.y));
}

float SDFCutSphere(float3 p, float r, float h)
{
	float w = sqrt(r * r - h * h);
	float2 q = float2(length(p.xz), p.y);
	float s = max((h - r) * q.x * q.x + w * w * (h + r - 2.0f * q.y), h * q.x - w * q.y);
	return (s < 0.0f) ? length(q) - r : (q.x < w) ? h - q.y : length(q - float2(w, h));
}

float SDFCutHollowSphere(float3 p, float r, float h, float t)
{
	float w = sqrt(r * r - h * h);
	float2 q = float2(length(p.xz), p.y);
	return ((h * q.x < w * q.y) ? length(q - float2(w, h)) : abs(length(q) - r)) - t;
}

float SDFDeathStar(float3 p2, float ra, float rb, float d)
{
	float a = (ra * ra - rb * rb + d * d) / (2.0f * d);
	float b = sqrt(max(ra * ra - a * a, 0.0f));
	float2 p = float2(p2.x, length(p2.yz));
	if (p.x * b - p.y * a > d * max(b - p.y, 0.0f))
	{
		return length(p - float2(a, b));
	}
	else
	{
		return max((length(p) - ra), -(length(p - float2(d, 0.0f)) - rb));
	}
}

float SDFRoundCone(float3 p, float r1, float r2, float h)
{
	float b = (r1 - r2) / h;
	float a = sqrt(1.0f - b * b);
	float2 q = float2(length(p.xz), p.y);
	float k = dot(q, float2(-b, a));
	if(k < 0.0f) return length(q) - r1;
	if(k > a * h) return length(q - float2(0.0f, h)) - r2;
	return dot(q, float2(a,b)) - r1;
}

float SDFRoundCone(float3 p, float3 a, float3 b, float r1, float r2)
{
	float3 ba = b - a;
	float l2 = dot(ba, ba);
	float rr = r1 - r2;
	float a2 = l2 - rr * rr;
	float il2 = 1.0f / l2;
	
	float3 pa = p - a;
	float y = dot(pa, ba);
	float z = y - l2;
	float3 x = pa * l2 - ba * y;
	float x2 = dot(x, x);
	float y2 = y * y * l2;
	float z2 = z * z * l2;

	float k = sign(rr) * rr * rr * x2;
	if(sign(z) * a2 * z2 > k) return sqrt(x2 + z2) * il2 - r2;
	if(sign(y) * a2 * y2 < k) return sqrt(x2 + y2) * il2 - r1;
	return (sqrt(x2 * a2 * il2) + y * rr) * il2 - r1;
}

float SDFEllipsoid(float3 p, float r)
{
	float k0 = length(p / r);
	float k1 = length(p / (r * r));
	return k0 * (k0 - 1.0f) / k1;
}

float SDFVesicaSegment(float3 p, float3 a, float3 b, float w)
{
	float3 c = (a + b) * 0.5f;
	float l = length(b - a);
	float3 v = (b - a) / l;
	float y = dot(p - c, v);
	float2 q = float2(length(p - c - y * v), abs(y));

	float r = 0.5f * l;
	float d = 0.5f * (r * r - w * w) / w;
	float3 h = (r * q.x < d * (q.y - r)) ? float3(0.0f, r, 0.0f) : float3(-d, 0.0f, d + w);

	return length(q - h.xy) - h.z;
}

float SDFRhombus(float3 p, float la, float lb, float h, float ra)
{
	p = abs(p);
	float2 b = float2(la, lb);
	float f = clamp((dot(b, b - 2.0f * p.xz)) / dot(b, b), -1.0f, 1.0f);
	float2 q = float2(length(p.xz - 0.5f * b * float2(1.0f - f, 1.0f + f)) * sign(p.x * b.y + p.z * b.x - b.x * b.y) - ra, p.y - h);
	return min(max(q.x, q.y), 0.0f) + length(max(q, 0.0f));
}

float SDFOctahedron(float3 p, float s)
{
	p = abs(p);
	float m = p.x + p.y + p.z - s;
	float3 q;

	if (3.0f * p.x < m) q = p.xyz;
	else if (3.0f * p.y < m) q = p.yzx;
	else if (3.0f * p.z < m) q = p.zxy;
	else return m * 0.57735027;

	float k = clamp(0.5f * (q.z - q.y + s), 0.0f, s);
	return length(float3(q.x, q.y - s + k, q.z - k));
}

float SDFOctahedronBound(float3 p, float s)
{
	p = abs(p);
	return (p.x + p.y + p.z - s) * 0.57735027;
}

float SDFPyramid(float3 p, float h)
{
	float m2 = h * h + 0.25f;
	
	p.xz = abs(p.xz);
	p.xz = (p.z > p.x) ? p.zx : p.xz;
	p.xz -= 0.5f;

	float3 q = float3(p.z, h * p.y - 0.5f * p.x, h * p.x + 0.5 * p.y);

	float s = max(-q.x, 0.0f);
	float t = clamp((q.y - 0.5f * p.z) / (m2 + 0.25f), 0.0f, 1.0f);

	float a = m2 * (q.x + s) * (q.x + s) + q.y * q.y;
	float b = m2 * (q.x + 0.5f * t) * (q.x + 0.5f * t) + (q.y - m2 * t) * (q.y - m2 * t);

	float d2 = min(q.y, -q.x * m2 - q.y * 0.5) > 0.0f ? 0.0f : min(a, b);
	
	return sqrt((d2 + q.z * q.z) / m2) * sign(max(q.z, -q.y));
}

float UDFTriangle(float3 p, float3 a, float3 b, float3 c)
{
	float3 ba = b - a; float3 pa = p - a;
	float3 cb = c - a; float3 pb = p - b;
	float3 ac = a - c; float3 pc = p - c;
	float3 nor = cross(ba, ac);

	float3 r1 = ba * clamp(dot(ba, pa) / dot(ba, ba), 0.0f, 1.0f) - pa;
	float3 r2 = cb * clamp(dot(cb, pb) / dot(cb, cb), 0.0f, 1.0f) - pb;
	float3 r3 = ac * clamp(dot(ac, pc) / dot(ac, ac), 0.0f, 1.0f) - pc;

	return sqrt(
		(
			sign(dot(cross(ba, nor), pa)) + 
			sign(dot(cross(cb, nor), pb)) +
			sign(dot(cross(ac, nor), pc)) < 2.0f
		) ?
		min(min(
			dot(r1, r1),
			dot(r2, r2)),
			dot(r3, r3))
		:
		(dot(nor, pa) * dot(nor, pa) / dot(nor, nor))
	);
}

float SDFQuad(float3 p, float3 a, float3 b, float3 c, float3 d)
{
	float3 ba = b - a; float3 pa = p - a;
	float3 cb = c - a; float3 pb = p - b;
	float3 dc = d - c; float3 pc = p - c;
	float3 ad = a - d; float3 pd = p - d;
	float3 nor = cross(ba, ad);

	float3 r1 = ba * clamp(dot(ba, pa) / dot(ba, ba), 0.0f, 1.0f) - pa;
	float3 r2 = cb * clamp(dot(cb, pb) / dot(cb, cb), 0.0f, 1.0f) - pb;
	float3 r3 = dc * clamp(dot(ad, pc) / dot(dc, dc), 0.0f, 1.0f) - pc;
	float3 r4 = ad * clamp(dot(ad, pd) / dot(ad, ad), 0.0f, 1.0f) - pd;

	return sqrt(
		(
			sign(dot(cross(ba, nor), pa)) + 
			sign(dot(cross(cb, nor), pb)) +
			sign(dot(cross(dc, nor), pc)) +
			sign(dot(cross(ad, nor), pd)) < 3.0f
		) ?
		min(min(min(
			dot(r1, r1),
			dot(r2, r2)),
			dot(r3, r3)),
			dot(r4, r4)
		)
		:
		(dot(nor, pa) * dot(nor, pa) / dot(nor, nor))
	);
}

#endif