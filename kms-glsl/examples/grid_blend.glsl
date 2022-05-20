// Copied from https://www.shadertoy.com/view/fsf3W8
// Created by https://www.shadertoy.com/user/Del

// Simple grids (SST)
//
// Feel free to optimize, golf and generally improve them :)
//
// Del - 15/03/2021
//
//
// will it blend? (attempt2) er, not really

vec4 HexGrid(vec2 uv, out vec2 id)
{
    uv *= mat2(1.1547,0.0,-0.5773503,1.0);
    vec2 f = fract(uv);
    float triid = 1.0;
	if((f.x+f.y) > 1.0)
    {
        f = 1.0 - f;
     	triid = -1.0;
    }
    vec2 co = step(f.yx,f) * step(1.0-f.x-f.y,max(f.x,f.y));
    id = floor(uv) + (triid < 0.0 ? 1.0 - co : co);
    co = (f - co) * triid * mat2(0.866026,0.0,0.5,1.0);    
    uv = abs(co);
    return vec4(0.5-max(uv.y,abs(dot(vec2(0.866026,0.5),uv))),length(co),co);
}

// EquilateralTriangle distance
float sdEqTri(in vec2 p)
{
    const float k = 1.7320508;//sqrt(3.0);
    p.x = abs(p.x) - 0.5;
    p.y = p.y + 0.5/k;
    if( p.x+k*p.y>0.0 ) p = vec2(p.x-k*p.y,-k*p.x-p.y)/2.0;
    p.x -= clamp( p.x, -1.0, 0.0 );
    return -length(p)*sign(p.y);
}

// triangle grid equiv
vec4 TriGrid(vec2 uv, out vec2 id)
{
    const vec2 s = vec2(1, .8660254); // Sqrt (3)/2
    uv /= s;
    float ys = mod(floor(uv.y), 2.)*.5;
    vec4 ipY = vec4(ys, 0, ys + .5, 0);
    vec4 ip4 = floor(uv.xyxy + ipY) - ipY + .5; 
    vec4 p4 = fract(uv.xyxy - ipY) - .5;
    float itri = (abs(p4.x)*2. + p4.y<.5)? 1. : -1.;
    p4 = itri>0.? vec4(p4.xy*s, ip4.xy) : vec4(p4.zw*s, ip4.zw);  

    vec2 ep = p4.xy;
    ep.y = (ep.y + 0.14433766666667*itri) * itri;
    float edge = sdEqTri(ep); // dist to edge
    id = p4.zw;
    id *= mat2(1.1547,0.0,-0.5773503,1.0); // adjust ID (optional)
    p4.y+=0.14433766666667*itri;
    return vec4(abs(edge),length(p4.xy),p4.xy);
}

// simple square grid equiv
vec4 SquareGrid(vec2 uv, out vec2 id)
{
    vec2 fs =  fract(uv)-0.5;
    id = floor(uv);
    id *= mat2(1.1547,0.0,-0.5773503,1.0); // adjust ID (optional)
    vec2 d = abs(fs)-0.5;
    float edge = length(max(d,0.0)) + min(max(d.x,d.y),0.0);
    return vec4(abs(edge),length(fs),fs.xy);
}

float hbar(vec2 p, float nline, float t)
{
    return 0.5+sin((p.y*nline)+t)*0.5;
}


#define	PI 3.14159
#define	TAU 6.28318
// out: 0->val->0
float SmoothTri2(float t, float val)
{
    return val * (1.0-(0.5+cos(t*TAU)*0.5));
}
mat2 rot( float th ){ vec2 a = sin(vec2(1.5707963, 0) + th); return mat2(a, -a.y, a.x); }
float smin( float a, float b, float k )
{
	float h = clamp( 0.5 + 0.5*(b-a)/k, 0.0, 1.0 );
	return mix( b, a, h ) - k*h*(1.0-h);
}

vec2 smin( vec2 a, vec2 b, float k )
{
    a.x = smin(a.x,b.x,k);
    a.y = smin(a.y,b.y,k);
    return a;
}
vec4 smin( vec4 a, vec4 b, float k )
{
    a.x = smin(a.x,b.x,k);
    a.y = smin(a.y,b.y,k);
    a.z = smin(a.z,b.z,k);
    a.w = smin(a.w,b.w,k);
    return a;
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float t = iTime;
	vec2 uv = (fragCoord.xy - 0.5 * iResolution.xy) / iResolution.y;
    float xx = ((fragCoord.x-.5*iResolution.x) / iResolution.x)+0.5;

    // dirty grid blend!
    vec2 id,id2;
    vec4 h,h2;
	
    //h2 = SquareGrid(uv*8.0, id2);
    vec2 uv2 = uv;
    vec2 uv1 = uv;
    uv1 *= rot(t*0.1);
    uv2 *= rot(-t*0.1);
    
    h2 = HexGrid(uv1*2.0, id2);
    
    uv2 *= 1.5+sin(t*0.6)*0.5;
    h = TriGrid(uv2*4.0, id);
	
	float tt = SmoothTri2(fract(t*0.2),3.0)-1.0;
	
	tt = clamp(tt,0.0,1.0);
	
	//h = mix(h,h2,tt);
	//id = mix(id,id2,tt);
	
    h = smin(h,h2,0.25);
    id = smin(id,id2,0.25);


    vec3 bordercol = vec3(1.0,1.0,1.0);
    vec3 shapecol = vec3(0.2,0.25,0.35);
    
    // just do a simple patterned shape tint based on cell IDs
    float patternVal = 132.5; // 4.1
    float cm = 1.0 + pow(abs(sin(length(id)*patternVal + t*0.65)), 4.0);	// pulse mult
    cm *= 1.0 + (hbar(uv,250.0,t*12.0)*0.1);					// bars mult
    shapecol *= cm;
    
    // Output to screen
    vec3 finalcol = mix(bordercol,shapecol, smoothstep(0.0,0.015,h.x)); // edge
    //finalcol = mix(bordercol,finalcol, smoothstep(0.0,0.065,h.y));  // centre
    
    // vignetting
    uv = fragCoord/iResolution.xy;
	finalcol *= 0.5 + 0.5*pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.1 );

    //finalcol = vec3(h.zw,0.0); // just show cell uv
    
    // divider lines
    //float div = (1.0-max( step(abs(xx-0.33),0.0025),step(abs(xx-0.66),0.0025)));
    fragColor = vec4(finalcol ,1.0);
}
