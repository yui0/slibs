// Copied from https://www.shadertoy.com/view/wdVyRt
// Created by https://www.shadertoy.com/user/athibaul

// Remake of "Infinite Gamecube" by Jinkweiq
// https://www.shadertoy.com/view/tdKyz3


#define PI 3.1415926535
#define TAU (2.*PI)
#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))
#define SPEED 5.

float random(in float v){
    return fract(sin(dot(vec2(v+2.),vec2(12.9898,78.233)))*43758.5453123);
}



float sdBox(vec3 p,vec3 r)
{
    p=abs(p)-r;
    return length(max(p,0.))+min(max(p.x,max(p.y,p.z)),0.);
}

float map(vec3 p)
{
    float t = SPEED*iTime;
    vec3 q = p;
    if(random(floor(t)) > 0.5)
    {
        q.xy = q.yx;
    }
    // Rotate in the y direction
    q -= vec3(0,fract(t),0);
    q -= vec3(0,-0.5,0.);
    float th = PI/2.*fract(t);
    q.yz *= R(th);
    q -= vec3(0,0.5,0.5);
    float rounded = 0.07;
    float box = sdBox(q, vec3(0.5)-rounded)-rounded;
    return min(p.z, box);
}


vec3 surfColor(vec3 p)
{
    if(p.z > 0.01) return vec3(0.3,0.2,1.);
    
    float t = SPEED*iTime;
    
    vec3 tileCenter = random(floor(t)) > 0.5 ? vec3(fract(t), 0,0) : vec3(0, fract(t),0);
    float rounded = 0.1;
    vec3 col = vec3(0);
    for(float t_ = floor(t)-1.; t_ > floor(t) - 10.; t_--)
    {
        float d = sdBox(p-tileCenter, vec3(0.45)-rounded)-rounded;
        float intensity = clamp(t-t_-1.2,0.,1.);
        col += vec3(0.3,0.2,1.) * smoothstep(0.01,-0.01,d) * intensity;
        if(random(floor(t_)) > 0.5)
            tileCenter += vec3(1,0,0);
        else
            tileCenter += vec3(0,1,0);
    }
    return col;
}

vec3 normal(vec3 p)
{
    vec2 e = 0.002 * vec2(1,-1);
    return normalize(e.xxx*map(p+e.xxx)
                    +e.xyy*map(p+e.xyy)
                    +e.yxy*map(p+e.yxy)
                    +e.yyx*map(p+e.yyx));
}


vec3 render(vec3 ro, vec3 rd)
{
    float d, t=0.;
    for(int i=0; i<256; i++)
    {
        d = map(ro+t*rd);
        if(d < 0.001 || t > 100.) break;
        t += d;
    }
    vec3 p = ro+t*rd;
    vec3 n = normal(p);
    vec3 c = surfColor(p);
    
    //return 0.5+0.5*n;
    vec3 lig = normalize(vec3(0,-1,1));
    float NdotL = dot(n, lig);
    float RdotL = dot(reflect(rd, n), lig);
    vec3 spec = pow(clamp(RdotL,0.,1.),5.) * vec3(p.z) * 0.5;
    return c * (0.5+0.5*NdotL) + spec;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord*2.-iResolution.xy)/iResolution.y;

    // Perspective camera
    /*
    vec3 ro = vec3(2);
    vec3 camFwd = normalize(vec3(0,0,0.5) - ro);
    vec3 camRight = normalize(cross(camFwd, vec3(0,0,1)));
    vec3 camUp = cross(camRight, camFwd);
    vec3 rd = normalize(camFwd + 0.5*(uv.x*camRight+uv.y*camUp));
	*/
    
    // Orthographic camera
    vec3 rd = normalize(vec3(-1));
    vec3 camRight = normalize(cross(rd, vec3(0,0,1)));
    vec3 camUp = cross(camRight, rd);
    vec3 ro = vec3(0,0,0.5) + (vec3(1)+uv.x*camRight + uv.y*camUp) * 2.5;
    
    vec3 col = render(ro, rd);
    
    col = sqrt(col);
    fragColor = vec4(col,1.0);
}

