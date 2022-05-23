// Source edited by David Hoskins - 2013.

// I took and completed this http://glsl.heroku.com/e#9743.20 - just for fun! 8|
// Locations in 3x7 font grid, inspired by http://www.claudiocc.com/the-1k-notebook-part-i/
// Had to edit it to remove some duplicate lines.
// ABC  a:GIOMJL b:AMOIG c:IGMO d:COMGI e:OMGILJ f:CBN g:OMGIUS h:AMGIO i:EEHN j:GHTS k:AMIKO l:BN m:MGHNHIO n:MGIO
// DEF  o:GIOMG p:SGIOM q:UIGMO r:MGI s:IGJLOM t:BNO u:GMOI v:GJNLI w:GMNHNOI x:GOKMI y:GMOIUS z:GIMO
// GHI
// JKL 
// MNO
// PQR
// STU

vec2 coord;

#define font_size 20. 
#define font_spacing .05
#define STROKEWIDTH 0.05
#define PI 3.14159265359

#define A_ vec2(0.,0.)
#define B_ vec2(1.,0.)
#define C_ vec2(2.,0.)

//#define D_ vec2(0.,1.)
#define E_ vec2(1.,1.)
//#define F_ vec2(2.,1.)

#define G_ vec2(0.,2.)
#define H_ vec2(1.,2.)
#define I_ vec2(2.,2.)

#define J_ vec2(0.,3.)
#define K_ vec2(1.,3.)
#define L_ vec2(2.,3.)

#define M_ vec2(0.,4.)
#define N_ vec2(1.,4.)
#define O_ vec2(2.,4.)

//#define P_ vec2(0.,5.)
//#define Q_ vec2(1.,5.)
//#define R_ vec2(1.,5.)

#define S_ vec2(0.,6.)
#define T_ vec2(1.,6.)
#define U_ vec2(2.0,6.)

#define A(p) t(G_,I_,p) + t(I_,O_,p) + t(O_,M_, p) + t(M_,J_,p) + t(J_,L_,p);caret.x += 1.0;
#define B(p) t(A_,M_,p) + t(M_,O_,p) + t(O_,I_, p) + t(I_,G_,p);caret.x += 1.0;
#define C(p) t(I_,G_,p) + t(G_,M_,p) + t(M_,O_,p);caret.x += 1.0;
#define D(p) t(C_,O_,p) + t(O_,M_,p) + t(M_,G_,p) + t(G_,I_,p);caret.x += 1.0;
#define E(p) t(O_,M_,p) + t(M_,G_,p) + t(G_,I_,p) + t(I_,L_,p) + t(L_,J_,p);caret.x += 1.0;
#define F(p) t(C_,B_,p) + t(B_,N_,p) + t(G_,I_,p);caret.x += 1.0;
#define G(p) t(O_,M_,p) + t(M_,G_,p) + t(G_,I_,p) + t(I_,U_,p) + t(U_,S_,p);caret.x += 1.0;
#define H(p) t(A_,M_,p) + t(G_,I_,p) + t(I_,O_,p);caret.x += 1.0;
#define I(p) t(E_,E_,p) + t(H_,N_,p);caret.x += 1.0;
#define J(p) t(E_,E_,p) + t(H_,T_,p) + t(T_,S_,p);caret.x += 1.0;
#define K(p) t(A_,M_,p) + t(M_,I_,p) + t(K_,O_,p);caret.x += 1.0;
#define L(p) t(B_,N_,p);caret.x += 1.0;
#define M(p) t(M_,G_,p) + t(G_,I_,p) + t(H_,N_,p) + t(I_,O_,p);caret.x += 1.0;
#define N(p) t(M_,G_,p) + t(G_,I_,p) + t(I_,O_,p);caret.x += 1.0;
#define O(p) t(G_,I_,p) + t(I_,O_,p) + t(O_,M_, p) + t(M_,G_,p);caret.x += 1.0;
#define P(p) t(S_,G_,p) + t(G_,I_,p) + t(I_,O_,p) + t(O_,M_, p);caret.x += 1.0;
#define Q(p) t(U_,I_,p) + t(I_,G_,p) + t(G_,M_,p) + t(M_,O_, p);caret.x += 1.0;
#define R(p) t(M_,G_,p) + t(G_,I_,p);caret.x += 1.0;
#define S(p) t(I_,G_,p) + t(G_,J_,p) + t(J_,L_,p) + t(L_,O_,p) + t(O_,M_,p);caret.x += 1.0;
#define T(p) t(B_,N_,p) + t(N_,O_,p) + t(G_,I_,p);caret.x += 1.0;
#define U(p) t(G_,M_,p) + t(M_,O_,p) + t(O_,I_,p);caret.x += 1.0;
#define V(p) t(G_,J_,p) + t(J_,N_,p) + t(N_,L_,p) + t(L_,I_,p);caret.x += 1.0;
#define W(p) t(G_,M_,p) + t(M_,O_,p) + t(N_,H_,p) + t(O_,I_,p);caret.x += 1.0;
#define X(p) t(G_,O_,p) + t(I_,M_,p);caret.x += 1.0;
#define Y(p) t(G_,M_,p) + t(M_,O_,p) + t(I_,U_,p) + t(U_,S_,p);caret.x += 1.0;
#define Z(p) t(G_,I_,p) + t(I_,M_,p) + t(M_,O_,p);caret.x += 1.0;
#define STOP(p) t(N_,N_,p);caret.x += 1.0;
	
vec2 caret_origin = vec2(3.0, .7);
vec2 caret;

//-----------------------------------------------------------------------------------
float minimum_distance(vec2 v, vec2 w, vec2 p)
{	// Return minimum distance between line segment vw and point p
  	float l2 = (v.x - w.x)*(v.x - w.x) + (v.y - w.y)*(v.y - w.y); //length_squared(v, w);  // i.e. |w-v|^2 -  avoid a sqrt
  	if (l2 == 0.0) {
		return distance(p, v);   // v == w case
	}
	
	// Consider the line extending the segment, parameterized as v + t (w - v).
  	// We find projection of point p onto the line.  It falls where t = [(p-v) . (w-v)] / |w-v|^2
  	float t = dot(p - v, w - v) / l2;
  	if(t < 0.0) {
		// Beyond the 'v' end of the segment
		return distance(p, v);
	} else if (t > 1.0) {
		return distance(p, w);  // Beyond the 'w' end of the segment
	}
  	vec2 projection = v + t * (w - v);  // Projection falls on the segment
	return distance(p, projection);
}

//-----------------------------------------------------------------------------------
float textColor(vec2 from, vec2 to, vec2 p)
{
	p *= font_size;
	float inkNess = 0., nearLine, corner;
	nearLine = minimum_distance(from,to,p); // basic distance from segment, thanks http://glsl.heroku.com/e#6140.0
	inkNess += smoothstep(0., 1., 1.- 14.*(nearLine - STROKEWIDTH)); // ugly still
	inkNess += smoothstep(0., 2.5, 1.- (nearLine  + 5. * STROKEWIDTH)); // glow
	return inkNess;
}

//-----------------------------------------------------------------------------------
vec2 grid(vec2 letterspace) 
{
	return ( vec2( (letterspace.x / 2.) * .65 , 1.0-((letterspace.y / 2.) * .95) ));
}

//-----------------------------------------------------------------------------------
float count = 0.0;
float gtime;
float t(vec2 from, vec2 to, vec2 p) 
{
	count++;
	if (count > gtime*20.0) return 0.0;
	return textColor(grid(from), grid(to), p);
}

//-----------------------------------------------------------------------------------
vec2 r()
{
	vec2 pos = coord.xy/iResolution.xy;
	pos.y -= caret.y;
	pos.x -= font_spacing*caret.x;
	return pos;
}

//-----------------------------------------------------------------------------------
void _()
{
	caret.x += 1.5;
}

//-----------------------------------------------------------------------------------
void newline()
{
	caret.x = caret_origin.x;
	caret.y -= .18;
}

//-----------------------------------------------------------------------------------
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float time = mod(iTime, 11.0);
    gtime = time;

	float d = 0.;
	vec3 col = vec3(0.1, .07+0.07*(.5+sin(fragCoord.y*3.14159*1.1+time*2.0)) + sin(fragCoord.y*.01+time+2.5)*0.05, 0.1);
    
    coord = fragCoord;
	
	caret = caret_origin;

	// the quick brown
	d += T(r()); d += H(r()); d += E(r()); _();
	d += Q(r()); d += U(r()); d += I(r()); d += C(r()); d += K(r()); _();
	d += B(r()); d += R(r()); d += O(r()); d += W(r()); d += N(r()); _();
	newline();
    // fox jumps over 
	d += F(r()); d += O(r()); d += X(r()); _();
	d += J(r()); d += U(r()); d += M(r()); d += P(r()); d += S(r()); _();
	d += O(r()); d += V(r()); d += E(r()); d += R(r()); _();
	newline();
    // the lazy dog...
	d += A(r()); _();
	d += L(r()); d += A(r()); d += Z(r()); d += Y(r()); _();
	d += D(r()); d += O(r()); d += G(r()); d += STOP(r()); d += STOP(r()); d += STOP(r());
	d = clamp(d* (.75+sin(fragCoord.x*PI*.5-time*4.3)*.5), 0.0, 1.0);
      
    col += vec3(d*.5, d, d*.85);
	vec2 xy = fragCoord.xy / iResolution.xy;
	col *= vec3(.4, .4, .3) + 0.5*pow(100.0*xy.x*xy.y*(1.0-xy.x)*(1.0-xy.y), .4 );	
    fragColor = vec4( col, 1.0 );
}

