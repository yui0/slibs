// Copied from https://www.shadertoy.com/view/fdXXWH
// Created by https://www.shadertoy.com/user/Xor

//Anti-Aliasing (SSAA). Use 1.0 on slower computers
#define AA 2.

//Background gradient
vec3 background(vec3 d)
{
    float light = dot(d,sqrt(vec3(.3,.5,.2)));
    
    return vec3(max(light*.5+.5,.0));
}
//Smooth minimum (based off IQ's work)
float smin(float d1, float d2)
{
    const float e = -6.;
    return log(exp(d1*e)+exp(d2*e))/e;
}
//Ripple and drop distance function
float dist(vec3 p)
{
    float l = pow(dot(p.xz,p.xz),.8);
    float ripple = p.y+.8+.4*sin(l*3.-iTime+.5)/(1.+l);
    
    float h1 = -sin(iTime);
    float h2 = cos(iTime+.1);
    float drop = length(p+vec3(0,1.2,0)*h1)-.4;
    drop = smin(drop,length(p+vec3(.1,.8,0)*h2)-.2);
    return smin(ripple,drop);
}
//Typical SDF normal function
vec3 normal(vec3 p)
{
    vec2 e = vec2(1,-1)*.01;
    
    return normalize(dist(p-e.yxx)*e.yxx+dist(p-e.xyx)*e.xyx+
    dist(p-e.xxy)*e.xxy+dist(p-e.y)*e.y);
}
//Basic raymarcher
vec4 march(vec3 p, vec3 d)
{
    vec4 m = vec4(p,0);
    for(int i = 0; i<99; i++)
    {
        float s = dist(m.xyz);
        m += vec4(d,1)*s;
        
        if (s<.01 || m.w>20.) break;
    }
    return m;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 res = iResolution.xy;
    vec3 col = vec3(0);
    
    vec3 pos = vec3(.05*cos(iTime),.1*sin(iTime),-4);
    
    //Sample
    for(float x = 0.;x<AA;x++)
    for(float y = 0.;y<AA;y++)
    {
        vec3 ray = normalize(vec3(fragCoord-res/2.+vec2(x,y)/AA,res.y));
        vec4 mar = march(pos,ray);
        vec3 nor = normal(mar.xyz);
        vec3 ref = refract(ray,nor,.75);
        float r = smoothstep(.8,1.,dot(reflect(ray,nor),sqrt(vec3(.3,.5,.2))));
        vec3 wat = background(ref)+.1*r*sqrt(abs(dot(ray,nor)));
        vec3 bac = background(ray)*.5+.5;

        float fade = pow(min(mar.w/20.,1.),.3);
        col += mix(wat,bac,fade);
    }
    col /= AA*AA;

    fragColor = vec4(col*col,1);
}
