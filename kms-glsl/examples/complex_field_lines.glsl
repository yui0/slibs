// Copied from https://www.shadertoy.com/view/7ds3W2
// Created by https://www.shadertoy.com/user/jllusty

float pi = 4.0 * atan(1.0);

vec2 mult(in vec2 a, in vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y);
}


vec2 div(in vec2 a, in vec2 b) {
    return mult(a,vec2(b.x,-b.y))/length(b);
}

vec2 clog(in vec2 z) {
    return vec2(log(length(z)), atan(z.y, z.x));
}

vec2 func(in vec2 z) {
    //return mult(z - vec2(2.0f, 0.0), div(vec2(1.0,0.0), z + vec2(sin(iTime), 0.0)));
    return clog(z-vec2(-4.,1.)) + clog(z-vec2(0.,2.)) - mult(vec2(2.,0.), clog(z-vec2(2.,-2.)));
}

vec3 checkerboard(in vec2 uv) {
    int parity = int(floor(uv.x)) + int(floor(uv.y));
    if(mod(float(parity),2.0) == 0.0) {
        return vec3(0.0);
    }
    else {
        return vec3(1.0);
    }
}

//float wrap(float x, float a) {
//    return x - a * floor(x / a);
//}


// Smooth wrap.
float wrap(float x, float a){
    x -= a*floor(x/a);
    // Smoothing factor.
    const float sf = 16.; 
    // Rough smoothing.
    return min(x, (1. - x)*x*16.);
}

float angdist(in float theta1, in float theta2) {
    return wrap(theta2 - theta1 + pi, 2.0*pi) - pi;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = 4.0*(2.0*fragCoord/iResolution.xy - 1.);
    float aspect = iResolution.y/iResolution.x;
    uv.x = uv.x/aspect;

    // Save Pixel Position
    vec2 pos = uv;

    // Apply Function
    uv = func(uv);
    
    // Get distance to closest arg(z)-isocurve
    float n = 22.;
    float arg = uv.y; //atan(uv.y, uv.x);
    float gEps = 0.001;
    float stretch = length(func(pos+vec2(gEps,gEps))-func(pos))/(sqrt(2.)*gEps);
    float eps = 0.05;
    float d = abs(fract(arg*n/(2.*pi)+.5)-.5)/stretch;
    float r = length(uv);
    //if (r >= 1.) d = d*r;
    float c = 1. - smoothstep(eps, 2.*eps, d);
    
    //
    float n1 = 22.;
    float arg1 = uv.x-iTime/4.; //atan(uv.y, uv.x);
    float gEps1 = 0.001;
    float stretch1 = length(func(pos+vec2(gEps1,gEps1))-func(pos))/(sqrt(2.)*gEps1);
    float eps1 = 0.05;
    float d1 = abs(fract(arg1*n1/(2.*pi)+.5)-.5)/stretch1;
    float r1 = length(uv);
    //if (r >= 1.) d = d*r;
    float c1 = 1. - smoothstep(eps1, 2.*eps1, d1);

    // Stack onto Base Image (Contours)
    //  background color
    float bc = 1.0-wrap(length(uv.x-iTime),1.);
    //  line color
    c = max(c,c1);
    vec3 lcol = vec3(1.0,0.0,0.0);
    //vec3 col = (1.0 - c) * bcol + c * vec3(abs(sin(uv.x-iTime)), 0.0, 0.0);
    vec3 col = vec3(0.);
    if (bc > c) {
        col = c*vec3(abs(sin(uv.x)),0.,0.);
    } else {
        col = bc*vec3(1.);
    }

    // Output to screen
    fragColor = vec4(col,1.0);
}
