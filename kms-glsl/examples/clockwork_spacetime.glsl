// Copied from https://www.shadertoy.com/view/7d23R3
// Created by https://www.shadertoy.com/user/Tater

#define pi 3.14159265359
#define rot(a) mat2( cos(a),-sin(a),sin(a),cos(a) )
#define a(c) float( clamp(c,0.0,1.0) )
float gear(vec2 uv, float aa,float tth){
    float inn = 0.275; //Inner Circle Size
    float th = atan(-uv.y,-uv.x)/(2.0*pi)+0.5;
    float id = mod(floor(th*tth),2.0);
    th = mod(th,1./tth)*tth;
    float r = length(uv);
    float c = smoothstep(r-aa,r+aa,inn);
    th = abs(th-0.5)*1.05;
    float end = (-pow(th,3.4)+1.0)*0.425; //Tooth Shape
    c += smoothstep(r-aa,r+aa,end)*id*
    smoothstep(th-aa*tth/2.5,th+aa*tth/2.5,0.5);
    return c;
}
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 R = iResolution.xy;
    vec2 uv = (fragCoord-.5*R.xy)/R.y;
    float aa = 1./R.y;
    
    float t = iTime*1.5;
    //Time Warping
    t = pow(fract(t),3.5)+floor(t);
    
    //Size of rotation steps
    t*=pi/(6.0);
    
    //Whole Screen Rotation + direction flip
    uv*=rot(-t*sign(sin(t)));
    vec2 uv2 = uv;
    
    //Gear Tooth Count /2 (works well with 20,16,12,8,4)
    float tth =12.0;
    
    //Overall Scale
    float scl = 3.7;
    
    //Zoom In/Out
    scl-=cos(t)*0.75;
    
    //Slight Initial Offset
    t+=pi/tth;
    
    //Edge of screen warping
    scl*=1.0+pow(length(uv),10.0);
    
    //The Gears
    aa*=scl;
    vec3 gearCol = vec3(1.0); //I like it b/w but you can change it
    uv = 0.5-fract(uv*scl);
    uv *= rot(-t);
    vec3 col = gearCol*a(gear(uv,aa,tth))*(1.0-length(uv));
    
    t+=pi/2.;
    uv2 += 0.5/scl;
    uv = 0.5-fract(uv2*scl);
    uv *= rot(t);
    col += gearCol*a(gear(uv,aa,tth))*(0.05+length(uv));
    
    fragColor = vec4(col,1.0);
}
