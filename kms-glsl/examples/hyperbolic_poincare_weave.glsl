// Copied from https://www.shadertoy.com/view/tljyRR
// Created by https://www.shadertoy.com/user/Shane

/*

	Hyperbolic Poincare Weave
	-------------------------


	This is a double sided Truchet weaved hyperbolic polygonal tiling of the 
    Poincare disc... which sounds more complex than it actually is, since it's 
    nothing more than an amalgamation of procedures that you'll find all over 
    Shadertoy.

    The imagery itself draws minor inspiration from M. C. Escher's Snake print, 
    and was rendered in a graphic ink style.

	I've always admired Matt's "Hyperbolic Truchet" example, and have wanted
	to put my own version together for a while, but I knew it'd involve some
	careful forethought, so put it aside until I was feeling energetic. However, 
    since that was never going to happen, I decided to fumble my through an
    example anyway. :D

	In regard to the Poincare disc side of things, it was helpful to reference
	MLA, SL2C and STB's examples -- All authors have really nice work on here,
	which is well worth the look. STB's "Hyperbolic Poincaré transformed"
	workings were more reminiscent of my own method (but nicer), so I adopted 
    his calculations for the initialization function.

	Double sided Truchet patterns were BigWings's idea some time back... Others
    may have done it, but none were advertising. :) Anyway,	they've been covered 
    before by both myself and others, if you wish to learn about that particular 
    aspect of this example.

    Poincare disc tiling and hyperbolic transformations have also been covered 
    everywhere, so there are plenty of examples on the net and on Shadertoy 
    regarding that aspect as well. Having said that, I'm hoping I've provided 
    enough of an explanation here to give people a start.

 	There's not as much code here as you'd think. Especially for the Poincare 
	disc setup. However, the example is a little long winded, so I'll put up a
	very basic example without the weave within the next day or two to give 
	people something easier to consume.

    The last thing I'll mention is that Poincare disc tiling is relatively
	rudimentary in the grand scheme of things. However, this is not my area of
	expertise, so don't look into anything I've written too deeply. In fact, if
	you happen to be one of the experts on Shadertoy, and you spot anything that 
    doesn't seem right, feel free to let me know.
	



    Poincare disc examples:

    // The hyperbolic transformation itself is based on STB's example here,
    // which I was informed was based on the following:
    //
    // Conformal Models of the Hyperbolic Geometry - Vladimir Bulatov 
    // http://www.bulatov.org/math/1001/index.html
    //
    // The Hyperbolic Chamber - Jos Leys
    // http://www.josleys.com/article_show.php?id=83
	//
    Hyperbolic Poincaré transformed - stb
	https://www.shadertoy.com/view/3lscDf

    // A beautiful example, and my reason for wanting to make one of these.
    //
	Hyperbolic Truchet tiles  - mattz 
    https://www.shadertoy.com/view/3llXR4

	// Like everyone else, I love the following example. However, be warned that 
    // it involves complex geometry. :)  Seriously though, Mattz can come up with 
    // solutions to problems in a few minutes that have utterly confounded me for 
    // weeks.
    //
	Hyperbolic Wythoff explorer - mattz 
	https://www.shadertoy.com/view/wtj3Ry

*/


// Regular pentagon tiling, which wouldn't work on the Euclidian plane, but
// works just fine within the Poincare disc.
//#define PENTAGON

// Moving to the half plane model. It's a very simple transformation, so I
// figured I may as well include it.
//#define HALF_PLANE


#define PI		3.14159265
#define TAU	    6.28318531

// Standard 2D rotation formula.
mat2 rot2(in float a){ float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }


// IQ's vec2 to float hash.
float hash21(vec2 p){  return fract(sin(dot(p, vec2(127.619, 57.583)))*43758.5453); }

void swap(inout int a, inout int b){ int tmp = a; a = b; b = tmp; }
// A swap without the extra declaration -- It works fine on my machine, but I'm
// not game  enough to use it, yet. :)
//void swap(inout int a, inout int b){ a = a + b; b = a - b; a = a - b; }

 
// Very handy. I rearranged it a bit, but it's the same function.
//
// Smooth Floor - Anastadunbar 
// https://www.shadertoy.com/view/lljSRV
float floorSm(float x, float c) {
    
    float ix = floor(x); x -= ix;
    return (pow(x, c) - pow(1.- x, c))/2. + ix;
}


// Poincare disc setup. Less work than you thiink is necessary.

// N represents the number of polygon vertices, and Q is the number of 
// adjacent polygons to each vertex within the Poincare disc.
//
// For tilings to work, the following must be true: (P − 2)*(Q − 2)>4.
//
// For instance, 3 and 7 will work, but 4 and 4 will not.
//
// 3-7, 3-8, 4-5, 5-4, 6-4, 7-3.
//
// The further caveat with this particular example is that Q must be even
// for the weave pattern to work... Why is a bit of a mystery to me. I'll 
// need to think about it, or wait for someone to explain. :)
//
#ifdef PENTAGON
const int N = 5;	// Polygon vertices.
const int Q	= 4;	// Polygons meeting at a vertex.
#else
const int N = 3;	// Polygon vertices.
const int Q	= 8;	// Polygons meeting at a vertex.
#endif


float count; // Circle inversion count variable. 

// Calculating the initial circular domain according to number of polygon
// sides (N) and the number of adjacent polygons (Q): STB was clever enough to  
// use repeat polar space to position the rest. Anyway, the idea is to use the
// polygon geometry to set up the required geometrical diagram (see the line 
// below), then use a mixture of standard Euclidean and hyperbolic geometry (if
// needed) to calculate the required values, which are described below.
// 
vec3 initDomain(){
    
    // There are quite a few ways to calculate the initial circular domain 
    // values, simply because there are several solutions to the same geometric 
    // problems, which is all this is. In fact, as geometric situations go,
    // this is not a particularly difficult one to solve.
    
    // Essentially, you're going to be reflecting points about polygonal edges, 
    // so you'll need the distance from your initial circle center to that of 
    // the center of the circle that runs adjacent to the current domain edges in
    // order to perform a hyperbolic reflection. You'll need the radius of that 
    // circle as well. The distance and radius will depend directly upon how
    // how many edges your polygon has and how many adjacent polygons (the
    // number that meet at a vertex point) there are.

    // The following is some highschool level circle and triangle geometry to 
    // get the values we're after. Of course, none of this will mean much without 
    // some imagery to refer to. I find the diagrams on the following page more 
    // helpful than most: http://www.malinc.se/noneuclidean/en/poincaretiling.php
    //
     
    /*
    float alpha = PI/float(Q);
    float beta = PI/float(N);
    float gamma = PI/2. - alpha;
    
    // I've left these in this form in order to help follow the logic in the
    // diagram that I've linked to above (about half way down the page). However,
    // converting to cos and sin, etc, would be better.
    float a = tan(gamma);
    float b = tan(beta);

    // Initial polygon radius.
	float polyRad = sqrt((a - b)/(a + b));

    //float OP = (d + 1./d)/2.; 
    //float OM = OP/cos(beta);
    float centDist = (polyRad + 1./polyRad)/2./cos(beta); // OM, on the diagram link above.
    float circRad = ((-polyRad + 1./polyRad)/2.)/cos(gamma); // AM, on the diagram.
    
    return vec3(centDist, circRad, polyRad);
    */
    
    
    // Shadertoy user, SLB, uses the following, which does the exact same
    // thing, but tackles the problem from a different perspective. It can
    // be derived from the calculations above (which I was bored enough
    // to confirm), and since I find them cleaner, I'm using them instead. By 
    // the way, the measurements given back will differ after a few decimal
    // places, so will produce a slightly different random pattern, but 
    // it's essentially the same thing.
    //
    // As an addendum, SLB informed me that the calculations were based on 
    // the following informative article:
    //
    // The Hyperbolic Chamber - Jos Leys
    // http://www.josleys.com/article_show.php?id=83
   
    float piDivN = PI/float(N), piDivQ = piDivN + PI/float(Q) - PI/2.;
    
    // Helper variables.
	vec2 trig1 = vec2(cos(piDivN), sin(piDivN));
    vec2 trig2 = vec2(cos(piDivQ), sin(piDivQ));
    
    // Distance from the current circular domain center to the adjacent
    // circle center, and the radius of that circle. These values will be 
    // the same for all polygons.
    float dist = trig1.x - trig2.x*trig1.y/trig2.y;
    float rad = length(vec2(dist, 0.) - trig1);
    
    float d = max(dist*dist - rad*rad, 0.);

    // The last value is the distance from the current center to each
    // vertex point... so the radius, I guess. It's used as a scaling 
    // component.
	return vec3(dist, rad, 1)/sqrt(d);
    
    /*
    // Shorter version, based off of the geometric diagram in the article above. 
	float a = sin(PI/float(N)), b = cos(PI/float(Q));
    float d2 = cos(PI/float(N) + PI/float(Q))/a;
    float r2 = 1./(b*b/a/a - 1.);
    return sqrt(max(vec3(1. + r2, r2, d2*d2*r2), 0.));    
    */
}


// Hyperbolically reflecting the polygon across each of its edges
// via an inverse Mobius transform. If you can't picture this fromt the 
// example imagery, looking at an image regarding polygon reflection in 
// a Poincare disc will make it clearer.
//
// By the way, if you want to get more of an intuitive feel for circle 
// inversion, I recommend Numberphile's "Epic Circles" video, here:
// https://www.youtube.com/watch?v=sG_6nlMZ8f4
vec2 transform(vec2 p, vec2 circ) {
    
    
    // The following is a standard polar repeat operation. It works
    // the same in hyperbolic space as it does in Euclidian space.
    // If you didn't do this, you'd reflect across just the one
    // edge. Set "ia" to ".5/float(N)" to see what I mean.
     
    float ia = (floor(atan(p.x, p.y)/TAU*float(N)) + .5)/float(N);
    // Start with a point on the boundary of the circle, then use 
    // polar repetition to put it on all the edge boundaries...
    // right in the middle of the edge, which makes sense.
    vec2 vert = rot2(ia*TAU)*vec2(0, circ.x);
   
    // The radius squared of the circle domain you're reflecting to. 
    float rSq = circ.y*circ.y;
    
    // Circle inversion, which relates back to an inverse Mobius
    // transformation. There are a lot of topics on just this alone, but 
    // the bottom line is, if you perform this operation on a point within
    // the Poincare disk, it will be reflected. It's similar to the
    // "p /= dot(p, p)" move that some may have used before.
    vec2 pc = p - vert;
    float lSq = dot(pc, pc);
    
    // If the distance (we're squaring for speed) from the current
    // point to any of the edge vertex points is within the limits, 
    // hyperbolically reflect it.
    if(lSq<rSq){
         
        p = pc*rSq/lSq + vert;
        
        // Attempting to add some extra randomness. Normally,
        // you wouldn't have this here.
        p = rot2(TAU/float(N)*(count + float(Q)))*p;
        
        
        // If we have a hit, increase the counter. This value can be useful
        // for coloring, and other things.
        count++; 
       
    }
     
    return p;
}


// IQ's signed distance to a quadratic Bezier. Like all of IQ's code, it's
// quick and reliable. :)
//
// Quadratic Bezier - 2D Distance - IQ
// https://www.shadertoy.com/view/MlKcDD
float sdBezier(vec2 pos, vec2 A, vec2 B, vec2 C){
  
    // p(t)    = (1 - t)^2*p0 + 2(1 - t)t*p1 + t^2*p2
    // p'(t)   = 2*t*(p0 - 2*p1 + p2) + 2*(p1 - p0)
    // p'(0)   = 2*(p1 - p0)
    // p'(1)   = 2*(p2 - p1)
    // p'(1/2) = 2*(p2 - p0)
    
    vec2 a = B - A;
    vec2 b = A - 2.0*B + C;
    vec2 c = a * 2.0;
    vec2 d = A - pos;

     // If I were to make one change to IQ's function, it'd be to cap off the value 
    // below, since I've noticed that the function will fail with straight lines.
    float kk = 1./max(dot(b,b), 1e-6); // 1./dot(b,b);
    float kx = kk * dot(a,b);
    float ky = kk * (2.0*dot(a,a)+dot(d,b)) / 3.0;
    float kz = kk * dot(d,a);      

    float res = 0.0;

    float p = ky - kx*kx;
    float p3 = p*p*p;
    float q = kx*(2.0*kx*kx - 3.0*ky) + kz;
    float h = q*q + 4.0*p3;

    if(h >= 0.0) 
    { 
        h = sqrt(h);
        vec2 x = (vec2(h, -h) - q) / 2.0;
        vec2 uv = sign(x)*pow(abs(x), vec2(1.0/3.0));
        float t = uv.x + uv.y - kx;
        t = clamp( t, 0.0, 1.0 );

        // 1 root
        vec2 qos = d + (c + b*t)*t;
        res = length(qos);
    }
    else
    {
        float z = sqrt(-p);
        float v = acos( q/(p*z*2.0) ) / 3.0;
        float m = cos(v);
        float n = sin(v)*1.732050808;
        vec3 t = vec3(m + m, -n - m, n - m) * z - kx;
        t = clamp( t, 0.0, 1.0 );

        // 3 roots
        vec2 qos = d + (c + b*t.x)*t.x;
        float dis = dot(qos,qos);
        
        res = dis;

        qos = d + (c + b*t.y)*t.y;
        dis = dot(qos,qos);
        res = min(res,dis);

        qos = d + (c + b*t.z)*t.z;
        dis = dot(qos,qos);
        res = min(res,dis);

        res = sqrt( res );
    }
    
    return res;
}

// Rendering the smooth Bezier segment. The idea is to calculate the midpoint
// between "a.xy" and "b.xy," then offset it by the average of the combined normals
// at "a" and "b" multiplied by a factor based on the length between "a" and "b."
// At that stage, render a Bezier from "a" to the midpoint, then from the midpoint
// to "b." I hacked away to come up with this, which means there'd have to be a more
// robust method out there, so if anyone is familiar with one, I'd love to know.
float doSeg(vec2 p, vec4 a, vec4 b, float r){
    
    // Mid way point.
    vec2 mid = (a.xy + b.xy)/2.; // mix(a.xy, b.xy, .5);
    
    // The length between "a.xy" and "b.xy," multiplied by... a number that seemed
    // to work... Worst coding ever. :D
    float l = length(b.xy - a.xy)*1.732/6.; // ;//(1.4142 - 1.)/1.4142;
 
    // Points on the same edge each have the same normal, and segments between them
    // require a larger arc. There was no science behind the decision. It's just 
    // something I noticed and hacked a solution for. Comment the line out, and you'll 
    // see why it's necessary. By the way, replacing this with a standard semicircular 
    // arc would be even better, but this is easier.
    if(abs(length(b.zw - a.zw))<.01) l = r; 
  
    // Offsetting the midpoint between the exit points "a" and "b"
    // by the average of their normals and the line length factor.
    mid += (a.zw + b.zw)/2.*l;

    // Piece together two quadratic Beziers to form the smooth Bezier curve from the
    // entry and exit points. The only reliable part of this method is the quadratic
    // Bezier function, since IQ wrote it. :
    float b1 = sdBezier(p, a.xy, a.xy + a.zw*l, mid);
    float b2 = sdBezier(p, mid, b.xy + b.zw*l, b.xy);
    
    // Return the minimum distance to the smooth Bezier arc.
    return min(b1, b2);
}

 

// Regular Polygon SDF - BasmanovDaniil
// https://www.shadertoy.com/view/MtScRG
float Polygon(vec2 p, float vertices, float radius){

    float segmentAngle = TAU/vertices;
     
    float angleRadians = atan(p.x, p.y);
    float repeat = mod(angleRadians, segmentAngle) - segmentAngle/2.;
    float inradius = radius*cos(segmentAngle/2.);
    float circle = length(p);
    return cos(repeat)*circle - inradius;
   
}

// Mouse pointer inversion.
vec2 mouseInversion(vec2 p){
    
    // Mouse coordinates.
    vec2 m = vec2((2.*iMouse.xy - iResolution.xy)/iResolution.y);
    // Hack for the zero case instance. If someone has a better way,
    // feel free to let me know.
    if(length(m) < 1e-3) m += 1e-3; 
    // A hack to stop some craziness occurring on the border.
    if(abs(m.x)>.98*.7071 || abs(m.y)>.98*.7071) m *= .98;
    
    // Taking the mouse point and inverting it into the circle domain.
    // Feel free to check some figures, but all will get mapped to 
    // values that lie within circle radius.
    float k = 1./dot(m, m);
    vec2 invCtr = k*m; 
    float t = (k - 1.)/dot(p -invCtr, p - invCtr);
    p = t*p + (1. - t)*invCtr;
    p.x = -p.x; // Keep chirality. MLA does this. 
    
    return p;
    
}

void mainImage(out vec4 fragColor, in vec2 fragCoord){
    
    
    // Aspect correct coordinates: Actually, fragCoord is already in 
    // aspect correct form, so shifting and scaling is all that is
    // required in this particular pixel shader environment.
    vec2 uv = (2.*fragCoord - iResolution.xy)/iResolution.y;
  
    
    #ifdef HALF_PLANE
    // Moving to the half plane model.
    const float sc = 4.;
    uv.y += sc/2. + 1.;
    uv /= dot(uv, uv)/sc;
    uv.y -= 1.; 
    #else   
    // Contracting things just a touch to fit the Poincare domain on 
    // the canvas.
    uv *= 1.05;
    #endif
      
    // Poincare coordinates.
    vec2 p = uv;
 


    // A bit of mouse inversion and rotation to begin with. You'll
    // see this a lot in various hyperbolic examples, and others.
    p = mouseInversion(p);
        
    // Perform some free rotation.
    p = rot2(iTime/16.)*p;
    
    
    // Inversion count. It's used for all kinds of things, like 
    // random number production, etc.
    count = 0.;

    
    // Filling in the domain origin information: From left to right, it 
    // returns the distance between adjacent polygon centers, the adjacent 
    // polygon radius, and the current polygon radius. These values remain
    // the same for all polygons throughout the hyperbolic plane, so this 
    // is all that's required to tile the disc.
    vec3 domInfo = initDomain();
    
 
    // Performing multiple reflective inversions to fill out the disk. Due 
    // to the nature of the hyperbolic transform, the polygon tiles get 
    // smaller as you approach the outer disk, and never reach the edge. 
    // There are a bunch of proofs in complex analysis that illustrate
    // this, but nothing shows it as well as a computer program. :)
    // Drop the number of iterations and you'll see the process unfold.
    //
    // One more important thing I should mention is that there's a test
    // within the transform function (if you're outside the domain) that 
    // would allow you to break early, but I'm bypassing that for 
    // simplicity. The function is cheap, so I'm letting it run through, 
    // but when speed is an issue, it should be considered.
    for(int i=0; i<24; i++){
        p = transform(p, domInfo.xy);
    }
    
    // Handling the imagery outside the Poincare circle domain by inverting or
    // mirroring it back into the circle so that it shows up... or something
    // to that effect... It's been a while. Either way, this is what you do
    // if you want to see the outside pattern. :)
    //
    // By the way, if you want to get more of an intuitive feel for circle 
    // inversion, I recommend Numberphile's "Epic Circles" video, here:
    // https://www.youtube.com/watch?v=sG_6nlMZ8f4
    if(length(p)> 1.) p /= dot(p, p); 
    
      
    // This isn't absolutely necessary, but scaling the transformed coordinates
    // by the disk radius brings things into the zero to one range, which saves
    // a lot of headaches when plotting vertex points, and so forth.
    p /= (domInfo.z);
    
    // Smoothing factor: You could never really get this right. Super sampling
    // would be the best option, but the cheap alternative is kind of OK.
    float sf = .015;
    // Reduces sparkling at the ring edges... Experimental, for now. 
    //float rsf = (2. - smoothstep(0., .2, abs(length(uv) - 1.) - .2));
    //float sf = 4./iResolution.y*rsf;//(count*count + 1.);//fwidth(shape);// 

      
    // Rendering the polygon with a standard routine. I you were after pixel 
    // precise imagery, you'd need to take the overlapping circle point route,
    // or take it a step further and carve out edge circles... At some stage, 
    // I'd like to put together an extruded version, in which case, I'd take
    // the aforementioned into account, but for now, this will work fine. :)
    //
    // By the way, it's possible to account for the warping a bit by pinching 
    // the local coordinates slightly to match the warped hyperbolic space. 
    // It's possible to use a cheap hacky method (see below), rather than the 
    // more advanced method suggested. However, the best way is to use some
    // CSG within the transform function. I'll do that when I put an extruded
    // version together at a later stage.
    //float shape = Polygon(p*(1. - dot(p, p)*domInfo.z/3.), float(N), .72); // Pinched.
    float shape = Polygon(p, float(N), .8);

 
    
    // Lines and vertex points.
    vec2 vp[N];
    float ln[N]; 
    
    // Offset mid points (sitting either side of the midpoint).
    // Shuffle array. This will be used to shuffle rendering order.
    const int N2 = N*2;
    vec4 mp[N2];
    int shuff[N2];
    
    float vert = 1e5;
    vec2 v0 = vec2(0, 1);
    
    for(int i = 0; i<N; i++){
        vert = min(vert, length(p - v0) - .09);//.25*domInfo.z
        vp[i] = v0;        
        v0 = rot2(TAU/float(N))*v0;
    }
    

    // Producing some colored segments based on the angle of the 
    // local polygon cell coordinates.
    vec2 rp = rot2(float(count + 2.)*TAU/float(N))*(p);
    float angl = mod(atan(rp.x, rp.y), TAU)*float(N2)/TAU;
    float triSeg = (floorSm(angl, .1) + .5)/float(N2);
    vec3 oCol = .55 + .45*cos(triSeg*TAU + vec3(0, 1, 2));
    // Fire palette option... Interesting, but I'm giving it a down vote. :D
    //vec3 oCol = pow(vec3(1.5, 1, 1)*max(triSeg, 0.), vec3(1, 3, 16));

    // Debug coloring to test out various color combinations.
    //oCol = vec3(1.2, .8, .5).xzy*(mod(count, float(N)) + 1.)/(float(N) + 1.); 
    //if(mod(float(count), 4.)>1.5) oCol *= vec3(1.2, 1.5, 1);
    
    
    // Coloring up the canvas. All this was added in on the fly. Once you have
    // access to the local coordinates and vertice information, you do
    // whatever you want.
    
    // Background.
    vec3 col = vec3(1, .99, .98);
    
    // The polygon. The default is a triangle, but others will work too.
    col = mix(col, vec3(0), 1. - smoothstep(0., sf, shape));
    col = mix(col, oCol, 1. - smoothstep(0., sf, shape + .04));//.35*f*f

    // Vertices.
    col = mix(col, vec3(0), 1. - smoothstep(0., sf, vert));
    //col = mix(col, vec3(0), (1. - smoothstep(0., sf, abs(length(p) - 1.) - .09)));//.25*domInfo.z
 

    // Polygon side length. Even though they look like different lengths,
    // they're hyperbolically the same.
    float sLength = length(vp[0] - vp[1]);
    
    
    for(int i = 0; i<N; i++){ 
        
        // Initializing the shuffle array while we're here
        shuff[i*2] = i*2;
        shuff[i*2 + 1] = i*2 + 1;
        
        // Constructing the two points on either side of the midpoint and
        // the respective normals to act as entry and exit points for
        // the Bezier curves.
        vec2 mpi = mix(vp[i], vp[(i + 1)%N], .5);
        vec2 tangenti = normalize(mpi - vp[i]);
        //
        // The neat formulae above is how you'd calculate mid points in Euclidean 
        // geometry, whereas in this case, you have to calculate the mid point by rotating
        // the vertex point to the mid point position, then using a known  center to mid 
        // point distance (neighbor to neighbor center to center minus initial radius)... 
        // However, I need to calculate proper edge tangents... I'll leave the
        // approximations above, for now. :)
        //vec2 mpi = (rot2(PI/float(N))*vp[i]/domInfo.z)*(domInfo.x - domInfo.y);
        //vec2 tangenti = normalize(mpi.yx*vec2(1, -1));
        
        // Points flanking either side of the mid point above.
        mp[i*2].xy = mpi - tangenti*sLength/6.;
        mp[i*2 + 1].xy = mpi + tangenti*sLength/6.;
        
        // Respective normals, which are fed into a Bezier line curve formula. That thing
        // is compicated as well, but thankfully, IQ calculated that. :)
        mp[i*2].zw = tangenti.yx*vec2(1, -1);
        mp[i*2 + 1].zw = tangenti.yx*vec2(1, -1);
    
    } 
    
    
    // Shuffling the variable array of points and normals -- Six is the maximum. I think this 
    // is the Fisher–Yates method, but don't quote me on it. It's been a while since I've used 
    // a shuffling algorithm, so if there are inconsistancies, etc, feel free to let me know.
    //
    // For various combinatorial reasons, some non overlapping tiles will probably be 
    // rendered more often, but generally speaking, the following should suffice.
    //
    //int index = N;
    for(int i = N2 - 1; i>0; i--){
        
        // For less than three edges, skip the upper array positions.
        if(i >= N2) continue;
        
        // Using the cell ID and shuffle number to generate a unique random number.
        float fi = float(i);
        
        // Random number for each triangle: The figure "s*3 + ioffs + 1" is unique for
        // each triangle... I can't remember why I felt it necessary to divide by 3,
        // but I'll leave it in there. :)
        float rs = hash21(vec2(count) + domInfo.xy + domInfo.z + fi/float(N2));
        
        // Other array point we're swapping with.
        //int j = int(floor(mod(rs*float(index)*1e6, fi + 1.)));
        // I think this does something similar to the line above, but if not, let us know.
        int j = int(floor(rs*(fi + .9999)));
        swap(shuff[i], shuff[j]);
        
        //swap(mp[i], mp[j]);
        
    }  
     
    
    for(int i = 0; i<N; i++){   
        
        // Shuffled indices.
        int j = shuff[i*2], jp1 = shuff[i*2 + 1];
        
        // Constructing each of the Bezier line segments. Each is
        // rendered from one random entry point to a random exit.
        // It's covered more thoroughly in my other examples, if
        // you're interested in the process.
        // 
        // Offset mid points, and their normal directions.
        float lnOff = sLength/4.; // Quarter side length.
        ln[i] = doSeg(p, mp[j], mp[jp1], lnOff) - .105;
        
        // Interwoven rings.
        // Technically, you need a proper arc formula (which you can find in my 
        // other Truchet examples), but this will illustrate the general idea.
        //ln[i] = abs(length(p - vp[i]) - sLength/1.75) - lw;
    
    }
    
    
    // Fine shape lines. They need to be faded out near the outer ring edges.
    // Hence, the esoteric ring blending add in.
    float ringBlend = smoothstep(0., .235, abs(length(uv) - 1.) - .1);
    float pat2 = abs(fract(shape*16.) - .5)*2. - .05; // Concentric dark lines.
    col = mix(col, vec3(0), ringBlend*(1. - smoothstep(0., .5, pat2))*.7);
    
     for(int i = 0; i<N; i++){
         
         // Rendering the Bezier curves. There's a drop shadow, stroke,
         // colored layer, and fine inner lines. All standard layering stuff.
         
         // Line pattern.
         pat2 = abs(fract(ln[i]*16. + .5) - .5)*2. - .05;
         pat2 = mix(1., 0., ringBlend*(1. - smoothstep(0., .5, pat2))*.7);
         
         // Bezier distance field layers.
         vec3 bgC = col;
         col = mix(col, vec3(0), (1. - smoothstep(0., sf*5., ln[i]))*.3);
         col = mix(col, vec3(0), 1. - smoothstep(0., sf, ln[i]));
         col = mix(col, mix(bgC*1.2, vec3(1, .97, .26), .85)*pat2, 
                   1. - smoothstep(0., sf, ln[i] + .04));
         col = mix(col, col*1.3, 1. - smoothstep(0., sf, ln[i] + .085));
         
    }

    
    // Rendering an outer ring with fake shadwo and all that. Poincare disks can 
    // get pretty busy visually on the outer edges, so I figured this was a kind
    // of artsy way to deal with it. There are others.
    //float lu2 = length(uv - vec2(.3));
    
    // Rotate the coordinates, construct a right, and put a dial pattern on it.
    uv*= rot2(iTime/16.);
    float lu = length(uv); // Circle.
    float ring = abs(lu - 1.) - .05; // Ring distance field.
    
    // Dial pattern.
    float ang = atan(uv.x, uv.y)/TAU;
    float pat = abs(fract(ang*72.) - .5)*2.;
    pat = smoothstep(0., sf, pat - .125);
    

    // Rendering the drop shadow, etc.
   
    col = mix(col, vec3(0), (1. - smoothstep(0., sf*5., ring-.01))*.6);
    col = mix(col, vec3(0), (1. - smoothstep(0., sf/2., ring)));

    col = mix(col, vec3(1, .99, .98)*pat*.35 + .65, (1. - smoothstep(0., .005, ring + .02)));
    ring = max(ring + .02, lu - .995);
    col = mix(col, vec3(.4)*pat, (1. - smoothstep(0., sf/3., ring)));
   
    
    // Black and white imagery outside the ring to give it more of an ink art kind of feel.
    col = mix(col, min( mix(col, vec3(1)*dot(col, vec3(.299, .587, .114)), min(lu*.7, 1.))*1.15, 1.), 
              1. - smoothstep(0., sf, 1.05 - lu));
    
    /*
    // Subtle light... I wasn't feeling it. I was going to put in the whole refracted dome thing,
    // but took the less is more option.
    float spe = max(1. - lu2*lu2*2., 0.);
    col = mix(col, col + pow(spe, 6.)*.05 + pow(spe, 16.)*.3, 1. - smoothstep(0., sf, lu2 - .6));
    col = mix(col, col + lu2*lu2, 1. - smoothstep(0., sf, lu2 - .25));
    */
    
    fragColor = vec4(sqrt(max(col, 0.)), 1.);
}

