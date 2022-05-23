#define YPOS (.09/(U.y+sin(C.z*5.+iTime)/10.))

void mainImage(out vec4 C, vec2 U){

    U = (U+U-iResolution.xy)/iResolution.xy;
    C = vec4(1.);
    
    for (C.xyz += fract(iTime); C.z > 0. && 1.-C.z < -YPOS; C.xyz -= .01) ;
    
    if(YPOS > 0.){
        C.xyz = vec3(0., 0., .05);
        return;
    }
    float xpos = abs(U.x+sin(C.z+iTime/2.)/2.);
    if(1.-C.z < .01/xpos){
        C.xyz = vec3(C.z);
        return;
    }
    if(1.-C.z > .2/xpos)
        C.xz /= 2.;
    else if(1.-C.z > .15/xpos)
        C.yz /= 2.;
        
    if(mod(C.z-iTime, .5) < .25)
        C.xyz /= 2.;
    else
        C.xyz /= 3.;
    
}

/* Made on 5/4/2022

Using a mini raymarcher with a low step count to march a pixelated road floor.
Another use for the simple raymarching method as an earlier shader,
Raymarcher Golf: https://www.shadertoy.com/view/stXBR2
The less steps it does the more pixelated it looks.
*/
