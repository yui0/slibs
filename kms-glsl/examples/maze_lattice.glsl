// Copied from https://www.shadertoy.com/view/llGGzh
// Created by https://www.shadertoy.com/user/Shane

/*

    Maze Lattice
    ------------

	Applying a geometric pattern and edging to Fizzer's maze lattice. I'm not sure what he technically 
	calls it, but it has a lattice feel, and as he infers in his article, it's a 3D structure with 
	maze-like qualities. Either way, I particularly like it because it's cleverly constructed, interesting 
	looking, and very cheap to produce. Basically, it's one of those distance fields that gives you your 
	money's worth. :)

	The only interesting thing here is the distance field equation that contains the maze-like lattice. I've 
	given a rough explanation behind its construction, but it's much better to read Fizzer's well explained 
	article, which I've provided a link to below.

	The geometric surfacing pattern is a 2D hexagonal Truchet design, which is applied to each of the flat 
    face sections in accordance to the surface normal orientation. It's a standard way to apply 2D patterns 
	to a cuboid-based surface, and is contained in the "texFaces" function.

	I used an edging algorithm to obtain the edges, which involves extra distance function and bump calls. 
	I've since thought of a better way to make that happen which will cut down on cost and complexity, so 
	I'll apply that in due course.

	Anyway, I have a reflection\refraction version, based on the same surface that I'll release later.
	
	Distance field based on the article accompanying the following:
	Maze Explorer - fizzer
    https://www.shadertoy.com/view/XsdGzM

	Accompanying article is here:
	Implicit Maze-Like Patterns
    http://amietia.com/slashmaze.html

	Truchet shaders:

    hexagonal truchet ( 352 ) - FabriceNeyret2
    https://www.shadertoy.com/view/Xdt3D8
 
    hexagonal tiling - mattz
    https://www.shadertoy.com/view/4d2GzV
    


*/

// Maximum ray distance.
#define FAR 40.

// I love this distance field. So elegant, and I can thank Fizzer for coming up with it.
// The idea is about as simple as it gets. Break space into octahedrons then use each
// otahedral cell to obtain a unique ID. Use that ID to render a randomly oriented square 
// tube and you're done.
//
// I've done a little trimming and shuffling, which probably confuses things slightly. 
// Either way, it's worth reading the following article for a much clearer explanation:
//
// Implicit Maze-Like Patterns - Fizzer
// http://amietia.com/slashmaze.html
//
float map(in vec3 p) {
    
    
   // Cubes, for a simpler, more orderly scene.
   //p = abs(fract(p) - .5);    
   //return max(max(p.x, p.y), p.z) - .225;
   
   // Unique identifier for the cube, but needs to be converted to a unique ID
   // for the nearest octahedron. The extra ".5" is to save a couple of 
   // of calculations. See below.
   vec3 ip = floor(p) + .5;
    
   p -= ip; // Break space into cubes. Equivalent to: fract(p) - .5.
    
   // Stepping trick used to identify faces in a cube. The center of the cube face also
   // happens to be the center of the nearest octahedron, so that works out rather well. 
   // The result needs to be factored a little (see the hash line), but it basically  
   // provides a unique octahedral ID. Fizzer provided a visual of this, which is easier 
   // to understand, and worth taking a look at.
   vec3 q = abs(p); 
   q = step(q.yzx, q.xyz)*step(q.zxy, q.xyz)*sign(p); // Used for cube mapping also.
   
   // Put the ID into a hash function to produce a unique random number. Reusing "q" to
   // save declaring a float. Don't know if it's faster, but it looks neater, I guess.
   q.x = fract(sin(dot(ip + q*.5, vec3(111.67, 147.31, 27.53)))*43758.5453);
    
   // Use the random number to orient a square tube in one of three random axial
   // directions... See Fizzer's article explanation. It's better. :) By the way, it's
   // possible to rewrite this in "step" form, but I don't know if it's quicker, so I'll
   // leave it as is for now.
   p.xy = abs(q.x>.333 ? q.x>.666 ? p.xz : p.yz : p.xy);
   return max(p.x, p.y) - .2;   

}

// Very basic raymarching equation. I thought I might need to use something more sophisticated,
// but it turns out that this structure raymarches reasonably well. Not all surfaces do.
float trace(vec3 ro, vec3 rd){

    float t = 0.0;
    for(int i=0; i< 72; i++){
        float d = map(ro + rd*t);
        if (abs(d) < 0.002*(t*.125 + 1.) || t>FAR) break;
        t += d;
    } 
    return min(t, FAR);
}

// The reflections are pretty subtle, so not much effort is being put into them. Only a few iterations.
float refTrace(vec3 ro, vec3 rd){

    float t = 0.0;
    for(int i=0; i< 16; i++){
        float d = map(ro + rd*t);
        if (abs(d) < 0.005*(t*.25 + 1.) || t>FAR) break;
        t += d;
    } 
    return t;
}

// The normal function with some edge detection rolled into it. Sometimes, it's possible to get away
// with six taps, but we need a bit of epsilon value variance here, so there's an extra six.
vec3 normal(in vec3 p, inout float edge) { 
	
    vec2 e = vec2(.034, 0); // Larger epsilon for greater sample spread, thus thicker edges.

    // Take some distance function measurements from either side of the hit point on all three axes.
	float d1 = map(p + e.xyy), d2 = map(p - e.xyy);
	float d3 = map(p + e.yxy), d4 = map(p - e.yxy);
	float d5 = map(p + e.yyx), d6 = map(p - e.yyx);
	float d = map(p)*2.;	// The hit point itself - Doubled to cut down on calculations. See below.
     
    // Edges - Take a geometry measurement from either side of the hit point. Average them, then see how
    // much the value differs from the hit point itself. Do this for X, Y and Z directions. Here, the sum
    // is used for the overall difference, but there are other ways. Note that it's mainly sharp surface 
    // curves that register a discernible difference.
    edge = abs(d1 + d2 - d) + abs(d3 + d4 - d) + abs(d5 + d6 - d);
    //edge = max(max(abs(d1 + d2 - d), abs(d3 + d4 - d)), abs(d5 + d6 - d)); // Etc.
    
    
    // Once you have an edge value, it needs to normalized, and smoothed if possible. How you 
    // do that is up to you. This is what I came up with for now, but I might tweak it later.
    //
    edge = smoothstep(0., 1., sqrt(edge/e.x*8.));
    
    // Curvature. All this, just to take out the inner edges.
    float crv = (d1 + d2 + d3 + d4 + d5 + d6 - d*3.)/e.x;;
    //crv = clamp(crv*32., 0., 1.);
    if (crv<0.) edge = 0.; // Comment out to see what it does.

	
    // Redoing the calculations for the normal with a more precise epsilon value. If you can roll the 
    // edge and normal into one, it saves a lot of map calls. Unfortunately, we want wide edges, so
    // there are six more, making 12 map calls in all. Ouch! :)
    e = vec2(.005, 0);
	d1 = map(p + e.xyy), d2 = map(p - e.xyy);
	d3 = map(p + e.yxy), d4 = map(p - e.yxy);
	d5 = map(p + e.yyx), d6 = map(p - e.yyx); 
    
    // Return the normal.
    // Standard, normalized gradient mearsurement.
    return normalize(vec3(d1 - d2, d3 - d4, d5 - d6));
}

// Ambient occlusion, for that self shadowed look.
// XT95 came up with this particular version. Very nice.
//
// Hemispherical SDF AO - https://www.shadertoy.com/view/4sdGWN
// Alien Cocoons - https://www.shadertoy.com/view/MsdGz2
float calcAO( in vec3 p, in vec3 n )
{
    
	float ao = 0.0, l;
	const float nbIte = 12.0;
	const float falloff = 1.;
    
    const float maxDist = 1.;
    for( float i=1.; i< nbIte+.5; i++ ){
    
        l = (i + fract(cos(i)*45758.5453))*.5/nbIte*maxDist;
        ao += (l - map( p + n*l ))/ pow(1. + l, falloff);
    }
	
    return clamp( 1. - ao*2./nbIte, 0., 1.);
}


// Cheap shadows are hard. In fact, I'd almost say, shadowing repeat objects - in a setting like this - with limited 
// iterations is impossible... However, I'd be very grateful if someone could prove me wrong. :)
float softShadow(vec3 ro, vec3 lp, float k){

    // More would be nicer. More is always nicer, but not really affordable... Not on my slow test machine, anyway.
    const int maxIterationsShad = 16; 
    
    vec3 rd = (lp-ro); // Unnormalized direction ray.

    float shade = 1.0;
    float dist = 0.05;    
    float end = max(length(rd), 0.001);
    float stepDist = end/float(maxIterationsShad);
    
    rd /= end;

    // Max shadow iterations - More iterations make nicer shadows, but slow things down. Obviously, the lowest 
    // number to give a decent shadow is the best one to choose. 
    for (int i=0; i<maxIterationsShad; i++){

        float h = map(ro + rd*dist);
        //shade = min(shade, k*h/dist);
        shade = min(shade, smoothstep(0.0, 1.0, k*h/dist)); // Subtle difference. Thanks to IQ for this tidbit.
        //dist += min( h, stepDist ); // So many options here: dist += clamp( h, 0.0005, 0.2 ), etc.
        dist += clamp(h, 0.02, 0.25);
        
        // Early exits from accumulative distance function calls tend to be a good thing.
        if (h<0.001 || dist > end) break; 
    }

    // I've added 0.5 to the final shade value, which lightens the shadow a bit. It's a preference thing.
    return min(max(shade, 0.) + 0.3, 1.0); 
}


// Simple hexagonal truchet patten. This is based on something Fabrice and Mattz did.
//
// hexagonal truchet ( 352 ) - FabriceNeyret2
// https://www.shadertoy.com/view/Xdt3D8
//
// hexagonal tiling - mattz
// https://www.shadertoy.com/view/4d2GzV
float hexTruchet(in vec2 p) { 
    
    p *= 6.;
    
	// Hexagonal coordinates.
    vec2 h = vec2(p.x + p.y*.577350269, p.y*1.154700538);
    
    // Closest hexagon center.
    vec2 f = fract(h); h -= f;
    float c = fract((h.x + h.y)/3.);
    h =  c<.666 ?   c<.333 ?  h  :  h + 1.  :  h  + step(f.yx, f); 

    p -= vec2(h.x - h.y*.5, h.y*.8660254);
    
    // Rotate (flip, in this case) random hexagons. Otherwise, you'd hava a bunch of circles only.
    // Note that "h" is unique to each hexagon, so we can use it as the random ID.
    c = fract(cos(dot(h, vec2(41.13, 289.57)))*43758.5453); // Reusing "c."
    p -= p*step(c, .5)*2.; // Equivalent to: if (c<.5) p *= -1.;
    
    // Minimum squared distance to neighbors. Taking the square root after comparing, for speed.
    // Three partitions need to be checked due to the flipping process.
    p -= vec2(-1, 0);
    c = dot(p, p); // Reusing "c" again.
    p -= vec2(1.5, .8660254);
    c = min(c, dot(p, p));
    p -= vec2(0, -1.73205);
    c = min(c, dot(p, p));
    
    return sqrt(c);
    
    // Wrapping the values - or folding the values over (abs(c-.5)*2., cos(c*6.283*1.), etc) - to produce 
    // the nicely lined-up, wavy patterns. I"m perfoming this step in the "map" function. It has to do 
    // with coloring and so forth.
    //c = sqrt(c);
    //c = cos(c*6.283*2.) + cos(c*6.283*4.);
    //return (clamp(c*.6+.5, 0., 1.));

}

// Bumping the faces.
float bumpFunc(vec3 p, vec3 n){
    
    // Mapping the 3D object position to the 2D UV coordinate of one of three
    // orientations, which are determined by the dominant normal axis.    
    n = abs(n);
    p.xy = n.x>.5? p.yz : n.y>.5? p.xz : p.xy; 
    
    // Wavy, 70s looking, hexagonal Truchet pattern.
    vec2 sc = (cos(hexTruchet(p.xy)*6.283*vec2(2, 4)));
    return clamp(dot(sc, vec2(.6)) + .5, 0., 1.);

}

// Standard function-based bump mapping function.
vec3 bumpMap(in vec3 p, in vec3 n, float bumpfactor){
    
    const vec2 e = vec2(0.002, 0);
    float ref = bumpFunc(p, n);                 
    vec3 grad = (vec3(bumpFunc(p - e.xyy, n),
                      bumpFunc(p - e.yxy, n),
                      bumpFunc(p - e.yyx, n) )-ref)/e.x;                     
          
    grad -= n*dot(n, grad);          
                      
    return normalize( n + grad*bumpfactor );
	
}

// Bumping the edges with some block partitions. Made up on the spot. 
float bumpFunc2(vec3 p, vec3 n){
    
    // Partition space to produce some smooth blocks.
    p = abs(fract(p*3.) - .5);
    float c = max(max(p.x, p.y), p.z);
    
    return 1. - smoothstep(0., .025, c - .47);
    
}

// A second function-based bump mapping function. Used for
// the edging. Messy, but probably faster... probably. :)
vec3 bumpMap2(in vec3 p, in vec3 n, float bumpfactor){
    
    const vec2 e = vec2(0.002, 0);
    float ref = bumpFunc2(p, n);                 
    vec3 grad = (vec3(bumpFunc2(p - e.xyy, n),
                      bumpFunc2(p - e.yxy, n),
                      bumpFunc2(p - e.yyx, n) )-ref)/e.x;                     
          
    grad -= n*dot(n, grad);          
                      
    return normalize( n + grad*bumpfactor );
	
}

// Cheap and nasty 2D smooth noise function with inbuilt hash function - based on IQ's 
// original. Very trimmed down. In fact, I probably went a little overboard. I think it 
// might also degrade with large time values. I'll swap it for something more robust later.
float n2D(vec2 p) {

	vec2 i = floor(p); p -= i; p *= p*(3. - p*2.);  
    
	return dot(mat2(fract(sin(vec4(0, 41, 289, 330) + dot(i, vec2(41, 289)))*43758.5453))*
                vec2(1. - p.y, p.y), vec2(1. - p.x, p.x) );

}

// Texturing the sides with a 70s looking hexagonal Truchet pattern.
vec3 texFaces(in vec3 p, in vec3 n){
    
    // Use the normal to determine the face. Dominant "n.z," then use the XY plane, etc.
    n = abs(n);
    p.xy = n.x>.5? p.yz : n.y>.5? p.xz : p.xy; 

    // Some fBm noise based bluish red coloring.
    n = mix(vec3(.3, .1, .02), vec3(.35, .5, .65), n2D(p.xy*8.)*.66 + n2D(p.xy*16.)*.34);
    n *= n2D(p.xy*512.)*1.2 + 1.4;
    
    //n =  n*.3 + min(n.zyx*vec3(1.3, .6, .2)*.75, 1.)*.7;
   
    // Overlaying with the hexagonal Truchet pattern.
    vec2 sc = (cos(hexTruchet(p.xy)*6.283*vec2(2, 4)));
    n *= clamp(dot(sc, vec2(.6))+.5, 0., 1.)*.95 + .05;
    
    return min(n, 1.);

}

// Terxturing the edges with something subtle.
vec3 texEdges(in vec3 p, in vec3 n){
    
    float bf = bumpFunc2(p, n); // Bump function.
    
    // 2D face selection.
    n = abs(n);
    p.xy = n.x>.5? p.yz : n.y>.5? p.xz : p.xy; 

    // Mixing color with some fBm noise.
    n = mix(vec3(.3, .1, .02), vec3(.35, .5, .65), n2D(p.xy*8.)*.66 + n2D(p.xy*16.)*.34);
    n *= n2D(p.xy*512.)*.85 + .15; 
    
    // More coloring.
    n = min((n + .35)*vec3(1.05, 1, .9), 1.);
    
    // Running the bump function over the top for some extra depth.
    n *= bf*.75+.25;
    
    return n;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ){
    
    
    // Unit direction ray vector: Note the absence of a divide term. I came across
    // this via a comment Shadertoy user "coyote" made. I'm pretty happy with this.
    vec3 rd = vec3(2.*fragCoord - iResolution.xy, iResolution.y);
    
    // Barrel distortion;
    rd = normalize(vec3(rd.xy, sqrt(max(rd.z*rd.z - dot(rd.xy, rd.xy)*.2, 0.))));
    
    // Rotating the ray with Fabrice's cost cuttting matrix. I'm still pretty happy with this also. :)
    vec2 m = sin(vec2(1.57079632, 0) + iTime/4.);
    rd.xy = rd.xy*mat2(m.xy, -m.y, m.x);
    rd.xz = rd.xz*mat2(m.xy, -m.y, m.x);
    
    // Ray origin: Sending it along the Z-axis.
    vec3 ro = vec3(0, 0, iTime);
    // Alternate: Set off in the YZ direction. Note the ".5." It's an old lattice trick.
    //vec3 ro = vec3(0, iTime/2. + .5, iTime/2.);
    
    vec3 lp = ro + vec3(.2, 1., .3); // Light, near the ray origin.
    
    // Set the initial scene color to black.
    vec3 col = vec3(0);

    
    float t = trace(ro, rd); // Raymarch.
    
    // Normally, you'd only light up the scene if the distance is less than the outer boundary.
    // However, in this case, since most rays hit, I'm clamping to the far distance, and doing
    // the few extra calculations. The payoff (I hope) is not having a heap of nested code.
    // Whether that results in more speed, or not, I couldn't really say, but I'd imagine you'd
    // receive a slight gain... maybe. If the scene were more open, you wouldn't do this.
    //if(t<FAR){
        
        float edge;
        vec3 sp = ro + rd*t; // Surface position.
        vec3 sn = normal(sp, edge); // Surface normal.

    	// Saving a copy of the unbumped normal, since the texture routine require it.
    	// I found that out the hard way. :)
        vec3 svn = sn;
		
    	// Bump mapping the faces and edges. The bump factor is reduced with distance
    	// to lessen artifacts.
        if(edge<.001) sn = bumpMap(sp, sn, .01/(1. + t*.25));
        else sn = bumpMap2(sp, sn, .03/(1. + t*.25));

        vec3 ref = reflect(rd, sn); // Reflected ray.

        vec3 oCol = texFaces(sp, svn); // Texture color at the surface point.
        if(edge>.001) oCol = texEdges(sp, svn);


        float sh = softShadow(sp, lp, 16.); // Soft shadows.
        float ao = calcAO(sp, sn); // Self shadows. Not too much.

        vec3 ld = lp - sp; // Light direction.
        float lDist = max(length(ld), 0.001); // Light to surface distance.
        ld /= lDist; // Normalizing the light direction vector.

        float diff = max(dot(ld, sn), 0.); // Diffuse component.
        float spec = pow(max(dot(reflect(-ld, sn), -rd), 0.), 32.); // Specular.

        float atten = 1.25/(1.0 + lDist*0.1 + lDist*lDist*.05); // Attenuation.



        ///////////////
        // Cheap reflection: Not entirely accurate, but the reflections are pretty subtle, so not much 
        // effort is being put in.
        //
        float rt = refTrace(sp + ref*0.1, ref); // Raymarch from "sp" in the reflected direction.
        float rEdge;
        vec3 rsp = sp + ref*rt; // Reflected surface hit point.
        vec3 rsn = normal(rsp, rEdge); // Normal at the reflected surface.
        //rsn = bumpMap(rsp, rsn, .005); // We're skipping the reflection bump to save some calculations.

        vec3 rCol = texFaces(rsp, rsn); // Texel at "rsp."    
        if(rEdge>.001)rCol = texEdges(rsp, rsn); // Reflection edges.

        float rDiff = max(dot(rsn, normalize(lp-rsp)), 0.); // Diffuse light at "rsp."
        float rSpec = pow(max(dot(reflect(-normalize(lp-rsp), rsn), -ref), 0.), 8.); // Diffuse light at "rsp."
        float rlDist = length(lp - rsp);
        // Reflected color. Not entirely accurate, but close enough. 
        rCol = (rCol*(rDiff*1. + vec3(.45, .4, .3)) + vec3(1., .6, .2)*rSpec*2.);
        rCol *= 1.25/(1.0 + rlDist*0.1 + rlDist*rlDist*.05);    
        ////////////////


        // Combining the elements above to light and color the scene.
        col = oCol*(diff*1. + vec3(.45, .4, .3)) + vec3(1., .6, .2)*spec*2.;


        // Adding the reflection to the edges and faces. Technically, there should be less on the faces,
        // but after all that trouble, I thought I'd bump it up a bit. :)
        if(edge<.001) col += rCol*.2;
        else col += rCol*.35;
        // Alternate way to mix in the reflection. Sometimes, it's preferable, but not here.
        //if(edge<.001) col = mix(col, rCol, .35)*1.4;
        //else col = mix(col, rCol, .5)*1.4;


        // Shading the scene color and clamping. By the way, we're letting the color go beyond the maximum to
        // let the structure subtly glow a bit... Not really natural, but it looks a little shinier.
        col = min(col*atten*sh*ao, 1.);
    
    //}
    
    // Mixing in some hazy bluish orange background.
    vec3 bg = mix(vec3(.5, .7, 1).zyx, vec3(1, .7, .3).zyx, -rd.y*.35 + .35);
    col = mix(col, bg, smoothstep(0., FAR-25., t));//min(bg.zyx*vec3(1.3, .6, .2)*1.5, 1.)
    
    // Postprocesing - A subtle vignette with a bit of warm coloring... I wanted to warm the atmosphere up
    // a bit. Uncomment it, if you want to see the bluer -possibly more natural looking - unprocessed version.
    vec2 uv = fragCoord/iResolution.xy;
    float vig = pow(16.*uv.x*uv.y*(1.-uv.x)*(1.-uv.y), 0.125);
    col *= vec3(1.2, 1.1, .85)*vig;

    // Rough gamma correction.
	fragColor = vec4(sqrt(clamp(col, 0., 1.)), 1.0);
    
}
