// Copied from https://www.shadertoy.com/view/XtfyDX
// Created by https://www.shadertoy.com/user/Shane

/*

	Square Truchet Flow
	-------------------

	After looking at Fabrice's "Smallest Truchet" example then reading one of 
	Iapafoto's comments regarding Truchet animation, I searched Shadertoy for a 
	simple Truchet flow-lines demonstration, but the only one I could find was a
	flowing hexagonal Truchet shader by "klk," and no square Truchet ones at all, 
	which surprised me, since they're kind of interesting looking and easy to produce.

	With that in mind, I dusted off some old code and put one together in less than 
	five minutes... then proceeded to waste way too much time prettying it up when I 
	should've been doing other things. :)

	Here's a brief explanation of the process: Partition into a grid, render the tile,
	use the grid coordinates to obtain the angle using the standard "atan(p.y, p.x)"
	formula, then use that angle and the time to animate something. At that point you'll
	notice that the flowing objects cross the boundaries in opposing directions. You
	can correct that by reversing the flow direction for all neighboring tiles, which
	means in a checkerboard fashion - You perform a similar step when rendering a grid
	of repeat rotating gears. There's a little more to it, but that's the basic idea.

	By the way, I almost rendered some gears on all the corners of the square cells,
	but decided it would complicate the example too much. Besides, I figured it'd be the
	kind of thing that someone like Dr2 would prefer to do anyway. :D

	Anyway, I'm going to get back to the example I'm supposed to be working on. Later,
	I'll have yet another go at animating a 3D Truchet. The comments are very rushed, 
	so I'll get in and tidy those up later too.
	
	If you take the comments away, there's not a great deal of code here, but just in 
	case there's too much window dressing, I've put a much simpler example together 
	using a smaller code imprint here:

	// A minimal implementation to show the main concept - for anyone who doesn't
	// want to sift through all the aesthetic related code in this one. :)
	Minimal Animated Truchet - Shane
	https://www.shadertoy.com/view/XtfyDf

	Other examples:

	// Hexagonal Truchet flow. I'll do one of these later.
	Hexlicity - klk
	https://www.shadertoy.com/view/lt2SzG

	// Fabrice has many Truchet examples. This one sidetracked me into making the 
	// example you're currently viewing. :D
	smallest truchet - FabriceNeyret2
	https://www.shadertoy.com/view/XllyWS

*/


#define SCROLL

// Standard 2D rotation formula.
mat2 r2(in float a){ float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }

// Standard vec2 to float hash - Based on IQ's original.
float hash21(vec2 p){ return fract(sin(dot(p, vec2(141.213, 289.867)))*43758.5453); }

float df(vec2 p, float tF, float s){
    
    return max(abs(p.x)*.866025 + tF*p.y*.5, -tF*p.y) - s;
    
}

float df2(float a, float s){ return abs(a) - s; }


void mainImage( out vec4 fragColor, in vec2 fragCoord ){
    
    // Screen coordinates.
    float res = clamp(iResolution.y, 200., 600.);
    vec2 uv = (fragCoord.xy - iResolution.xy*.5)/res;
    
    // Some minor fish-eye distortion to break up the monotony of the square grid
    // lines, and to give it some subtle - albeit fake - depth.
    uv *= 1. + dot(uv, uv)*.05;
    
    // Wavy screen coordinate distortion... Probably a bit much for this example.
    //uv -= sin(uv*3.14159/2. - cos(uv.yx*3.14159/4. + iTime)*3.14159)*.01;
    
    // Right to left scrolling. I wanted this in to show the continuity of the pattern,
    // but kind of interferes with the flow effect... Anyway, it's optional.
    #ifdef SCROLL
    float tm = iTime/32.;
    // Slowing things down at larger resolutions - Based on Flockaroo's observation.
    if(iResolution.x>800.) tm *= 1.5;
    uv += vec2(tm, 7.);
    #endif
    
    
    // TRUCHET PATTERN
    //
    // Scaling the slightly distorted screen coordinates.
    vec2 p = uv*5.;
    
	// Cell ID. Used to generate unique random numbers for each grid cell.
    vec2 ip = floor(p);
    
    // Grid partitioning. Converting to unit cell coordinates centered at the origin.
    p -= ip + .5; // Equivalent to "p = fract(p) - .5;."

    
    // Flow direction: The flow direction must be reversed every time you enter a 
    // neighboring tile. What this means is that you flip the direction for half the tiles 
    // in a checkerboard pattern. The line below is the way to do that... If you're as slow 
    // as I am, knowing the aforementioned will save you a lot of wasted time. :D
    //
    // I figured this out for myself ages ago, but it took me "way" too long. Set the 
    // direction variable, "dir," to "1." then note that every single neighboring tile has 
    // its direction reversed. Yes, it's obvious, and anyone who's rendered repeat animated 
    // gears would know this, but like I said, I'm a bit slow on the uptake. :D
    // 
    float dir = fract(dot(ip, vec2(.5)))>.25 ? -1. : 1.; 
    // Another - more intuitive - way to write the above:
    //float dir = mod(ip.x + ip.y, 2.)>.5 ? -1. : 1.; 
    
    // A unique random number assigned to each grid tile.
    float rnd = hash21(ip);
    
    
    // Vertical tile flipping. If the random ID for the tile is more than the threshold,
    // reverse the cell's Y coordinate, which effectively vertically flips the tile. Comment
    // the line out and all tiles will have the same orientation, resulting in a pretty
    // lacklustre pattern.
    p.y *= (rnd >.5)? -1. : 1.;
    
    
	
    // This line requires a bit of an explanation: The standard 2D Truchet tile consists of
    // two arcs centered on opposite diagonal corners. Each are mirror reflections of the
    // other about the diagonal line cutting the square grid cell. The following line of
    // code is just a way to repeat space about the diagonal. What this means is that you 
    // can draw just one arc (corner torus) and the one in the other diagaonal corner will
    // be rendered too. The easiest way to see this is to comment the line out and you'll
    // see half the pattern disappear.
    p = p.x>-p.y ? p : -p; 
    // "p *= sign(p.x + p.y)" would be nice, but it can sometimes return zero.
    
    

    // Storage vector. 
    vec2 q;
    
    const float th = .3; // Arc thickness.
    
    // The arc, centered in the top right of the grid cell - or the bottom right if flipped.
    float d = length(p - .5) - .5 - th/2.;
    d = max(d, -th - d); // Remove the inner circles to produce an arc of thickness "th."
    
    // The dashed grid lines.
    float bord = max(abs(p.x), abs(p.y)) - .49;
    q = abs(mod(p, 1./8.) - .5/8.);
    float lines = (min(q.x, q.y) - .5/8./3.);
    bord = min(bord, lines);
    bord = step(0., bord);
    
    // Manipulating the resultant distant field for some cheap edges.
    float ed = max(d -.035, -d);
    ed = smoothstep(0., .0005/max(ed, .0005), ed);
    
    
    // Used for the Truchet pattern border shadows. Produced through trial and error, so 
    // there's probably a cleaner way to write it.
    float sh = smoothstep(0., .075, d - .01) - smoothstep(0., .3, d+.05);
    
    
    
    // Rendering the moving arrows and border dashes.
    
    // Moving the grid coordinates to the corner, or the center of the arc - in order to
    // determine the angle of the pixel subtended to the arc center.
    q = p - .5; 
    
    // The actual animation. You perform that before polar partitioning.
    q = r2(iTime*dir)*q;
    
    // Using the angle to convert into polar coordinates in order to partition into cells 
    // (8, in this case) around the arc, then rendering an object in each.
    const float aNum = 8.;
    float a = atan(q.y, q.x); // Pixel angle.
    float ia = floor(a/6.283*aNum) + .5; // Obtaining the cell centers.
    
    q = r2(ia*6.283/aNum)*q; // Converting to polar coordinates: p.x = radius, p.y = angle.
    q.x -= .5; // Moving the radial coordinate out to the radius of the arc.
    
    // Rendering the arrows - I could have made life easy for myself, rendered a nice 
    // symmetrical dot and left it at that, but I thought arrows would look cooler. It
    // was a bit fidly, but I managed. :)
    //
    // The arrow line - I had to use the angle coordinate itself, and finally figured out
    // that I'd need to used the truchet lines themselves to give the arrow lines curvature...
    // Yeah, I should've rendered animated dots. :D
    float d2 = df2(mod(a - .15*dir, 6.283/aNum) - .5*6.283/aNum, .2);
    d2 = max(d + .14, -d2); // Control the arrow line width with the Truchet distance field.
    
    // Arrow head, or a triangle if you want to get technical. :) The triangles need to be
    // flipped on neighboring cells. I figured that out via observation.
    d2 = min(d2, df(q, dir, .025)); 
    
    // Refining the 2D distance field - Analogous to honing in on the surface edges.
    d2 = smoothstep(0., .015, d2); 
    

    // Border dashes - More lines shaped by the Truchet borders.
    float d4 = df2(mod(a - .15*dir, 6.283/aNum/3.) - .5*6.283/aNum/3., .03);
    d4 = max(abs(abs(d) + .09/3.) - .09, -d4); // Control arrow width.
    d4 = max(d4, d + .05);
    // Refining the distance field - Analogous to honing in on the surface edges.
    d4 = smoothstep(0., .015, d4);
    

    
    // Refining the distance field - Analogous to honing in on the surface edges. I performed 
    // this down here because I think "d" was needed above somewhere to shape some lines.
    d = smoothstep(0., .0005/max(d, .0005), d);
    
    
    // COMBINING ALL THE LAYERS.
	//
    // Mix the background in with the border first. For anyone who isn't aware, if you're mixing
    // in a few layers, you start with the bottom layer first, then progress through to the
    // front layer like so:
    //
    // col = mix(layer1, layer2, maskLayer1*alpha1);
    // col = mix(col, layer3, maskLayer2*alpha2)
    // col = mix(col, layer4, maskLayer3*alpha3)
    // etc.
    //
    // The background - Just some subtle blending between redish colors and some randomized
    // looking hatching. It's a design cliche, but it works. :)
    float blend = dot(sin(uv*3.14159/8. - cos(uv.yx*3.14159/4.)*3.14159), vec2(.25)) + .5;
    vec3 bG = mix(vec3(1.1, .05, .08)*1.1, vec3(.9, .1, 0)*1.1, length(p)*blend);
    // Random looking diagonal hatch lines.
    float hatch = clamp(sin((uv.x - uv.y)*3.14159*160.)*2. + .5, 0., 1.); // Diagonal lines.
    float hRnd = hash21(floor(uv*240.));
    if(hRnd>.66) hatch = hRnd; // Slight, randomization of the diagonal lines.  
    bG *= hatch*.25 + .75; // Combining the colorful gradient background with the lines.
    
    // Mix the background with the border overlay. Note the ".8" value for transparency.
    vec3 col = mix(bG, vec3(0), bord*.8);

    // Back shadow.
    col = mix(col, vec3(.1, .02, .04), sh*.85);
    
    // Truchet overlay.
    vec3 tCol = mix(vec3(1.2), vec3(.7, .75, .8), cos(a*2.*dir)*.5 + .5);
    // "mix(tCol, col, d)" is the same, and more concise, but I wanted to show order.
    col = mix(col, tCol, 1. - d); 
    
    
    // Adding in the edging.
    col = mix(col, vec3(.05, 0, 0), 1. - ed);
    
    
    // Mix in the arrows.
    col = mix(col, vec3(.5, .05, .05), (1. - d2)*.95);
    

    // Dashed borders beside the arrows.
    col = mix(col, vec3(0), (1. - d4)*.75);
    
    
    // Subtle vignette.
    uv = fragCoord/iResolution.xy;
    //col *= pow(16.*uv.x*uv.y*(1. - uv.x)*(1. - uv.y) , .125);
    // Colored variation.
    col = mix(pow(min(vec3(1.5, 1, 1).zyx*col, 1.), vec3(1, 3, 16).zyx), col, 
             pow(16.*uv.x*uv.y*(1. - uv.x)*(1. - uv.y) , .125));    
    
    
    
    // Screen color.
	fragColor = vec4(sqrt(max(col, 0.)), 1);
    
}
