// "Endless Penguins" by dr2 - 2018
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

#define AA        0   // optional antialiasing

float PrSphDf (vec3 p, float r);
float PrCylDf (vec3 p, float r, float h);
float PrEllipsDf (vec3 p, vec3 r);
float SmoothMin (float a, float b, float r);
float SmoothBump (float lo, float hi, float w, float x);
vec2 PixToHex (vec2 p);
vec2 HexToPix (vec2 h);
vec2 Rot2D (vec2 q, float a);
float Hashfv2 (vec2 p);
vec2 Hashv2v2 (vec2 p);
float Fbm2 (vec2 p);
vec3 VaryNf (vec3 p, vec3 n, float f);

vec3 sunDir, qHit;
vec2 gId, obOff;
float dstFar, tCur, fAng, obRot, szFac, hgSize;
int idObj;
const float pi = 3.14159, sqrt3 = 1.7320508;

#define DMINQ(id) if (d < dMin) { dMin = d;  idObj = id;  qHit = q; }

float ObjDf (vec3 p)
{
  vec3 q;
  float dMin, d, dh;
  dMin = dstFar;
  if (szFac > 0.) {
    p.xz -= HexToPix (gId * hgSize) + obOff;
    p.xz = Rot2D (p.xz, obRot);
    dMin /= szFac;
    p /= szFac;
    p.y -= 1.55;
    q = p;
    d = PrEllipsDf (q.xzy, vec3 (1.3, 1.2, 1.4));
    q.y -= 1.5;
    dh = PrEllipsDf (q.xzy, vec3 (0.8, 0.6, 1.3));
    q = p;
    q.x = abs (q.x);
    q -= vec3 (0.3, 2., -0.4);
    d = SmoothMin (d, max (dh, - PrCylDf (q, 0.15, 0.3)), 0.2);
    DMINQ (1);
    q = p;
    q.yz -= vec2 (1.6, -0.6);
    d = max (PrEllipsDf (q, vec3 (0.4, 0.2, 0.6)), 0.01 - abs (q.y));
    DMINQ (2);
    q = p;
    q.x = abs (q.x);
    q -= vec3 (0.3, 2., -0.4);
    d = PrSphDf (q, 0.15);
    DMINQ (3);
    q = p;
    q.x = abs (q.x);
    q.xy -= vec2 (0.4, -0.8);
    d = PrCylDf (q.xzy, 0.12, 0.7);
    DMINQ (4);
    q -= vec3 (0.1, -0.67, -0.4);
    q.xz = Rot2D (q.xz, -0.07 * pi);
    d = PrEllipsDf (q.xzy, vec3 (0.15, 0.5, 0.05));
    q.z -= 0.5;
    q.xz = Rot2D (q.xz, 0.15 * pi);
    q.z -= -0.5;
    d = SmoothMin (d, PrEllipsDf (q.xzy, vec3 (0.15, 0.5, 0.05)), 0.05);
    q.z -= 0.5;
    q.xz = Rot2D (q.xz, -0.3 * pi);
    q.z -= -0.5;
    d = SmoothMin (d, PrEllipsDf (q.xzy, vec3 (0.15, 0.5, 0.05)), 0.05);
    DMINQ (5);
    q = p;
    q.x = abs (q.x);
    q -= vec3 (1.1, 0.3, -0.2);
    q.yz = Rot2D (q.yz, -0.25 * pi);
    q.xy = Rot2D (q.xy, fAng) - vec2 (0.1, -0.4);
    d = PrEllipsDf (q.xzy, vec3 (0.05, 0.25, 0.9));
    DMINQ (6);
    q = p;
    q.y -= -1.53;
    d = abs (q.y) - 0.01;
    DMINQ (7);
    dMin *= szFac;
  }
  return dMin;
}

float GrndHt (vec2 p)
{
  return Fbm2 (0.05 * p) - 0.5;
}

void SetPngConf ()
{
  vec2 g, w;
  float t;
  g = HexToPix (gId * hgSize);
  if (GrndHt (g) < 0.) szFac = 0.;
  else {
    t = Hashfv2 (17. * gId + 99.);
    szFac = 0.8 * (0.5 * sqrt3 - 0.3 * t);
    obRot = 2. * pi * Hashfv2 (37. * gId + 99.);
    w = Hashv2v2 (73. * gId + 77.);
    obOff = hgSize * max (0., 0.5 * sqrt3 - szFac) * w.x * sin (2. * pi * w.y + vec2 (0.5 * pi, 0.));
    fAng = -0.2 * pi + 0.1 * pi * SmoothBump (0.25, 0.75, 0.1, mod (0.2 * tCur + 1.3 * t, 1.)) *
       sin (16. * pi * tCur);
  }
}

float ObjRay (vec3 ro, vec3 rd)
{
  vec3 vri, vf, hv, p;
  vec2 edN[3], pM, gIdP;
  float dHit, d, s;
  edN[0] = vec2 (1., 0.);
  edN[1] = 0.5 * vec2 (1., sqrt3);
  edN[2] = 0.5 * vec2 (1., - sqrt3);
  for (int k = 0; k < 3; k ++) edN[k] *= sign (dot (edN[k], rd.xz));
  vri = hgSize / vec3 (dot (rd.xz, edN[0]), dot (rd.xz, edN[1]), dot (rd.xz, edN[2]));
  vf = 0.5 * sqrt3 - vec3 (dot (ro.xz, edN[0]), dot (ro.xz, edN[1]),
     dot (ro.xz, edN[2])) / hgSize;
  pM = HexToPix (PixToHex (ro.xz / hgSize));
  gIdP = vec2 (-99.);
  dHit = 0.;
  for (int j = 0; j < 200; j ++) {
    hv = (vf + vec3 (dot (pM, edN[0]), dot (pM, edN[1]), dot (pM, edN[2]))) * vri;
    s = min (hv.x, min (hv.y, hv.z));
    p = ro + dHit * rd;
    gId = PixToHex (p.xz / hgSize);
    if (gId.x != gIdP.x || gId.y != gIdP.y) {
      gIdP = gId;
      SetPngConf ();
    }
    d = ObjDf (p);
    if (dHit + d < s) {
      dHit += d;
    } else {
      dHit = s + 0.002;
      pM += sqrt3 * ((s == hv.x) ? edN[0] : ((s == hv.y) ? edN[1] : edN[2]));
    }
    if (d < 0.0005 || dHit > dstFar || p.y < 0. || rd.y > 0. && p.y > 5.) break;
  }
  if (d >= 0.0005) dHit = dstFar;
  return dHit;
}

vec3 ObjNf (vec3 p)
{
  vec4 v;
  vec2 e = vec2 (0.0001, -0.0001);
  v = vec4 (ObjDf (p + e.xxx), ObjDf (p + e.xyy), ObjDf (p + e.yxy), ObjDf (p + e.yyx));
  return normalize (vec3 (v.x - v.y - v.z - v.w) + 2. * v.yzw);
}

float ObjSShadow (vec3 ro, vec3 rd)
{
  vec3 p;
  vec2 gIdP;
  float sh, d, h;
  sh = 1.;
  d = 0.05;
  gIdP = vec2 (-99.);
  for (int j = 0; j < 30; j ++) {
    p = ro + d * rd;
    gId = PixToHex (p.xz / hgSize);
    if (gId.x != gIdP.x || gId.y != gIdP.y) {
      gIdP = gId;
      SetPngConf ();
    }
    h = ObjDf (p);
    sh = min (sh, smoothstep (0., 0.05 * d, h));
    d += clamp (h, 0.05, 0.3);
    if (sh < 0.05) break;
  }
  return 0.5 + 0.5 * sh;
}

float GrndRay (vec3 ro, vec3 rd)
{
  vec3 p;
  float dHit, d;
  dHit = -0.98 * ro.y / rd.y;
  for (int j = 0; j < 100; j ++) {
    p = ro + dHit * rd;
    d = p.y + 0.01 * step (GrndHt (p.xz), -0.01);
    if (d < 0.0005 || dHit > dstFar) break;
    dHit += d;
  }
  return dHit;
}

vec3 SkyCol (vec3 ro, vec3 rd)
{
  ro.xz += tCur;
  return mix (mix (vec3 (0.2, 0.3, 0.7), vec3 (0.9), pow (1. - max (rd.y, 0.), 8.)) +
     vec3 (1., 1., 0.9) * (0.3 * pow (max (dot (rd, sunDir), 0.), 32.)),
     vec3 (0.9), clamp (1.6 * Fbm2 (0.05 * (ro + rd * (50. - ro.y) / (rd.y + 0.0001)).xz) * rd.y +
     0.1, 0., 1.));
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  vec4 col4;
  vec3 col, vn;
  float dstObj, dstGrnd, sh;
  bool isSky, isGrnd, isWat, doSh;
  isSky = false;
  isGrnd = false;
  isWat = false;
  doSh = false;
  dstObj = ObjRay (ro, rd);
  dstGrnd = dstFar;
  if (dstObj < dstFar) {
    ro += dstObj * rd;
    if (idObj != 7) {
      gId = PixToHex (ro.xz / hgSize);
      vn = ObjNf (ro);
      if (idObj == 1 || idObj == 6) vn = VaryNf (64. * ro, vn, 0.3);
    }
    if (idObj == 1) col4 = (qHit.z < -0.2 || qHit.z < 0. && length (qHit.xy) < 0.2) ?
       vec4 (0.95, 0.95, 0.95, 0.05) : vec4 (0.1, 0.1, 0.15, 0.1);
    else if (idObj == 2) col4 = vec4 (1., 0.8, 0.2, 0.2);
    else if (idObj == 3) col4 = vec4 (0.05, 0.15, 0.05, 0.2);
    else if (idObj == 4) col4 = vec4 (0.05, 0.1, 0.05, 0.1);
    else if (idObj == 5) col4 = vec4 (0.9, 0.9, 0., 0.3);
    else if (idObj == 6) col4 = vec4 (0.15, 0.15, 0.2, 0.1);
    else if (idObj == 7) isGrnd = true;
    doSh = true;
  } else if (rd.y < 0.) {
    dstGrnd = GrndRay (ro, rd);
    ro += dstGrnd * rd;
    if (ro.y < 0.) isWat = true;
    else isGrnd = true;
  } else isSky = true;
  if (! isSky) {
    if (isWat) {
      rd = reflect (rd, VaryNf (0.5 * ro + vec3 (0.1 * tCur, 0., 0.1 * tCur), vec3 (0., 1., 0.), 0.1));
      col = mix (0.9 * SkyCol (ro, rd), ((0.1 + 0.9 * GrndHt (ro.xz) + 0.5)) *
         mix (vec3 (0.7, 0.7, 0.9), vec3 (0.8, 0.9, 0.8), Fbm2 (0.5 * ro.xz)),
         smoothstep (0.5, 0.9, rd.y));
    } else {
      if (isGrnd) {
        col4 = vec4 (vec3 (0.95, 0.95, 1.) * min (0.9 + 0.2 * Fbm2 (ro.xz), 1.), 0.1);
        vn = VaryNf (ro, vec3 (0., 1., 0.), 0.3);
        doSh = (min (dstGrnd, dstObj) < dstFar);
      }
      sh = doSh ? ObjSShadow (ro, sunDir) : 1.;
      col = col4.rgb * (0.3 + 0.2 * max (dot (normalize (vec3 (vn.x, 0., vn.z)), - sunDir), 0.) +
         0.6 * sh * max (dot (vn, sunDir), 0.)) +
         col4.a * smoothstep (0.8, 1., sh) * pow (max (dot (normalize (sunDir - rd), vn), 0.), 32.);
    }
    col = mix (col, vec3 (0.9), smoothstep (0.3, 1., min (dstGrnd, dstObj) / dstFar));
  } else col = SkyCol (ro, rd);
  return clamp (col, 0., 1.);
}

vec3 TrackPos (float t)
{
  return vec3 (5. * cos (0.01 * 2. * pi * t) + 0.1 * t, 10., t);
}

void mainImage (out vec4 fragColor, in vec2 fragCoord)
{
  mat3 vuMat;
  vec4 mPtr, dateCur;
  vec3 ro, rd, vd, col;
  vec2 canvas, uv, ori, ca, sa;
  float el, az, spd;
  canvas = iResolution.xy;
  uv = 2. * fragCoord.xy / canvas - 1.;
  uv.x *= canvas.x / canvas.y;
  tCur = iTime;
  dateCur = iDate;
  mPtr = iMouse;
  mPtr.xy = mPtr.xy / canvas - 0.5;
  tCur = mod (tCur, 2400.) + 30. * floor (dateCur.w / 7200.);
  az = 0.;
  el = 0.;
  if (mPtr.z > 0.) {
    az += 2. * pi * mPtr.x;
    el += 0.2 * pi * mPtr.y;
  }
  hgSize = 2.3;
  spd = 4.;
  ro = TrackPos (spd * tCur);
  vd = normalize (TrackPos (spd * tCur + 0.1) - ro);
  az += 0.5 * pi + atan (- vd.z, vd.x);
  el += asin (vd.y) - 0.15 * pi;
  ori = vec2 (el, az);
  ca = cos (ori);
  sa = sin (ori);
  vuMat = mat3 (ca.y, 0., - sa.y, 0., 1., 0., sa.y, 0., ca.y) *
          mat3 (1., 0., 0., 0., ca.x, - sa.x, 0., sa.x, ca.x);
  dstFar = 300.;
  sunDir = normalize (vec3 (0.5, 2., -1.));
#if ! AA
  const float naa = 1.;
#else
  const float naa = 4.;
#endif  
  col = vec3 (0.);
  for (float a = 0.; a < naa; a ++) {
    rd = vuMat * normalize (vec3 (uv + step (1.5, naa) * Rot2D (vec2 (0.71 / canvas.y, 0.),
       0.5 * pi * (a + 0.5)), 1.8));
    col += (1. / naa) * ShowScene (ro, rd);
  }
  fragColor = vec4 (pow (col, vec3 (0.8)), 1.);
}

float PrSphDf (vec3 p, float r)
{
  return length (p) - r;
}

float PrCylDf (vec3 p, float r, float h)
{
  return max (length (p.xy) - r, abs (p.z) - h);
}

float PrEllipsDf (vec3 p, vec3 r)
{
  return (length (p / r) - 1.) * min (r.x, min (r.y, r.z));
}

float SmoothMin (float a, float b, float r)
{
  float h;
  h = clamp (0.5 + 0.5 * (b - a) / r, 0., 1.);
  return mix (b, a, h) - r * h * (1. - h);
}

float SmoothBump (float lo, float hi, float w, float x)
{
  return (1. - smoothstep (hi - w, hi + w, x)) * smoothstep (lo - w, lo + w, x);
}

vec2 PixToHex (vec2 p)
{
  vec3 c, r, dr;
  c.xz = vec2 ((1./sqrt3) * p.x - (1./3.) * p.y, (2./3.) * p.y);
  c.y = - c.x - c.z;
  r = floor (c + 0.5);
  dr = abs (r - c);
  r -= step (dr.yzx, dr) * step (dr.zxy, dr) * dot (r, vec3 (1.));
  return r.xz;
}

vec2 HexToPix (vec2 h)
{
  return vec2 (sqrt3 * (h.x + 0.5 * h.y), (3./2.) * h.y);
}

vec2 Rot2D (vec2 q, float a)
{
  vec2 cs;
  cs = sin (a + vec2 (0.5 * pi, 0.));
  return vec2 (dot (q, vec2 (cs.x, - cs.y)), dot (q.yx, cs));
}

const float cHashM = 43758.54;

float Hashfv2 (vec2 p)
{
  return fract (sin (dot (p, vec2 (37., 39.))) * cHashM);
}

vec2 Hashv2v2 (vec2 p)
{
  vec2 cHashVA2 = vec2 (37., 39.);
  return fract (sin (vec2 (dot (p, cHashVA2), dot (p + vec2 (1., 0.), cHashVA2))) * cHashM);
}

float Noisefv2 (vec2 p)
{
  vec2 t, ip, fp;
  ip = floor (p);  
  fp = fract (p);
  fp = fp * fp * (3. - 2. * fp);
  t = mix (Hashv2v2 (ip), Hashv2v2 (ip + vec2 (0., 1.)), fp.y);
  return mix (t.x, t.y, fp.x);
}

float Fbm2 (vec2 p)
{
  float f, a;
  f = 0.;
  a = 1.;
  for (int j = 0; j < 5; j ++) {
    f += a * Noisefv2 (p);
    a *= 0.5;
    p *= 2.;
  }
  return f * (1. / 1.9375);
}

float Fbmn (vec3 p, vec3 n)
{
  vec3 s;
  float a;
  s = vec3 (0.);
  a = 1.;
  for (int j = 0; j < 5; j ++) {
    s += a * vec3 (Noisefv2 (p.yz), Noisefv2 (p.zx), Noisefv2 (p.xy));
    a *= 0.5;
    p *= 2.;
  }
  return dot (s, abs (n));
}

vec3 VaryNf (vec3 p, vec3 n, float f)
{
  vec3 g;
  vec2 e = vec2 (0.1, 0.);
  g = vec3 (Fbmn (p + e.xyy, n), Fbmn (p + e.yxy, n), Fbmn (p + e.yyx, n)) - Fbmn (p, n);
  return normalize (n + f * (g - n * dot (n, g)));
}

