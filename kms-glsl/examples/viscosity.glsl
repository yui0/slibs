// Copied from https://www.shadertoy.com/view/tdyBRt
// Created by https://www.shadertoy.com/user/klk

// Created by Alex Kluchikov

#define PI 3.141592654

vec2 rot(vec2 p,float a)
{
    float c=cos(a*15.83);
    float s=sin(a*15.83);
    return p*mat2(s,c,c,-s);
}

void mainImage(out vec4 o, in vec2 uv)
{
    uv/=iResolution.xx;
    uv=vec2(.125,.75)+(uv-vec2(.125,.75))*.03;
    float T=iTime*.25;

    vec3 c = clamp(1.-.7*vec3(
        length(uv-vec2(.1,0)),
        length(uv-vec2(.9,0)),
        length(uv-vec2(.5,1))
        ),0.,1.)*2.-1.;

    vec3 c0=vec3(0);
    float w0=0.;
    const float N=16.;
    for(float i=0.;i<N;i++)
    {
        float wt=(i*i/N/N-.2)*.3;
        float wp=0.5+(i+1.)*(i+1.5)*0.001;
        float wb=.05+i/N*0.1;
    	c.zx=rot(c.zx,1.6+T*0.65*wt+(uv.x+.7)*23.*wp);
    	c.xy=rot(c.xy,c.z*c.x*wb+1.7+T*wt+(uv.y+1.1)*15.*wp);
    	c.yz=rot(c.yz,c.x*c.y*wb+2.4-T*0.79*wt+(uv.x+uv.y*(fract(i/2.)-0.25)*4.)*17.*wp);
    	c.zx=rot(c.zx,c.y*c.z*wb+1.6-T*0.65*wt+(uv.x+.7)*23.*wp);
    	c.xy=rot(c.xy,c.z*c.x*wb+1.7-T*wt+(uv.y+1.1)*15.*wp);
        float w=(1.5-i/N);
        c0+=c*w;
        w0+=w;
    }
    c0=c0/w0*2.+.5;//*(1.-pow(uv.y-.5,2.)*2.)*2.+.5;
    c0*=.5+dot(c0,vec3(1,1,1))/sqrt(3.)*.5;
    c0+=pow(length(sin(c0*PI*4.))/sqrt(3.)*1.0,20.)*(.3+.7*c0);
	o=vec4(c0,1.0);
}

