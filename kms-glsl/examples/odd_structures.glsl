// Copied from https://www.shadertoy.com/view/NsfSWj
// Created by https://www.shadertoy.com/user/sukupaper

// Author: paperu
// Title: odd structures

const float P = 6.28318;

float t;
int matId;
vec3 map, pOffset;

mat2 rot(in float a) { return mat2(cos(a), sin(a), -sin(a), cos(a)); }

float box(in vec3 p, in float s, in float r) { return length(max(abs(p) - s, 0.)) - r; }
float ibox(in vec3 p, in float s) { p = abs(p) - s; return min(max(p.y, p.z), min(max(p.x, p.y), max(p.x, p.z))); }

float df(in vec3 p) {
    p += pOffset;
    
    vec3 pp = map = mod(p, 4.) - 2.;
    p = mod(p - .5, 1.) - .5;
    
    float b1 = max(box(p, .45, .02), -ibox(p, .4));
    p = abs(p) - .5;
    float b2 = max(box(p, .15, .01), -ibox(p, .125));
    float b3 = max(box(pp, 1.3, .01), -ibox(pp, 1.25));
    
    float d = min(min(b1, b2), b3);
    matId = d == b1 ? 1 : d == b2 ? 2 : 3;
    return d;
}

struct rmRes { vec3 pos; int it; bool hit; };
rmRes rm(in vec3 c, in vec3 r) {
    rmRes res; res.pos = c, res.hit = true, res.it = 0;
    for(int i = 0; i < 25; i++) {
        float d, dd;
        if((d = df(res.pos)) < .0001) return res;
        res.pos += d*(r += r*d*.075);
        mat2 rt = rot(d*.25); r.xy *= rt, r.xz *= rt, r.yz *= rt;
        res.it++;
    }
    res.hit = false;
    return res;
}

vec3 plane2sphere(in vec2 p) {
    float t = -4./(dot(p,p) + 4.);
    return vec3(-p*t, 1. + 2.*t);
}

float anim1(in float x, in float off, in float steps) {
    x += off;
    return (1. - min(mod(floor(x + 1.), steps), 1.))*smoothstep(0., 1., fract(x)) + floor(x/steps);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 st = (fragCoord.xy - .5*iResolution.xy)/iResolution.x;
    
    t = iTime*.5 + .75;
    float a[6];
    for(int i = 0; i < 6; i++)
        a[i] = anim1(t, float(i), 6.);
    pOffset = vec3(a[1] + a[5], a[0] - a[3], a[2] - a[4]);
    
    vec3 c = vec3(0., 0., -5.), r = plane2sphere(st*P*1.5);
    mat2 rt = rot(t*.75); r.xy *= rt, r.xz *= rt, r.yz *= rt;
    
    rmRes res = rm(c,r);
    
    vec3 color = vec3(0.);
    if(res.hit) {
        color = matId == 1 ? vec3(.174, .239, .765)
            : matId == 2 ? vec3(1., .23, .249)
            : vec3(1., .681, .049)*step(.5,fract((map.x + map.y + map.z)*10.));
    }
    float dist = distance(res.pos, c);
    color += dist*.025;
    color -= pow(max(0., float(res.it)*.0225),2.);
	color = mix(color, color.ggg, min(1., dist*dist*dist*.08));
    
    fragColor = vec4(color, 1.);
}
