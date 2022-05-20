// Copied from https://www.shadertoy.com/view/7slSW4
// Created by https://www.shadertoy.com/user/SnoopethDuckDuck

float func(vec2 uv, float t)
{
vec2 dir = uv - 0.5;
float theta = atan(dir.y, dir.x);
float d = length(dir + .28 * (1. +cos( theta + 3. * t)) * vec2(cos(t), sin(t))) * length(dir);
return 1. - smoothstep(d,0.015 + 0.005 * cos(0.5 * t), 0.015 + 0.005 *  (1. +cos(0.5 * t)));
}

const float pi = 3.1415;
const float pi2 = 2. * pi / 3.;
const float pi3 = 4. * pi /3.;
const float pi4 = 1. * pi /180.;
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.y - vec2(0.375,0);
float t= 0.95 * iTime;

vec2 m = 0.01 * vec2(cos(t),sin(t));
vec2 n = 0.01 * vec2(cos(t+ 3.14159),sin(t+3.14159));
vec3 col = vec3(func(uv,t + pi4 * cos(3. * t)), func(uv + m,t+pi4 * cos(3. * t+pi2)),func(uv+ n,t+pi4 * cos(3. * t+pi3)));

    // Output to screen
    fragColor = vec4(col,1.0);
}
