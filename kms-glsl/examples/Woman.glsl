// [SH18] Woman. Created by Reinder Nijhoff 2018
// Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// @reindernijhoff
//
// https://www.shadertoy.com/view/4tdcWS
//
// I wanted to create an organic-looking SDF scene in a single, fully procedural,
// fragment shader. The scene is modelled for this specific camera viewpoint and 
// lighting setup.
//
// Please change AA (line 13) to 1 if this shader is running slow.
//

#define AA 1
#define FLOOR 0.
#define BODY 1.
#define HAIR 2.

//
// Hash functions by Dave Hoskins:
//
// https://www.shadertoy.com/view/4djSRW
//

float hash12(vec2 p) {
    vec3 p3  = fract(vec3(p.xyx) * 443.8975);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

vec3 hash33(vec3 p3) {
    p3 = fract(p3 * vec3(443.897, 441.423, 437.195));
    p3 += dot(p3, p3.yxz + 19.19);
    return fract((p3.xxy + p3.yxx)*p3.zyx);
}

float noise(in vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f*f*(3. -2.*f);
    
    return mix(mix(hash12(i + vec2(0, 0)), 
                   hash12(i + vec2(1, 0)), u.x), 
               mix(hash12(i + vec2(0, 1)), 
                   hash12(i + vec2(1, 1)), u.x), u.y);
}

//
// SDF framework by Inigo Quilez:
//
// https://www.shadertoy.com/view/Xds3zN
//

vec2 boxIntersect(in vec3 ro, in vec3 rd, in vec3 rad) {
    vec3 m = 1./rd;
    vec3 n = m*ro;
    vec3 k = abs(m)*rad;
    
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    
    float tN = max(max(t1.x, t1.y), t1.z);
    float tF = min(min(t2.x, t2.y), t2.z);
    
    if(tN > tF || tF < .0) return vec2(-1);
    
    return vec2(tN, tF);
}

float smin(float a, float b, float k) {
    float h = clamp(.5 + .5*(b - a)/k, .0, 1.);
    return mix(b, a, h) - k * h * (1. - h);
}

float udRoundBox(vec3 p, vec3 b, float r) {
    return length(max(abs(p)-b, .0)) -r;
}

float sdCapsuleF(vec3 p, vec3 a, vec3 b, const float r0, const float r1, const float f) {
    vec3 d = b -a;
    float h = length(d);
    d = normalize(d);
    float t=dot(p-a, d);
    float th = t/h;
    return distance(a+clamp(t,0.,h)*d, p)-mix(r0, r1, th) * 
           max(0., 1.+f-f*4.*abs(th-.5)*abs(th -.5));
}

float sdCapsule(vec3 p, vec3 a, vec3 b, const float r0, const float r1) {
    vec3 d = b -a;
    float h = length(d);
    d = normalize(d);
    float t=clamp(dot(p-a, d), 0., h);
    return distance(a+t*d, p) -mix(r0, r1, t/h);
}

float mapHand(in vec3 p) {
    float sph = length(p) - .1;
    if (sph > .1) return sph; //  bounding sphere
    
    const float s = 1.175;
    float d = udRoundBox(p, vec3(.0175/s + p.y * (.25/s), .035/s + p.x * (.2/s), 0.), .01);
    d = smin(d, min(sdCapsule(p, vec3(.025, .0475, 0)/s, vec3(.028, .08, .02)/s, .01/s, .0075/s), 
                    sdCapsule(p, vec3(.028, .08, .02)/s, vec3(.03, 0.1, .06)/s, .0075/s, .007/s)), .0057);
    d = smin(d, min(sdCapsule(p, vec3(.01, .0425, 0)/s, vec3(.008, .07, .025)/s, .009/s, .0075/s), 
                    sdCapsule(p, vec3(.008, .07, .025)/s, vec3(.008, .085, .065)/s, .0075/s, .007/s)), .0057);
    d = smin(d, min(sdCapsule(p, vec3(-.01, .04, 0)/s, vec3(-.012, .065, .028)/s, .009/s, .0075/s), 
                    sdCapsule(p, vec3(-.012, .065, .028)/s, vec3(-.012, .07, .055)/s, .0075/s, .007/s)), .0057);
    d = smin(d, min(sdCapsule(p, vec3(-.025, .035, 0)/s, vec3(-.027, .058, .03)/s, .009/s, .0075/s), 
                    sdCapsule(p, vec3(-.027, .058, .03)/s, vec3(-.028, .06, .05)/s, .0075/s, .007/s)), .0057);
    return d;
}

vec2 map(in vec3 pos) {
    const float f0 = .075;
    const float f1 = .2;
    const float f2 = .275;
    
    vec3 ph = pos;
    
    if (pos.x < 0.) {
        ph += vec3(.11, -.135, .2);
        ph = mat3(-0.8674127459526062, -0.49060970544815063, 0.08304927498102188, 0.22917310893535614, -0.5420454144477844, -0.8084964156150818, 0.4416726529598236, -0.6822674870491028, 0.5826116800308228) * ph;
    } else {
        ph.x = -ph.x;
        ph += vec3(.075, -.09, .125);
        ph = mat3(-0.6703562140464783, -0.7417424321174622, 0.020991835743188858, 0.36215442419052124, -0.3517296612262726, -0.8632093667984009, 0.6476624608039856, -0.5710554718971252, 0.5044094920158386) * ph;
    }
    
    float dh = mapHand(ph);
    
    //  right arm
    float d = sdCapsuleF(pos, vec3(0.13, 0.535, -.036), vec3(.09, 0.292, -0.1), .035, .025, f1);
    d = smin(d, sdCapsuleF(pos, vec3(.08, 0.29, -0.1), vec3(-.09, 0.15, -0.17), .03, .02, f0), .0051);
    if (pos.x < 0.) d = smin(d, dh, .015);
    
    //  left arm
    float d1 = sdCapsuleF(pos, vec3(-0.12, 0.56, .02), vec3(-0.11, 0.325, -.045), .035, .025, f1);
    d1 = smin(d1, sdCapsuleF(pos, vec3(-0.11, 0.315, -.05), vec3(.07, .08, -0.11), .024, .022, f2), .005);
    if (pos.x > 0.) d1 = smin(d1, dh, .015);
    d = min(d1, d);
    
    //  body
    vec3 bp1 = pos;
    bp1 += vec3(0, -.44, -.027);
    bp1 = mat3(0.9761762022972107, 0.033977385610342026, 0.2143024057149887, -0.07553963363170624, 0.9790945649147034, 0.18885889649391174, -0.20340539515018463, -0.20054790377616882, 0.9583353996276855) * bp1;
    float db = udRoundBox(bp1, vec3(.07 + bp1.y*.3, 0.135 -abs(bp1.x)*0.2, 0.), .04);
    
    vec3 bp2 = pos;
    bp2 += vec3(-.032, -.235, -.06);
    bp2 = mat3(0.8958174586296082, -0.37155669927597046, 0.24383758008480072, 0.3379548490047455, 0.9258314967155457, 0.16918234527111053, -0.28861331939697266, -0.0691504031419754, 0.9549453258514404) * bp2;
    db = smin(db, udRoundBox(bp2, vec3(.065 - bp2.y*.25, 0.1, .02 -bp2.y*.13), .04), .03);
    
    db = smin(db, sdCapsule(pos, vec3(0.11, 0.5, -.032), vec3(.05, 0.52, -.015), .04, .035), .01);
    db = smin(db, sdCapsule(pos, vec3(.01, 0.4, -.01), vec3(.01, 0.7, .0), .045, .04), .02);
    
    vec3 bp3 = pos;
    bp3 += vec3(-.005, -.48, .018);
    bp3 = mat3(0.9800665974617004, 0.05107402056455612, 0.19199204444885254, 0, 0.9663899540901184, -0.2570805549621582, -0.19866932928562164, 0.2519560754299164, 0.9471265077590942) * bp3;
    db = smin(db, udRoundBox(bp3, vec3(.056 + bp3.y*.23 , .06, 0.), .04), .01);
    
    d = smin(d, db, .01);
    
    //  right leg
    float d2 = sdCapsuleF(pos, vec3(0.152, 0.15, .05), vec3(-.03, 0.43, -.08), .071, .055, f2);
    d2 = smin(d2, sdCapsuleF(pos, vec3(0.14, .08, .05), vec3(-.01, 0.23, -.02), .05, .02, f1), .075);
    d = min(d, d2);
    float d3 = sdCapsuleF(pos, vec3(-.03, 0.43, -.084), vec3(.055, .04, -.04), .053, .02, f0);
    d3 = smin(d3, sdCapsuleF(pos, vec3(-.0, 0.35, -.05), vec3(.025, 0.2, -.03), .04, .02, f2), .05);
    d = min(d, d3);
    
    //  left leg
    d = min(d, sdCapsuleF(pos, vec3(-.02, 0.12, 0.1), vec3(-0.145, .08, -0.17), .07, .055, f2));
    float d4 = sdCapsuleF(pos, vec3(-0.145, .08, -0.17), vec3(0.205, .02, -0.09), .05, .0185, f0);
    d4 = smin(d4, sdCapsuleF(pos, vec3(-.05, .085, -0.145), vec3(.05, .03, -.09), .035, .03, f2), .0075);
    
    //  right feet
    float d6 = distance(pos, vec3(.0, .0, -0.1)) -.1; //  bounding sphere
    if(d6 < 0.1) {
        d = min(d, sdCapsule(pos, vec3(.03, .03, -.08), vec3(.031, .01, -0.146), .015, .005));
        d = min(d, sdCapsule(pos, vec3(.02, .03, -.08), vec3(.018, .01, -0.1505), .015, .006));
        d = min(d, sdCapsule(pos, vec3(.00, .03, -.08), vec3(.005, .01, -0.1525), .015, .007));
        d = min(d, sdCapsule(pos, vec3(-.01, .03, -.08), vec3(-.014, .01, -0.1575), .015, .01));
    } else {
        d = min(d6, d);
    }
    
    //  left feet
    float d5 = distance(pos, vec3(0.25, .025, -0.1)) -.12; //  bounding sphere
    if(d5 < 0.1) {
        d5 = sdCapsule(pos, vec3(0.2, .035, -.075), vec3(0.3, .01, -.09), .035, .02);
        d5 = smin(d5, sdCapsule(pos, vec3(0.31, .035, -.0975), vec3(0.1, .01, -0.10), .015, .02), .02);
        d5 = smin(d5, sdCapsule(pos, vec3(0.31, .035, -.0975), vec3(0.355, .034, -0.10), .015, .01), .005);
        d5 = min(d5, sdCapsule(pos, vec3(0.31, .022, -.0875), vec3(0.335, .022, -.09), .02, .01));
    }
    d4 = smin(d4, d5, .025);
    d = min(d, d4);
    
    //  hair
    vec3 hp = pos;
    hp.x += smoothstep(.55, .45, pos.y)*.035;
    hp.z *= 1.9 - .8 * pos.y;
    hp.yz -= 2.*pos.x*pos.x;
    float h = sdCapsule(hp, vec3(.0, 0.725, -.02), vec3(-.02, 0.415, .0), .094, .085);
    h = smin(h, sdCapsule(hp, vec3(.0, 0.725, -.02), vec3(.06, 0.705, -.05), .085, .095), .02);
    h = max(-(pos.y - abs(fract(pos.x*90.) -.5)*0.1 -.14 - smoothstep(-0.2, 0.1, pos.x)*.5), h);
    
    return (h < d) ? vec2(h, HAIR) : vec2(d, BODY);
}


float calcSoftshadow(in vec3 ro, in vec3 rd, in float mint, in float tmax) {
    float res = 1.;
    float t = mint;
    for(int i=0; i<14; i++) {
        float h = map(ro + rd*t).x;
        res = min(res, 8.*h/t);
        t += max(h, .02);
        if(res<.005 || t>tmax) break;
    }
    return clamp(res,0.,1.);
}

vec3 calcNormal(in vec3 pos) {
    vec2 e = vec2(1,-1)*.00005;
    return normalize(e.xyy*map(pos + e.xyy).x +
                     e.yyx*map(pos + e.yyx).x +
                     e.yxy*map(pos + e.yxy).x +
                     e.xxx*map(pos + e.xxx).x);
}

float calcAO(in vec3 pos, in vec3 nor) {
    float occ = 0.;
    float sca = 1.;
    for(int i=0; i<5; i++) {
        float hr = .005 + .12*float(i)/4.;
        vec3 aopos =  nor * hr + pos;
        float dd = min(aopos.y, map(aopos).x);
        occ += -(dd -hr)*sca;
        sca *= .95;
    }
    return clamp(1. - 3.*occ, 0., 1.);
}

float render(in vec3 ro, in vec3 rd, in vec2 uv) {
    //  cast ray
    float planeIntersect = abs(-ro.y/rd.y);
    vec2 box = boxIntersect(ro, rd, vec3(.37, 1, .3));
    float t = box.x;
    float tmax = min(box.y, planeIntersect);
    
    float m = FLOOR;
    if (t > 0.) {
        for(int i=0; i<40; i++) {
            float precis = .0004*t;
            vec2 res = map(ro+rd*t);
            m = res.y;
            if(abs(res.x) < precis || t > tmax) break;
            t += res.x;
        }
    }
    if(t>=tmax || t<0.) {
        t = rd.y < 0. ? planeIntersect : 1000.;
        m = FLOOR;
    }
    
    //  shade scene
    float col = 0.;
    if(t < 10.) {
        vec3 pos = ro + t*rd;
        vec3 nor = vec3(0, 1, 0);
        
        if (m < FLOOR + .5) {
            col = .03;
        } else {
            col = .5;
            nor = calcNormal(pos);
        }
        if (m > BODY + .5) {
            col = 0.;
        }
        nor = normalize(nor + (hash33(pos) -.5) * .1);
        vec3 ligp = vec3(5., 5., -.5);
        vec3 lig = -normalize(pos - ligp);
        float dif = clamp(dot(nor, lig), 0., 1.);
        float bac = clamp(dot(nor, normalize(vec3(-.2, .5, -.02))), .0, 1.0)
                    *clamp(1.-pos.y,0.,1.);  
        if(m > FLOOR) bac *= clamp(-10.*pos.z+.4,0.,1.);
        
        float occ, sha;
        if (pos.x > .4 || pos.z > 1.) {
            occ = sha = 1.;
        } else {
            occ = calcAO(pos, nor);
            sha = calcSoftshadow(pos, lig, .01, 1.5);
        }
        
        col *= 1.30*dif*sha*(.25+.75*occ) + .5*bac*occ;
        
        if (m > BODY + .5) {
            // totally fake hair lighting
            vec3 ref = reflect(rd, nor);
            vec3 hal = normalize(normalize(vec3(-.1, .5, .35)) -rd);
            vec2 hv = mix(vec2(pos.x*60. -pos.z*35., 0.), 
                          vec2(-pos.x*50. -pos.z*50., 0.), 
                          smoothstep(.0, .01, -dot(pos, normalize(vec3(-1., .15, .45)))));
            float n = noise(hv*20. + vec2(2. + 10.*sin(pos.y*20.+.4), 2.5));
            n = smoothstep(.4, 1., n);
            float nd = noise(hv*40.+ vec2(2. + 10.*sin(pos.y*20.+.4), .5));
            n *= nd * nd;
            col += n * pow(max(0., dot(ref, hal)), 8.);
            col += .03 * n * max(0., dot(ref, hal)) * smoothstep(.75, .5, pos.y);
        }
        
        col *= smoothstep(.985, 1., dot(normalize(vec3(0.7, 0.58, -.05)), lig));
    }
    
    return clamp(col, 0., 1.);
}

mat3 setCamera(in vec3 ro, in vec3 ta) {
    vec3 cw = normalize(ta -ro);
    vec3 cu = normalize(cross(cw, vec3(0,1,0)));
    vec3 cv = normalize(cross(cu, cw));
    return mat3(cu, cv, cw);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    float angle = -.18+ .18*sin(iTime*0.5);
    float tot = 0.;
    
    for(int m=0; m<AA + min(0,iFrame); m++)
        for(int n=0; n<AA + min(0,iFrame); n++) {
            vec2 o = vec2(float(m), float(n))/float(AA) - .5;
            vec2 p = (-iResolution.xy + 2.*(fragCoord+o))/iResolution.y;
            
            p.x -= .8;
            
            vec3 ro = vec3(2.9*sin(angle) , .65, -2.9*cos(angle));
            vec3 ta = vec3(0., 0.45, 0.);
            mat3 ca = setCamera(ro, ta);
            vec3 rd = ca * normalize(vec3(p.xy, 5));
            
            float col = render(ro, rd, p);
            tot += pow(col, .4545);
        }
    tot /= float(AA*AA);
    
    tot += .075 * hash12(fragCoord/iResolution.xy);
    tot *= 1.35;
    
    fragColor = vec4(min(tot*vec3(1, .97, .92), 1.), 1.);
}

void mainVR( out vec4 fragColor, in vec2 fragCoord, in vec3 ro, in vec3 rd ) {
    vec2 p = (-iResolution.xy + 2.*fragCoord)/iResolution.y;
    float c = pow(render(ro + vec3(0,.65,-1), rd, p), .4545);
    fragColor = vec4(c,c,c,1.);
}
