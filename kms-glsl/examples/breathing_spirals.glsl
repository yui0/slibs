// Copied from https://www.shadertoy.com/view/NsBSRt
// Created by https://www.shadertoy.com/user/ykcwa

// from https://www.shadertoy.com/view/Mls3R7
void rotate(inout vec2 p,float angle,vec2 rotationOrigin)
{
    p -= rotationOrigin;
    p *= mat2(cos(angle),-sin(angle),sin(angle),cos(angle));
    p += rotationOrigin;
}


float stripe(float x, float stripeWidth) {
  float s = mod(x + 0.5 * stripeWidth, 2.0 * stripeWidth);
  if (s > stripeWidth) {
    s = 2.0 * stripeWidth - s;
  }
  float smoothness =  4./iResolution.y;
  return smoothstep(stripeWidth * 0.5, stripeWidth * 0.5 - smoothness, s);
}

float xorBW(float color1, float color2) { 
  return abs(color1 - color2);
}

float spiralColor(vec2 uv, int numStripes, float curveRate) {
  float r = length(uv);
  float angle = atan(uv.x, uv.y) + curveRate * r;
  return stripe(angle, 2.0 * 3.14159 / float(numStripes));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (y from -0.5 to 0.5, x scaled appropriately)
    vec2 uv = ( fragCoord - .5* iResolution.xy) / iResolution.y;

    int numStripes = 16;
    float curveRate = 2.0;
    float rotAngle = cos(iTime / 3.0);
    
    vec2 uv1 = vec2(uv.x, uv.y);
    rotate(uv1, 0.0 * rotAngle, vec2(0));

    vec2 uv2 = vec2(-1.0 * uv.x, uv.y);
    rotate(uv2, 3.0 * rotAngle, vec2(0));

    float spiral1 = spiralColor(uv1, numStripes, curveRate);
    float spiral2 = spiralColor(uv2, numStripes, curveRate);


    fragColor = vec4(vec3(xorBW(spiral1, spiral2)), 1.0);
}
