// Copied from https://www.shadertoy.com/view/3lGyRy
// Created by https://www.shadertoy.com/user/Hatchling

#define F1 float
#define F3 vec3

F1 Linear1(F1 c){return(c<=0.04045)?c/12.92:pow((c+0.055)/1.055,2.4);}
F3 Linear3(F3 c){return F3(Linear1(c.r),Linear1(c.g),Linear1(c.b));}
F1 Srgb1(F1 c){return(c<0.0031308?c*12.92:1.055*pow(c,0.41666)-0.055);}
F3 Srgb3(F3 c){return F3(Srgb1(c.r),Srgb1(c.g),Srgb1(c.b));}

vec3 rgb_to_oklab(vec3 c) 
{
    float l = 0.4121656120f * c.r + 0.5362752080f * c.g + 0.0514575653f * c.b;
    float m = 0.2118591070f * c.r + 0.6807189584f * c.g + 0.1074065790f * c.b;
    float s = 0.0883097947f * c.r + 0.2818474174f * c.g + 0.6302613616f * c.b;

    float l_ = pow(l, 1./3.);
    float m_ = pow(m, 1./3.);
    float s_ = pow(s, 1./3.);

    vec3 labResult;
    labResult.x = 0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_;
    labResult.y = 1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_;
    labResult.z = 0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_;
    return labResult;
}

vec3 oklab_to_rgb(vec3 c) 
{
    float l_ = c.x + 0.3963377774f * c.y + 0.2158037573f * c.z;
    float m_ = c.x - 0.1055613458f * c.y - 0.0638541728f * c.z;
    float s_ = c.x - 0.0894841775f * c.y - 1.2914855480f * c.z;

    float l = l_*l_*l_;
    float m = m_*m_*m_;
    float s = s_*s_*s_;

    vec3 rgbResult;
    rgbResult.r = + 4.0767245293f*l - 3.3072168827f*m + 0.2307590544f*s;
    rgbResult.g = - 1.2681437731f*l + 2.6093323231f*m - 0.3411344290f*s;
    rgbResult.b = - 0.0041119885f*l - 0.7034763098f*m + 1.7068625689f*s;
    return rgbResult;
}

vec3 tonemap( vec3 linearRGB )
{
    const float limitHardness = 1.5;
    
    vec3 okl = rgb_to_oklab(linearRGB);
    
    // Limit luminance.
    /*okl.x = okl.x / pow(pow(okl.x, limitHardness) + 1., 1./limitHardness);
    
    // Limit magnitude of chrominance.
    {
        float mag = length(okl.yz);
        float magAfter = mag;
        magAfter *= 4.;
        magAfter = magAfter / pow(pow(magAfter, limitHardness) + 1., 1./limitHardness);
        magAfter /= 4.;
        okl.yz *= magAfter/mag;
    }*/
    
    linearRGB = oklab_to_rgb(okl);

    // Try to keep the resulting value within the RGB gamut while
    // preserving chrominance and compensating for negative clipping.
    {
        {
            // Compensate for negative clipping.
            float lumBefore = dot(linearRGB, vec3(0.2126, 0.7152, 0.0722));
            linearRGB = max(vec3(0), linearRGB);
            float lumAfter = dot(linearRGB, vec3(0.2126, 0.7152, 0.0722));
            linearRGB *= lumBefore/lumAfter;
            
            // Keep the resulting value within the RGB gamut.
            linearRGB = linearRGB / pow(pow(linearRGB, vec3(limitHardness)) + vec3(1), vec3(1./limitHardness));
        }
        
        for(int i = 0; i < 2; i++)
        {
            vec3 okl2 = rgb_to_oklab(linearRGB);

            // Control level of L preservation.
            okl2.x = mix(okl2.x, okl.x, 1.0);
             
            float magBefore = length(okl2.yz);
            // Control level of ab preservation.
            okl2.yz = mix(okl2.yz, okl.yz, 0.5);
            float magAfter = length(okl2.yz);
            
            // Uncomment this to only preserve hue.
            // okl2.yz *= magBefore/magAfter;

            linearRGB = oklab_to_rgb(okl2);  
  
            {
                // Compensate for negative clipping.
                float lumBefore = dot(linearRGB, vec3(0.2126, 0.7152, 0.0722));
                linearRGB = max(vec3(0), linearRGB);
                float lumAfter = dot(linearRGB, vec3(0.2126, 0.7152, 0.0722));
                linearRGB *= lumBefore/lumAfter;
                
                // Keep the resulting value within the RGB gamut.
                linearRGB = linearRGB / pow(pow(linearRGB, vec3(limitHardness)) + vec3(1), vec3(1./limitHardness));
            }
        }
    }
        
  
    return Srgb3(linearRGB);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 position = (fragCoord/iResolution.xy)* 2.0 - 1.0;
    position.x += iTime * 0.2;

	vec3 color = pow(sin(position.x * 4.0 + vec3(0.0, 1.0, 2.0) * 3.1415 * 2.0 / 3.0) * 0.5 + 0.5, vec3(2.0)) * (exp(abs(position.y) * 4.0) - 1.0);;

	if(position.y < 0.0)
    {
		color = tonemap(color);   
	}
    else
    {
        color = Srgb3(color);
    }

	fragColor = vec4(color,1.0);
}

