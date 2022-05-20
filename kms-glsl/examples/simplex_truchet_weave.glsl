// Copied from https://www.shadertoy.com/view/4ltyRn
// Created by https://www.shadertoy.com/user/Shane

/*

	Simplex Truchet Weave
	---------------------

	Mattz made a pretty cool 2D simplex grid-based Truchet pattern a while back,
    which sent me on a tangent to make a 3D simplex grid version. At the time, I 
	also considered making a 2D simplex weave, but got sidetracked -- probably 
	by another of Mattz's interesting examples. :) Anyway, I finally got around
	to it. 

	A hexagonal weave arrangement has more variation, but I think the triangular 
	version has a certain tightly wound appeal. I tried to track down an example 
	image on the internet to reference, but couldn't find one, so didn't know
	what the pattern would look like until I'd coded it. 

	Thankfully, the construction process was pretty simple: First, set up a 2D 
	simplex grid -- the equilateral triangle one. For each grid (triangle) cell, 
	render a torus at each of the vertices. Make sure the center of the ring of 
	each toroidal segment cuts the midpoint of each of the triangle edge lines. 
	The magic happens when you render the three tori in random order, which
	results in the spaghetti-like pattern you see.

	How one sets about randomizing the rendering order depends upon the preferred 
	technique of the individual, but I've explained how I went about it in the 
	code. On a side note, most of the code in this example is window dressing. 
	The pattern portion isn't long at all. I might put together a minimal version
	to illustrate that... or wait for Fabrice or Greg Rostami o do it. :)

	I coded this for a bit of fun, but it's possible to apply the pattern to far 
	more interesting things. For instance, it can be applied to anthing that's
	constructed with equilateral triangles -- like an icosahedral surface, etc. 

	On the 3D side, I've already rendered some prisms in a simplex arrangement, 
	but I'd like to put together an extruded version of this particular random 
	pattern at some stage... I'm not looking forward to it, so I hope it looks 
	interesting when completed. :)

	By the way, there's a "NO_WEAVE" option below that is mildy interesting. 
	Plus, a "SHOW_SIMPLEX_GRID" option for anyone who requires a rough visual 
	cell tile construction aid.



	Similar examples:

	// A triangular Truchet pattern. This one is rendered in a non-overlapping
	// fashion.
	slimy triangular truchet - mattz
	https://www.shadertoy.com/view/lsffzX

	// I was pretty enamored with this example, so posted a 2D and 3D example not 
	// long afterward.    
	Hexagonal Truchet Weaving - BigWIngs
	https://www.shadertoy.com/view/llByzz


*/


// You can order the random object heights from lowest to heighest to render what
// look like tori randomly stacked on top of one another... I'm probably not 
// describing that very well. Uncommenting will make it clearer, but in essence,
// the weave involves random render ordering, so to take out the weaving effect,
// you need to use specific front-to-back ordering.
//#define NO_WEAVE

// Curve shape - Circle: 0, Hexagon: 1, Dodecahedron: 2.
#define SHAPE 0 

// A visual aid to enable you to see the simplex cell (triangle) borders.
//#define SHOW_SIMPLEX_GRID


// Standard 2D rotation formula.
mat2 rot2(in float a){ float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }


// Standard 2x2 hash algorithm.
vec2 hash22(vec2 p) { 
    
    
    // Faster, but probaly doesn't disperse things as nicely as other methods.
    float n = sin(dot(p, vec2(1, 113)));
    p = fract(vec2(2097152, 262144)*n);
    
    //return p*2. - 1.;
    
    return sin(p*6.283 + iTime*2.);
    //return abs(fract(p+ iTime*.5)-.5)*4.-1.; // Snooker.
    //return abs(cos(p*6.283 + iTime*2.))*2.-1.; // Bounce.

}


float hash21(vec2 p){
 
    float n = dot(p, vec2(127.183, 157.927));
    n = fract(sin(n)*43758.5453);
    
    return n;
    
    //return sin(n*6.2831853 + iTime*.125)*.5 + .5;
    //return abs(fract(n + iTime*.125) - .5)*2.;
    
}
 

#ifdef SHOW_SIMPLEX_GRID
// Unsigned distance to the segment joining "a" and "b."
float distLine(vec2 a, vec2 b){
    
	b = a - b;
	float h = clamp(dot(a, b) / dot(b, b), 0., 1.);
    return length(a - b*h);
}
#endif

// Distance metric.
float dist(vec2 p){
    
    #if SHAPE == 0
    return length(p); // Circle.
    #elif SHAPE == 1
    p *= rot2(3.14159/12.);
    p = abs(p);
    return max(p.y*.8660254 + p.x*.5, p.x); // Hexagon.
    #else
    p *= rot2(3.14159/12.);
    p = abs(p);
    vec2 p2 = p*.8660254 + p.yx*.5;
    return max(max(p2.x, p2.y), max(p.x, p.y)); // Dodecahedron.
    #endif
    
}


// Simplex Truchet weave function.

vec3 simplexWeave(vec2 p){
    
    // Keeping a copy of the orginal position.
    vec2 oP = p;
    
    // Scaling constant.
    const float gSc = 5.;
    p *= gSc;
    
    
    // SIMPLEX GRID SETUP
    
    vec2 s = floor(p + (p.x + p.y)*.36602540378); // Skew the current point.
    
    p -= s - (s.x + s.y)*.211324865; // Use it to attain the vector to the base vertex (from p).
    
    // Determine which triangle we're in. Much easier to visualize than the 3D version.
    float i = p.x < p.y? 1. : 0.; // Apparently, faster than: i = step(p.y, p.x);
    vec2 ioffs = vec2(1. - i, i);
    
    // Vectors to the other two triangle vertices.
    vec2 p1 = p - ioffs + .2113248654, p2 = p - .577350269; 
 
    
    
    ////////////
    // SIMPLEX NOISE... or close enough.
    //
    // We already have the triangle points, so we may as well take the last few steps to
    // produce some simplex noise.
    //
    // Vector to hold the falloff value of the current pixel with respect to each vertice.
    vec3 d = max(.5 - vec3(dot(p, p), dot(p1, p1), dot(p2, p2)), 0.); // Range [0, 0.5]
    //
    // Determining the weighted contribution of each random gradient vector for each point...
    // Something to that effect, anyway. I could save three hash calculations below by using 
    // the following line, but it's a relatively cheap example, and I wanted to keep the noise 
    // seperate. By the way, if you're after a cheap simplex noise value, the calculations 
    // don't have to be particularly long. From here to the top, there's only a few lines, and 
    // the quality is good enough.
    vec3 w = vec3(dot(hash22(s), p), dot(hash22((s + ioffs)), p1), dot(hash22(s + 1.), p2));
    //
    // Combining the above to achieve a rough simplex noise value.
    float noise = clamp(0.5 + dot(w, d*d*d)*12., 0., 1.);    
    ////////////
    
    
    // THE WEAVE PATTERN
    
    // Three random values -- taken at each of the triangle vertices, and ranging between zero 
    // and one. Since neighboring triangles share vertices, the segments are guaranteed to meet
    // at edge boundaries, provided the right shape is chosen, etc.
    vec3 h = vec3(hash21(s), hash21((s + ioffs)), hash21(s + 1.));
    //vec3 h = vec3(length(hash22(s)), length(hash22(s - vec2(i, sc - vec2(i, 1. + i)))), length(hash22(s + 1.)))*.35;

    
    #ifdef NO_WEAVE
    // To draw the stacked circle version, the layers need to be have a lighting range from zero to one,
    // but have to be distinct (not equal) for ordering purposes. To ensure that, I've spaced the layers
    // out by a set amount, then with a little hack, seperated X from Y and Z, then Y from Z... I think
    // the logic is sound? Either way, I'm not noticing any random flipping, so it'll do.
    h = floor(h*15.999)/15.;
    if(h.x == h.y) h.y += .0001;
    if(h.x == h.z) h.z += .0001;
    if(h.y == h.z) h.z += .0001;
    #endif
    
     
    
    // Angles subtended from the current position to each of the three vertices... There's probably a 
    // symmetrical way to make just one "atan" call. Anyway, you can use these angular values to create 
    // patterns that follow the contours. In this case, I'm using them to create some cheap repetitious lines.
    vec3 a = vec3(atan(p.y, p.x), atan(p1.y, p1.x), atan(p2.y, p2.x));
 
    // The torus rings. 
    // Toroidal axis width. Basically, the weave pattern width.
    float tw = .25;
    // With the hexagon shape, the pattern width needs to be decreased.
    #if SHAPE == 1 
    tw = .19;
    #endif
    // For symmetry, we want the middle of the torus ring to cut dirrectly down the center
    // of one of the equilateral triangle sides, which is half the distance from one of the
    // vertices to the other. Add ".1" to it to see that it's necessary.
    float mid = dist((p2 - p))*.5;
    // The three distance field functions: Stored in cir.x, cir.y and cir.z.
    vec3 cir = vec3(dist(p), dist(p1), dist(p2));
    // Equivalent to: vec3 tor =  cir - mid - tw; tor = max(tor, -(cir - mid + tw));
    vec3 tor =  abs(cir - mid) - tw;

    
    // It's not absolutely necessary to scale the distance values by the scaling factor, but I find
    // it helps, since it allows me scale up and down without having to change edge widths, smoothing
    // factor variables, and so forth.
    tor /= gSc;
    cir /= gSc;

    
    
    

    #ifdef NO_WEAVE
    // Front to back ordering:
    //
    // Specifically ordering the torus rings based on their individual heights -- as
    // opposed to randomly ordering them -- will create randomly stacked rings, which
    // I thought was interesting enough to include... But at the end of the day, it's
    // probably not all that interesting. :D
    
    // I'm not sure how fond I am of the following hacky logic block, but it's easy 
    // enough to follow, plus it gets the job done:
    //
    // If the torus assoicated with the X vertex is lowest, render it first, then
    // check to see whether Y or Z should be rendered next. Swap the rendering order --
    // via swizzling -- accordingly. Repeat the process for the other vertices.
    //
    if(h.x<h.y && h.x<h.z){ // X vertex is lowest.
        
        // If you reorder one thing, you usually have to reorder everything else.
        // Forgetting to do this, which I often do, sets me up for a lot of debugging. :)
        if(h.z<h.y) { tor = tor.xzy; h = h.xzy; a = a.xzy; }
        else {  tor = tor.xyz; h = h.xyz; a = a.xyz; }
    }
    else if(h.y<h.z && h.y<h.x) {  // Y vertex is lowest.
        
         if(h.z<h.x) { tor = tor.yzx; h = h.yzx; a = a.yzx; }
         else { tor = tor.yxz; h = h.yxz; a = a.yxz; }
    }
    else { // Z vertex is lowest.
        
        if(h.y<h.x) { tor = tor.zyx; h = h.zyx; a = a.zyx; }
        else { tor = tor.zxy; h = h.zxy; a = a.zxy;}
    }
    
    #else
    // Random order logic to create the weave pattern: Use the unique
    // ID for this particular simplex grid cell to generate a random
    // number, then use it to randomly mix the order via swizzling
    // combinations. For instance, "c.xyz" will render layer X, Y then
    // Z, whereas the swizzled combination "c.zyx" will render them
    // in reverse order. There are six possible order combinations.
    // The order in which you render the tori surrounding the three
    // vertices will result in the spaghetti-like pattern you see.
    //
    // On a side note, including all six ordering possibilities 
    // guarantees that the pattern randomization is maximized, but
    // there's probably a simpler way to achieve the same result.
    
    // Random value -- unique to each grid cell.
    float dh = hash21((s + s + ioffs + s + 1.));
    if(dh<1./6.){ tor = tor.xzy; a = a.xzy; }
    else if(dh<2./6.){ tor = tor.yxz; a = a.yxz; }
    else if(dh<3./6.){ tor = tor.yzx; a = a.yzx; }
    else if(dh<4./6.){ tor = tor.zxy; a = a.zxy; }
    else if(dh<5./6.){ tor = tor.zyx; a = a.zyx; } 
   
    #endif

    
    // RENDERING
    // Applying the layered distance field objects.
    
    // The background. This one barely shows, so is very simple.
    vec3 bg = vec3(.075, .125, .2)*noise;
    bg *= clamp(cos((oP.x - oP.y)*6.2831*128.)*1., 0., 1.)*.15 + .925;
   
    // The scene color. Initialized to the background.
    vec3 col = bg;
    
    // Outer torus ring color. Just a bit of bronze.
    vec3 rimCol = vec3(1, .7, .5);
    // Applying some contrasty noise for a fake shadowy lighting effect. Since the 
    // noise is simplex based, the shadows tend to move in a triangular motion that
    // matches the underlying grid the pattern was constucted with.
    rimCol *= (smoothstep(0., .75, noise - .1) + .5);
    
    // Toroidal segment color. The angle is being used to create lines run perpendicular 
    // to the curves.
    vec3 torCol = vec3(.2, .4, 1);
    a = clamp(cos(a*48. + iTime*0.)*1. + .5, 0., 1.)*.25 + .75;
    
    // Using the tori's distance field to produce a bit of faux poloidal curvature.
    // The value has also been repeated to create the line pattern that follows the
    // pattern curves... Set it to "vec3(1)" to see what it does. :)
    vec3 cc = max(.05 - tor*32., 0.);
    cc *= clamp(cos(tor*6.2831*80.)*1. + .5, 0., 1.)*.25 + .75;
    #ifdef NO_WEAVE
    // If not using a weave pattern, you end up with some distinct, stacked tori,
    // which means you can use the random height values to shade them and introduce 
    // some depth information.
    cc *= (h*.9 + .1);
    #endif
        
    // Smoothing factor and line width.
    const float sf = .005, lw = .005;
   
    // Rendering the the three ordered (random or otherwise) objects:
    //
    // This is all pretty standard stuff. If you're not familiar with using a 2D
    // distance field value to mix a layer on top of another, it's worth learning.
    // On a side note, "1. - smoothstep(a, b, c)" can be written in a more concise
    // form (smoothstep(b, a, c), I think), but I've left it that particular way
    // for readability. You could also reverse the first two "mix" values, etc.
    // By readability, I mean the word "col" is always written on the left, the
    // "0." figure is always on the left, etc. If this were a more GPU intensive
    // exercise, then I'd rewrite things.
    
    // Bottom toroidal segment.
    //
    // Drop shadow with 50% transparency.
    col = mix(col, vec3(0), (1. - smoothstep(0., sf*4., tor.x - .00))*.5);
    // Outer dark edges.
    col = mix(col, vec3(0), 1. - smoothstep(0., sf, tor.x));
    // The bronze toroidal outer rim color.
    col = mix(col, rimCol*cc.x, 1. - smoothstep(0., sf, tor.x + lw));
    // The main blueish toroidal face with faux round shading and pattern.
    col = mix(col, torCol*col.x*a.x, 1. - smoothstep(0., sf, tor.x + .015));
    // Some inner dark edges. Note the "abs." This could be rendered before
    // the later above as a thick dark strip...
    col = mix(col, vec3(0), 1. - smoothstep(0., sf, abs(tor.x + .015)));
    
    // Same layring routine for the middle toroidal segment.
    col = mix(col, vec3(0), (1. - smoothstep(0., sf*4., tor.y - .00))*.5);
    col = mix(col, vec3(0), 1. - smoothstep(0., sf, tor.y));
    col = mix(col, rimCol*cc.y, 1. - smoothstep(0., sf, tor.y + lw)); 
    col = mix(col, torCol*col.x*a.y, 1. - smoothstep(0., sf, tor.y + .015));
    col = mix(col, vec3(0), 1. - smoothstep(0., sf, abs(tor.y + .015)));

    // Render the top toroidal segment last.
    col = mix(col, vec3(0), (1. - smoothstep(0., sf*4., tor.z - .00))*.5);
	col = mix(col, vec3(0), 1. - smoothstep(0., sf, tor.z));
    col = mix(col, rimCol*cc.z, 1. - smoothstep(0., sf, tor.z + lw));
    col = mix(col, torCol*col.x*a.z, 1. - smoothstep(0., sf, tor.z + .015));
    col = mix(col, vec3(0), 1. - smoothstep(0., sf, abs(tor.z + .015)));
    

    
    #ifdef SHOW_SIMPLEX_GRID
    // Displaying the 2D simplex grid. Basically, we're rendering lines between
    // each of the three triangular cell vertices to show the outline of the 
    // cell edges.
    vec3 c = vec3(distLine(p, p1), distLine(p1, p2), distLine(p2, p));
    c /= gSc;
    c.x = min(min(c.x, c.y), c.z);
    torCol = col;
    col = mix(col, vec3(0), (1. - smoothstep(0., sf*2., c.x - .005))*.65);
    col = mix(col, torCol*3., (1. - smoothstep(0., sf/2., c.x - .0015))*.75);
    #endif
    
   
    // Just the simplex noise, for anyone curious.
    //return vec3(noise);
    
    // Return the simplex weave value.
    return col;
 

}



void mainImage( out vec4 fragColor, in vec2 fragCoord ){

    // Screen coordinates. I've put a cap on the fullscreen resolution to stop
    // the pattern looking too blurred out.
	vec2 uv = fragCoord/min(750., iResolution.y);
   
    // Position with some scrolling.
    vec2 p = uv + vec2(.8660254, .5)*iTime/16.; 
    
    // Screen rotation to level the pattern, but I liked the current angle.
    //p *= rot2(-3.14159/12.); 
    
    // The simplex Truchet weave routine.
    vec3 col = simplexWeave(p);
    
    // Using the Y screen coordinate to produce a subtle change in color.
    col = mix(col, col.yxz, -uv.y*.5 + .5);
    
    // A subtle grid line overlay.
    //col *= clamp(cos((p.x - p.y)*6.2831*96.)*1., 0., 1.)*.2 + .9;
    
     
    // Subtle vignette.
    uv = fragCoord/iResolution.xy;
    //col *= pow(16.*uv.x*uv.y*(1. - uv.x)*(1. - uv.y) , .0625) + .1;
    // Colored variation.
    col = mix(col.zyx/2., col, pow(16.*uv.x*uv.y*(1. - uv.x)*(1. - uv.y) , .125));

 	
    // Rough gamma correction.
    fragColor = vec4(sqrt(clamp(col, 0., 1.)), 1);
    
}

