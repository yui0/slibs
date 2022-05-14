#include <math.h>

typedef struct vec3 {
	float x;
	float y;
	float z;
} vec3;

float dot(vec3, vec3 );
vec3 cross(vec3, vec3);
vec3 add(vec3, vec3);
vec3 add3(vec3, vec3, vec3);
vec3 addf(vec3, float);
vec3 sub(vec3, vec3);
vec3 subf(vec3, float);
vec3 scale(vec3, vec3);
vec3 scalef(vec3, float);
float length(vec3);
float length2(vec3);
float distance(vec3, vec3);
vec3 flat(vec3);
vec3 norm(vec3);
vec3 rotate_y(vec3, float);

typedef struct ray {
	vec3 o;
	vec3 d;
} ray;


float dot(vec3 a, vec3 b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

vec3 cross(vec3 a, vec3 b)
{
	vec3 c = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
	return c;
}

vec3 add(vec3 a, vec3 b)
{
	vec3 c = {a.x+b.x, a.y+b.y, a.z+b.z};
	return c;
}

vec3 add3(vec3 a, vec3 b, vec3 c)
{
	vec3 d = {a.x+b.x+c.x, a.y+b.y+c.y, a.z+b.z+c.z};
	return d;
}

vec3 addf(vec3 a, float f)
{
	vec3 c = {a.x+f, a.y+f, a.z+f};
	return c;
}

vec3 sub(vec3 a, vec3 b)
{
	vec3 c = {a.x-b.x, a.y-b.y, a.z-b.z};
	return c;
}

vec3 subf(vec3 a, float f)
{
	vec3 c = {a.x-f, a.y-f, a.z-f};
	return c;
}

vec3 scale(vec3 a, vec3 b)
{
	vec3 c = {a.x*b.x, a.y*b.y, a.z*b.z};
	return c;
}

vec3 scalef(vec3 a, float f)
{
	vec3 c = {a.x*f, a.y*f, a.z*f};
	return c;
}

float length(vec3 a)
{
	return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}

float length2(vec3 a)
{
	return a.x*a.x + a.y*a.y + a.z*a.z;
}

float distance(vec3 a, vec3 b)
{
	return sqrtf((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z));
}

vec3 flat(vec3 a)
{
	vec3 b = {a.x, 0.0f, a.z};
	return b;
}

vec3 norm(vec3 a)
{

	float l = length(a);
	vec3 b = {a.x/l, a.y/l, a.z/l};
	return b;
}

vec3 rotate_y(vec3 a, float angle)
{

	vec3 b = {a.x*cos(angle) + a.z*sin(angle), a.y, a.x*-sin(angle) + a.z*cos(angle)};
	return b;
}
