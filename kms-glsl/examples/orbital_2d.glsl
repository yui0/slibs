// Copied from https://www.shadertoy.com/view/sdsXWs
// Created by https://www.shadertoy.com/user/milesWaugh

/*Disclaimer: while the program smoothly transitions between two 
states who's wave function's frequency is an integer multiple 
times the electrons' frequency, there would be no smooth transition 
in real life. In real life, the states would instantaneously switch.
The program switches smoothly for aesthetic purposes. 

The blue --> orange gradient is where the wavefunction is positive,
the purple --> green gradient is where the wavefunction is negative.
The blue and purple colors represent low probability of the 
wavefunction collapsing to that specific position apon observation.
The orange and green colors represent a high probability (of what
I just mentioned, no need to repeat).
*/

float pi = 3.141592653589793;
float sq2 = sqrt(2.);
float transitionSpeed = 60.; //Higher is faster

float sw(float x, float s){
    return(atan(s*sin(pi*x)))/(.5*pi);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord){
    vec3 col = vec3(0.);
    float contour = 0.1;
    int AA = 4; //Default value is 4x4 anti-aliasing
    float AAA = float(AA*AA);
    for(int i=0; i<AA; i++){
    for(int j=0; j<AA; j++){
        vec2 uv = (fragCoord+vec2(i,j)/float(AA)-iResolution.xy/2.)/iResolution.y;
        uv*=7.;
        float a = atan(uv.y/uv.x)-(uv.x<0.?pi:0.);
        a+=a<0.?2.*pi:0.;
        float d = sqrt(dot(uv,uv));
        float mu = sw(iTime/4.,transitionSpeed);
        mu = mu/2.+.5;
        float rot =  iTime/5.;
        rot *= mu/10.+1.;
        float w1 = cos(2.*a+rot)*exp(-d)*d*2.5;
        float w2 = cos(a+rot)*exp(-d)*d*2.5;
        float w = mu*w1+w2*(1.-mu);
        float p = w*w;
        p = max(min(p,1.),0.);
        if(w<0.){
            col += vec3(p,(-cos(pi*(p))/2.+.5),(1.-(acos(2.*p-1.))/pi))/AAA;
        }else{
            col += vec3((-cos(pi*(p))/2.+.5),p,(1.-(acos(2.*p-1.))/pi))/AAA;
        }
        float er = sin(pi*(p*10.-floor(p*10.)));
        er-=p<1./20.?5.:0.;
        er = (0.01)/(er*er);
        col += vec3(min(er/AAA,1./AAA), min(0.7*er/AAA,1./AAA), min(0.6*er/AAA,1./AAA))/2.;
    }}
    fragColor = vec4(col,1.0);
}
