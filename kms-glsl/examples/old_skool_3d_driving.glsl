// Copied from https://www.shadertoy.com/view/XtlGW4
// Created by https://www.shadertoy.com/user/mattz

void mainImage( out vec4 f, vec2 p ){
    
    vec3 q=iResolution,d=vec3(p-.5*q.xy,q.y)/q.y,c=vec3(0,.5,.7);
    
    q=d/(.1-d.y);
    float a=iTime, k=sin(.2*a), w = q.x *= q.x-=.05*k*k*k*q.z*q.z;

    f.xyz=d.y>.04?c:
    	sin(4.*q.z+40.*a)>0.?
        w>2.?c.xyx:w>1.2?d.zzz:c.yyy:
	    w>2.?c.xzx:w>1.2?c.yxx*2.:(w>.004?c:d).zzz;
    
}
