// Created by inigo quilez - iq/2013
//   https://www.youtube.com/c/InigoQuilez
//   https://iquilezles.org
// I share this piece (art and code) here in Shadertoy and through its Public API, only for educational purposes. 
// You cannot use, sell, share or host this piece or modifications of it as part of your own commercial or non-commercial product, website or project.
// You can share a link to it or an unmodified screenshot of it provided you attribute "by Inigo Quilez, @iquilezles and iquilezles.org". 
// If you are a teacher, lecturer, educator or similar and these conditions are too restrictive for your needs, please contact me and we'll work it out.


//========================================================================================
// noises
//========================================================================================

float sin_noise( in vec2 x ) { return sin(x.x+sin(x.y)); }
float fbm( vec2 p )
{
    const mat2 m2 = mat2( 0.80, -0.60, 0.60, 0.80 );
    float f = 0.0;
    f += 0.5000*sin_noise( p ); p = m2*p*2.02;
    f += 0.2500*sin_noise( p ); p = m2*p*2.03;
    f += 0.1250*sin_noise( p ); p = m2*p*2.01;
    f += 0.0625*sin_noise( p );
    return f/0.9375;
}

//========================================================================================
// distance primitives
//========================================================================================

float sdBox( vec3 p, vec3 b )
{
    vec3 d = abs(p) - b;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

vec2 sdSegment( vec3 a, vec3 b, vec3 p )
{
	vec3 pa = p - a;
	vec3 ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return vec2( length( pa - ba*h ), h );
}

float sdCone( in vec3 p, in vec3 c )
{
    vec2 q = vec2( length(p.xz), p.y );
	return max( max( dot(q,c.xy), p.y), -p.y-c.z );
}

float sdSphere( vec3 p, float s )
{
    return length(p)-s;
}

float sdCylinder( vec3 p, vec2 h )
{
    vec2 d = abs(vec2(length(p.xz),p.y)) - h;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float udRoundBox( vec3 p, vec3 b, float r )
{
    return length(max(abs(p)-b,0.0))-r;
}

float det( vec2 a, vec2 b ) { return a.x*b.y-b.x*a.y; }
vec3 getClosest( vec2 b0, vec2 b1, vec2 b2 ) 
{
    float a =     det(b0,b2);
    float b = 2.0*det(b1,b0);
    float d = 2.0*det(b2,b1);
    float f = b*d - a*a;
    vec2  d21 = b2-b1;
    vec2  d10 = b1-b0;
    vec2  d20 = b2-b0;
    vec2  gf = 2.0*(b*d21+d*d10+a*d20); gf = vec2(gf.y,-gf.x);
    vec2  pp = -f*gf/dot(gf,gf);
    vec2  d0p = b0-pp;
    float ap = det(d0p,d20);
    float bp = 2.0*det(d10,d0p);
    float t = clamp( (ap+bp)/(2.0*a+b+d), 0.0 ,1.0 );
    return vec3( mix(mix(b0,b1,t), mix(b1,b2,t),t), t );
}

vec2 sdBezier( vec3 a, vec3 b, vec3 c, vec3 p, in float thickness )
{
	vec3 w = normalize( cross( c-b, a-b ) );
	vec3 u = normalize( c-b );
	vec3 v = normalize( cross( w, u ) );

	vec2 a2 = vec2( dot(a-b,u), dot(a-b,v) );
	vec2 b2 = vec2( 0.0 );
	vec2 c2 = vec2( dot(c-b,u), dot(c-b,v) );
	vec3 p3 = vec3( dot(p-b,u), dot(p-b,v), dot(p-b,w) );

	vec3 cp = getClosest( a2-p3.xy, b2-p3.xy, c2-p3.xy );

	return vec2( 0.85*(sqrt(dot(cp.xy,cp.xy)+p3.z*p3.z) - thickness), cp.z );
}

//========================================================================================
// distance operators
//========================================================================================

float opU( float d2, float d1 ) { return min( d1,d2); }
float opS( float d2, float d1 ) { return max(-d1,d2); }
vec3  opU( vec3  d2, vec3  d1 ) { return ( d2.x<d1.x)? d2:d1; }
vec3  opS( vec3  d2, vec3  d1 ) { return (-d1.x>d2.x)?-d1:d2; }

//========================================================================================
// Bender
//========================================================================================

vec4 map( vec3 p )
{
	vec3 q = vec3( abs(p.x), p.yz );

    // head and body
	float d1;
    d1 =          sdSegment(     vec3(0.0,1.8,0.0), vec3(0.0,0.7,0.0), p ).x - 0.5;
    d1 = opU( d1, sdCone(      p-vec3(0.0,2.8,0.0), vec3( normalize(vec2(0.9,0.1)), 0.5 ) ) );
    d1 = opS( d1, sdCylinder( (p-vec3(0.0,1.0,0.4)).zxy, vec2(0.2,1.0) ) );
    d1 = opU( d1, sdCylinder( (p-vec3(0.0,-0.32,0.0)), vec2(0.93,0.7) ) );
    d1 = opU( d1, sdCone(     (p-vec3(2.2,1.7,1.0))*vec3(1.0,-1.0,-1.0), vec3( normalize(vec2(0.8,0.4)), 0.5 ) ) );
    vec4 res = vec4( d1,1.0,0.0,0.0 );
    
    // neck and fingers    
    {
    vec3 d;
    d =         vec3( udRoundBox( p-vec3(0.0,1.55,0.30), vec3(0.35,0.1,0.3)-0.03, 0.12 ), 1.0, 5.0 );
    d = opS( d, vec3( udRoundBox( p-vec3(0.0,1.55,0.35), vec3(0.35,0.1,0.5)-0.04, 0.10 ), 0.0, 5.0) );
    d = opU( d, vec3( sdCone(     p-vec3(0.0,1.08,0.00), vec3(0.60,0.8,0.7) ), 1.0, 6.0) );
    d = opU( d, vec3( sdSphere(   p-vec3(0.0,2.70,0.00), 0.08 ), 1.0, 7.0) );
    d = opU( d, vec3( sdSphere(   p-vec3(0.0,2.20,0.00), 0.15 ), 1.0, 8.0) );
    d = opU( d, vec3( sdSegment( vec3(2.2,2.10,1.0), vec3(2.2,2.35,0.8), q ).x - 0.08, 0.8,  9.0) );
	d = opU( d, vec3( sdSegment( vec3(2.2,2.10,1.0), vec3(2.4,2.35,1.2), q ).x - 0.08, 0.8, 10.0) );
	d = opU( d, vec3( sdSegment( vec3(2.2,2.10,1.0), vec3(2.0,2.35,1.2), q ).x - 0.08, 0.8, 11.0) );
    d = opU( d, vec3( sdSegment( vec3(2.2,2.35,0.8), vec3(2.2,2.40,0.8), q ).x - 0.08, 0.8,  9.0) );
	d = opU( d, vec3( sdSegment( vec3(2.4,2.35,1.2), vec3(2.4,2.50,1.1), q ).x - 0.08, 0.8, 10.0) );
	d = opU( d, vec3( sdSegment( vec3(2.0,2.35,1.2), vec3(2.0,2.50,1.1), q ).x - 0.08, 0.8, 11.0) );
    if( d.x<res.x ) res = vec4( d.x,3.0,d.yz );
    }
    
    // eyes and mouth
    {
    vec3 d;
    d =         vec3( sdSphere(   p-vec3( 0.18,1.55,0.48), 0.18           ), 1.0, 12.0 );
    d = opU( d, vec3( sdSphere(   p-vec3(-0.18,1.55,0.48), 0.18           ), 1.0, 13.0 ) );
    d = opU( d, vec3( sdCylinder( p-vec3( 0.00,1.00,0.00), vec2(0.47,0.4) ), 1.0, 14.0 ) );
    if( d.x<res.x ) res = vec4( d.x,2.0, d.yz );
    }
    
    // arms
    {
    vec3 d;
    d =         vec3( sdSphere( q-vec3( 0.88,0.15,0.0), 0.19 ), 0.0, 15.0 );
    d = opU( d, vec3( sdBezier( vec3(0.9,0.15,0.0), vec3(2.0,-0.5,0.0), vec3(2.2,2.0,1.0), q, 0.13 ), 16.0) );
    if( d.x<res.x ) res = vec4( d.x,0.0, d.yz );
    }

    return res;
}

//========================================================================================

vec4 raycast( in vec3 ro, in vec3 rd )
{
	const float maxd = 9.0;
    float t = 0.0;
    vec4 res = vec4(-1.0);
    for( int i=0; i<128; i++ )
    {
	    vec4 tmp = map( ro+rd*t );
        res = vec4(t,tmp.yzw);
        if( tmp.x<0.001 || t>maxd ) break;
        t += tmp.x;
    }
    return (t>maxd) ? vec4(-1.0) : res;
}

// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal( in vec3 pos )
{
    vec2 e = vec2(-1.0,1.0) * 0.005;   
	return normalize( e.yxx*map( pos + e.yxx ).x + 
					  e.xxy*map( pos + e.xxy ).x + 
					  e.xyx*map( pos + e.xyx ).x + 
					  e.yyy*map( pos + e.yyy ).x );    
}

// https://iquilezles.org/articles/rmshadows
float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<48; i++ )
    {
        float h = map(ro + rd*t).x;
        res = min( res, k*h/t );
        if( res<0.001 || t>5.0 ) break;
		t += clamp( h, 0.005, 0.5 );
    }
    return clamp(res,0.0,1.0);
}

void generateRay( out vec3 resRo, out vec3 resRd, in vec3 po, in vec3 ta, in vec2 pi )
{
	vec2 p = (2.0*pi-iResolution.xy)/iResolution.y;
        
    // camera matrix
    vec3 ww = normalize( ta - po );
    vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
    vec3 vv = normalize( cross(uu,ww));

	// create view ray
	vec3 rd = normalize( p.x*uu + p.y*vv + 2.2*ww );

    resRo = po;
    resRd = rd;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 p = (2.0*fragCoord-iResolution.xy)/iResolution.y;
    
    //-----------------------------------------------------
    // camera
    //-----------------------------------------------------
    float an = 5.7 + 0.62831*iTime;
	vec3  po = vec3(1.0 + 3.5*sin(an),0.6,3.5*cos(an));
    vec3  ta = vec3(1.0,1.5,0.0);

    vec3 ro,  rd;  generateRay( ro,  rd,  po, ta, fragCoord.xy );
    vec3 ro2, rd2; generateRay( ro2, rd2, po, ta, fragCoord.xy+vec2(2.0,0.0) );
    vec3 ro3, rd3; generateRay( ro3, rd3, po, ta, fragCoord.xy+vec2(0.0,2.0) );
    
    //-----------------------------------------------------
	// sky
    //-----------------------------------------------------
    
    float gra = smoothstep(-0.3,0.6,rd.y);                    
    vec3  sky = vec3(0.4,0.7,1.0)*0.2;
    float h = (1000.0-ro.y)/rd.y;
    if( h>0.0 )
    {
        vec2 uv = ro.xz + rd.xz*h;
        float cl = smoothstep( 0.0, 0.8, fbm( 0.002*uv ) );
        cl = clamp( floor(cl*3.0)/(3.0-1.0), 0.0, 1.0 );
        sky = mix( sky, vec3(0.9,0.6,0.3), 0.4*cl*gra );
    }
    vec3 col = mix( 0.6*vec3(0.6,0.9,1.0), sky, gra );

    //-----------------------------------------------------
	// bender
    //-----------------------------------------------------

    // raymarch
    vec4 tmat  = raycast( ro ,rd );
    if( tmat.y>-0.5 )
    {
        // geometry
        vec3 pos = ro + tmat.x*rd;
        vec3 nor = calcNormal(pos);
		vec3 ref = reflect( rd, nor );

        // materials
		vec3 mate = vec3(0.0);
		if( tmat.y<0.5 )
		{
			mate = vec3(0.1,0.12,0.15);
            mate *= smoothstep( -0.95, -0.94, sin(40.0*tmat.z) );
		}
		else if( tmat.y<1.5 )
		{
			mate = vec3(0.13,0.17,0.24);
            mate *= clamp( 200.0*udRoundBox( pos-vec3(0.0,1.55,0.35), vec3(0.35,0.1,0.5)-0.04, 0.1 ), 0.0, 1.0 );
		}
		else if( tmat.y<2.5 )
		{
			mate = vec3(0.7,0.68,0.4);
            vec3 qos = vec3( abs(pos.x), pos.yz );
            mate *= clamp( 100.0 * sdBox(qos-vec3(0.18+0.05*sign(pos.x),1.55+0.05,0.5),vec3(0.04,0.04,0.5) ), 0.0, 1.0 );
            float teeth = min( sdBox(qos-vec3(0.0,1.08,0.5),vec3(1.0,0.01,0.3) ),
                               sdBox(qos-vec3(0.0,0.92,0.5),vec3(1.0,0.01,0.3) ) );
            teeth = min( teeth, sdBox( vec3(mod(qos.x+0.09,0.18),qos.yz) -vec3(0.0,1.0,0.5),vec3(0.01,0.3,0.3) ) );
            mate *= clamp( 100.0 * teeth, 0.0, 1.0 );
		}
		else if( tmat.y<3.5 )
		{
            mate = vec3(0.25,0.29,0.34) * tmat.z;
		}
        
		// lighting 
        const vec3 lig = normalize(vec3(0.8,0.6,0.3));
		float dif = max(dot(nor,lig),0.0);
		float sha = 0.0; if( dif>0.01 ) sha=softshadow( pos+0.01*nor, lig, 0.0005, 32.0 );
        float fre = pow( clamp( 1.0 + dot(nor,rd), 0.0, 1.0 ), 5.0 );
        vec3 brdf = vec3(0.0);
        brdf += 1.5*sha;
		brdf += 0.75;
        brdf += 0.7*smoothstep(0.1,0.15,fre*sha);
        
		// surface-light interacion
		col = mate*brdf;
	}

    // cartoon edges
    vec4 tmat2 = raycast(ro2,rd2); // raytrace right pixel
    vec4 tmat3 = raycast(ro3,rd3); // raytrace top pixel too
    col *= clamp( 1.0 - 10.0*max(abs(tmat.w-tmat2.w),abs(tmat.w-tmat3.w)), 0.0, 1.0 );

    // gamma
	col = pow( clamp(col,0.0,1.0), vec3(0.4545) );
    
	// vignetting
	vec2 q = fragCoord / iResolution.xy;
    col *= 0.4 + 0.6*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );
 
    fragColor = vec4( col, 1.0 );
}
