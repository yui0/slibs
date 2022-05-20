// Copied from https://www.shadertoy.com/view/MsX3Wj
// Created by https://www.shadertoy.com/user/fb39ca4

//Thank you iquilez for some of the primitive distance functions!


const float PI = 3.14159265358979323846264;


const int MAX_PRIMARY_RAY_STEPS = 64; //decrease this number if it runs slow on your computer

vec2 rotate2d(vec2 v, float a) { 
	return vec2(v.x * cos(a) - v.y * sin(a), v.y * cos(a) + v.x * sin(a)); 
}

float sdTorus( vec3 p, vec2 t ) {
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float distanceField(vec3 p) {
	return -sdTorus(p, vec2(4.0, 3.0));
}

vec3 castRay(vec3 pos, vec3 dir, float treshold) {
	for (int i = 0; i < MAX_PRIMARY_RAY_STEPS; i++) {
			float dist = distanceField(pos);
			//if (abs(dist) < treshold) break;
			pos += dist * dir;
	}
	return pos;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec4 mousePos = (iMouse / iResolution.xyxy) * 2.0 - 1.0;
	vec2 screenPos = (fragCoord.xy / iResolution.xy) * 2.0 - 1.0;
	vec3 cameraPos = vec3(0.0, 0.0, -3.8);
	
	vec3 cameraDir = vec3(0.0, 0.0, 0.5);
	vec3 planeU = vec3(1.0, 0.0, 0.0) * 0.8;
	vec3 planeV = vec3(0.0, iResolution.y / iResolution.x * 1.0, 0.0);
	vec3 rayDir = normalize(cameraDir + screenPos.x * planeU + screenPos.y * planeV);
	
	//cameraPos.yz = rotate2d(cameraPos.yz, mousePos.y);
	//rayDir.yz = rotate2d(rayDir.yz, mousePos.y);
	
	//cameraPos.xz = rotate2d(cameraPos.xz, mousePos.x);
	//rayDir.xz = rotate2d(rayDir.xz, mousePos.x);
	
	vec3 rayPos = castRay(cameraPos, rayDir, 0.01);
	
	float majorAngle = atan(rayPos.z, rayPos.x);
	float minorAngle = atan(rayPos.y, length(rayPos.xz) - 4.0);
		
	float edge = mod(8.0 * (minorAngle + majorAngle + iTime) / PI, 1.0);
	float color = edge < 0.7 ? smoothstep(edge, edge+0.03, 0.5) : 1.0-smoothstep(edge, edge+0.03, 0.96);
	//float color = step(mod(8.0 * (minorAngle + majorAngle + iTime) / PI, 1.0), 0.5);
	//color -= 0.20 * step(mod(1.0 * (minorAngle + 1.0 * majorAngle + PI / 2.0) / PI, 1.0), 0.2);
	
	fragColor = vec4(color);
}
