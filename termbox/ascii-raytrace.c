#include <stdio.h>
#include <stdbool.h>
#include "vec3.h"
#include "termbox.h"

#define CLIP_DIST  0.1f
#define CLIP_WIDTH 0.2f
#define GRAVITY 0.01f

#define CLAMP(X, lo, hi) (X<lo ? lo : X>hi ? hi : X)

vec3 UP            = { 0.0f,  1.0f,  0.0f};
vec3 DOWN          = { 0.0f, -1.0f,  0.0f};
vec3 VEC3_ZERO     = { 0.0f,  0.0f,  0.0f};

vec3 LIGHTPOS      = { 3.6f,  1.5f,  0.0f};
float LIGHTRADIUS  = 0.3f;

vec3 SPHERECENTRE  = { 0.0f,  2.0f,  0.0f};
vec3 SPHEREVEL     = { 0.0f,  0.0f,  0.0f};
float SPHERERADIUS = 0.9f;

// char SHADES[10] = " .-~=+<#%@"; // alternate character only palette
//char SHADES[10] = {32, 32, 176, 176, 177, 177, 178, 178, 219, 219};
char SHADES[10] = {176, 177, 178, 179, 180, 181, 182, 183, 184, 185};

// window resolution in characters
int COLUMNS, ROWS;

bool intersectLight(ray ray)
{
	// true if ray passes within LIGHTRADIUS of LIGHTPOS
	return length( cross(ray.d, sub(LIGHTPOS, ray.o)) ) < LIGHTRADIUS;
}

float bloom(ray ray)
{
	// gives a glow around the light

	// closest distance from ray to light centre
	float dist = length( cross(ray.d, sub(LIGHTPOS, ray.o)) );

	// calculate amount of bloom light from distance
	return CLAMP(8.0f-dist*6.0f, 0.0f, 4.0f);
}

bool intersectSphere(ray viewRay, ray* outNormal)
{
	// intersection formula of ray with sphere gives a quadratic equation
	// b and c are x and unit coefficients
	float b = 2.0f*dot(viewRay.d, sub(viewRay.o, SPHERECENTRE));
	float c = length2(sub(viewRay.o, SPHERECENTRE)) - SPHERERADIUS*SPHERERADIUS;

	// if determinant of equation is less than zero there is no solutions so ray doesn't intersect
	float determinant = b*b - 4.0f*c;
	if (determinant < 0.0f) {
		return false;
	}

	// get distance along ray to sphere from sphere ray intersection fomula - negative sqrt for closer intersection
	float distanceAlongRayToSphere = 0.5f*( -b - sqrtf(determinant) );

	// if we have to go backwards along the ray then the intersection is behind the camera so don't render
	bool intersectionIsBehindCamera = distanceAlongRayToSphere < 0.0f;
	if (intersectionIsBehindCamera) {
		return false;
	}

	// set the normal ray at the hit and return
	outNormal->o = add(viewRay.o, scalef(viewRay.d, distanceAlongRayToSphere));
	outNormal->d = norm(sub(outNormal->o, SPHERECENTRE));

	return true;
}

float shadowOfSphere(ray ray)
{
	// dist along ray to closest distance to sphere
	float distToClosestDist = dot(ray.d, sub(SPHERECENTRE, ray.o));

	// if sphere is further from the ray origin than the light then there's no shadow
	if ( distToClosestDist > distance(ray.o, LIGHTPOS) ) {
		return 0;
	}

	// closest distance from ray to sphere centre
	float dist = length( cross(ray.d, sub(SPHERECENTRE, ray.o)) );

	// calculate shadow from distance
	return CLAMP(9.0f - dist*7, 0.0f, 9.0f);
}

bool intersectPlane(ray viewRay, ray* outNormal, ray* outShadow)
{
	// interection with ground plane - at y=0 with normal facing up

	// if ray is perpendicular to plane or close enought then it won't intersect
	bool perpendicularToPlane = fabs(viewRay.d.y) < 0.001f;
	if (perpendicularToPlane) {
		return false;
	}

	// get distance to plane using ray-plane intersection formula
	float distanceAlongRayToPlane = -viewRay.o.y / viewRay.d.y;

	// if we have to go backwards along the ray then the intersection is behind the camera so don't render
	bool intersectionIsBehindCamera = distanceAlongRayToPlane < 0.0f;
	if (intersectionIsBehindCamera) {
		return false;
	}

	// set normal and shadow rays before returning
	outNormal->o = add(viewRay.o, scalef(viewRay.d, distanceAlongRayToPlane));
	outNormal->d = UP;

	outShadow->o = outNormal->o;
	outShadow->d = norm( sub(LIGHTPOS, outShadow->o) );

	return true;
}

void drawScreen(vec3 eye, vec3 view)
{
	// screen buffer
//	char screen[COLUMNS*ROWS];
	struct tb_cell* c = tb_cell_buffer();

	// vectors that face left and up on the screen in world space
	vec3 screenLeft = cross(view, UP);
	vec3 screenUp   = cross(view, screenLeft);

	// current index in screen buffer
	int i=0;

	// loop over all pixels in screen buffer
	for (int cy=-ROWS/2; cy<ROWS/2; ++cy) {
		for (int cx=-COLUMNS/2; cx<COLUMNS/2; ++cx, ++i) {

			// view direction from eye pos through pixel
			vec3 viewDir = add3(
			                       scalef(view,       CLIP_DIST),
			                       scalef(screenLeft, CLIP_WIDTH*cx/(float)COLUMNS),
			                       scalef(screenUp,   CLIP_WIDTH*cy/(float)(ROWS*2))
			               );

			// viewray, normal of hit and shadow ray at hit
			ray viewRay = {eye, norm(viewDir)};
			ray normal  = {};
			ray shadow  = {};

			// light is used for the index in the SHADES
			unsigned char light = 0;

			// raytracing routine is a bit arbitrary and lacking extensibility
			if ( intersectLight(viewRay) && (!intersectSphere(viewRay, &normal) || eye.x > 0.0f) ) {

				light = 9;
			} else if ( intersectSphere(viewRay, &normal) ) {

				light = (unsigned char)(5*(dot(normal.d, norm(sub(LIGHTPOS, normal.o))) + 1));
			} else if ( intersectPlane(viewRay, &normal, &shadow) ) {

				float direct       = CLAMP(9.0f - 0.25f*distance(normal.o, flat(LIGHTPOS)), 2.0f, 9.0f);
				float sphereShadow = shadowOfSphere(shadow);

				light = CLAMP(direct-sphereShadow, 0.0f, 9.0f);
			}

			if ( !intersectSphere(viewRay, &normal) && light<=6 ) {

				unsigned char bloomLight = (unsigned char) bloom(viewRay);

				light = CLAMP(light + bloomLight, 0, 7);
			}

//			screen[i] = SHADES[light];
			c[i].ch = ' ';
			c[i].fg = SHADES[light];
		}
	}

	// set null at end of string and printf it
//	screen[i] = 0x00;
//	printf("%s", screen);
}

void animate(vec3* eye, vec3* view)
{
	// verlet integration for sphere
	SPHERECENTRE = add( add(SPHERECENTRE, SPHEREVEL), scalef(DOWN, GRAVITY) );
	SPHEREVEL    = add( SPHEREVEL, scalef(DOWN, GRAVITY) );

	// bounce the sphere off the plane
	if (SPHERECENTRE.y < SPHERERADIUS) {
		SPHEREVEL.y *= -1;
		SPHERECENTRE.y = SPHERERADIUS*1.01f;
	}

	// rotate camera around the origin
	*eye  = rotate_y(*eye,  0.03);
	*view = rotate_y(*view, 0.03);
}

int main()
{
	tb_init();
//	tb_select_output_mode(TB_OUTPUT_NORMAL);
	tb_select_output_mode(TB_OUTPUT_256);
	tb_clear();

	COLUMNS = tb_width();
	ROWS = tb_height();

//	COLUMNS = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
//	ROWS    = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;

	// set eye position and view direction
	vec3 eye  = { -4.0,  1.5f, 0.0f};
	vec3 view = {  1.0, -0.1f, 0.0f};
	view = norm(view);

	// sim time
	float t = 0.0f;

	while (t < 30.0f) {
		struct tb_event ev;
		int e = tb_peek_event(&ev, 10);

		if (e == -1) break;
		if (e == TB_EVENT_KEY) break;

		t += 0.033f;

		// sleep for 0.033s (windows only)
//		sleep(33);

		drawScreen(eye, view);
		animate(&eye, &view);

		tb_present();
	}

	tb_shutdown();
	return 0;
}
