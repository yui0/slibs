// Copied from https://www.shadertoy.com/view/4sSBW3
// Created by https://www.shadertoy.com/user/Klems

#define PI 3.14159265359
#define rot(a) mat2(cos(a + PI*0.25*vec4(0,6,2,0)))

vec3 hsv( in vec3 c ) {
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0,4,2),6.0)-3.0)-1.0, 0.0, 1.0);
	return c.z * mix(vec3(1), rgb, c.y);
}

float sdBox( in vec3 p, in vec3 b ) {
    vec3 d = abs(p) - b;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

vec2 hash22( in vec2 p ) {
    #define HASHSCALE3 vec3(.1031, .1030, .0973)
	vec3 p3 = fract(vec3(p.xyx) * HASHSCALE3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.xx + p3.yz) * p3.zy);
}

float perlin( in float x, in float seed ) {
    float a = floor(x);
    float b = a + 1.0;
    float f = x - a;
    f = f*f*(3.0-2.0*f);
    return mix(hash22(vec2(seed, a)).x, hash22(vec2(seed, b)).x, f);
}

// main distance function
float de( in vec3 p, in float r, out vec3 color ) {
    
    float d = -p.z;
    
    vec2 center = floor(p.xy)+0.5;
    
    color = vec3(0);
    float colorAcc = 0.0;
    
    for (int x = -1 ; x <= 1 ; x++) {
        for (int y = -1 ; y <= 1 ; y++) {
            
            vec2 offset = center + vec2(x, y);
            vec3 inCenter = p - vec3(offset, 0);
            
            // get random stuff for the cube
            vec2 rnd = hash22(offset);
            float height = 1.0 + perlin( iTime+rnd.x, rnd.y )*0.75;
            vec3 colorHere = hsv( vec3(-offset.y*0.1 + rnd.y*0.3, 1, 1) );
            // select the nearest cube
            float dist = sdBox(inCenter, vec3(vec2(0.45), height))-0.05;
            d = min(d, dist);
            // accumulate the color
            float alpha = max(0.001, smoothstep(r, -r, dist));
            color += colorHere*alpha;
            colorAcc += alpha;

        }
    }
    
    color /= colorAcc;
    
    return d;

} 

// normal function
vec3 normal( in vec3 p, float here ) {
	vec3 e = vec3(0.0, 0.001, 0.0);
    vec3 dummy = vec3(0);
	return normalize(vec3(
		here-de(p-e.yxx, 0.0, dummy),
		here-de(p-e.xyx, 0.0, dummy),
		here-de(p-e.xxy, 0.0, dummy)));	
}

// cone-tracing
vec4 trace( in vec3 from, in vec3 dir, in float sinTheta ) {
    
    float totdist = 0.01;
    vec4 acc = vec4(0, 0, 0, 1);
    
    for (int i = 0 ; i < 10 ; i++) {
        
        vec3 p = from + totdist * dir;
        
        // find color here, as well as distance
        float r = totdist*sinTheta;
        vec3 color = vec3(0);
        float dist = de(p, r, color);
        
        // find opacity here
        float alpha = clamp(dist / r * -0.5 + 0.5, 0.0, 1.0);
        acc.rgb += acc.a * alpha * color;
        acc.a *= 1.0 - alpha;
        
        // break early if the accumulated opacity is almost zero
        if (acc.a < 0.01) break;
        // otherwise continue forward
        totdist += abs(dist);
        
    }
    
    acc.a = 1.0 - acc.a;
    return acc;
    
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    
    vec2 uv = fragCoord.xy / iResolution.xy * 2.0 - 1.0;
	uv.y *= iResolution.y / iResolution.x;
	
	vec3 from = vec3(iTime, 0.2*iTime, -5)+vec3(iMouse.xy*0.04, 0);
	vec3 dir = normalize(vec3(uv, 0.5));
    
	float totdist = 0.0;
	
	for (int steps = 0 ; steps < 20 ; steps++) {
		vec3 p = from + totdist * dir;
        vec3 dummy = vec3(0);
		float dist = de(p, 0.0, dummy);
		totdist += dist;
		if (dist < 0.001) {
			break;
		}
	}
    
    vec3 p = from + totdist * dir;
    vec3 color = vec3(0);
	vec3 norm = normal(p, de(p, 0.01, color));
    
    // glossy reflection
    vec3 refl = reflect(dir, norm);
    vec4 gloss = trace(p, refl, 0.5);
    gloss.rgb = mix(vec3(1), gloss.rgb, gloss.a);
    
    // fresnel
    float fres = pow(max(0.0, dot(-dir, norm)), 2.0);
    
    fragColor.rgb = mix(color, gloss.rgb, fres);
    fragColor.a = 1.0;

}

