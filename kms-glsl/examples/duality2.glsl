// Copied from https://www.shadertoy.com/view/fd2XRK
// Created by https://www.shadertoy.com/user/morisil

// Fork of "duality for twitch.tv/sableraph" by morisil. https://shadertoy.com/view/7dBXzV
// 2021-05-09 17:16:25

// the same shader, just at bit different parameters for more color spread

// check out https://www.twitch.tv/sableraph
// sableraph is posting creative coding chanllange every week and then review
// sketches on the stream every Sunday. This time the topic was "duality"

// Some comments:

// I like that using trigonometric shapes is reducing the need of antialiasing whatsover

// I started with grayscale, but then decided to apply distance based and angular
// based chromatic abberration, which gives more video synthesis-like aesthetics
// Maybe my childhood CRT experience makes me love vaporwave aesthetics so much :)


#define SCALE                  140.0
#define ROTATION_SPEED         -1.
#define DISTANCE_SPREAD        -.01
#define ANGLE_SPREAD           .5
#define SHAPE_RANGE            1.6
#define OSCILLATION_SPEED      .15
#define OSCILLATION_AMPLITUDE  .2
#define MIRROR_TRUE            1.
#define MIRROR_FALSE           -1.


float getColorComponent(float dist, float angle, float mirror) {
    return clamp(
        sin(
            (dist * SCALE)
                + angle * mirror
                + (cos(dist * SCALE))
                - (iTime * ROTATION_SPEED) * mirror
        )
        - dist * SHAPE_RANGE
        ,-0.05 // try putting small negative value here, like -.2 :)
        ,1.
    );
}

vec3 getSwirl(vec2 center, float dist, float mirror) {
    float angle = atan(center.x, center.y);
    return vec3(
        getColorComponent(dist * (1. - DISTANCE_SPREAD), angle - ANGLE_SPREAD, mirror),
        getColorComponent(dist * (1. + 0.)             , angle - 0.          , mirror),
        getColorComponent(dist * (1. + DISTANCE_SPREAD), angle + ANGLE_SPREAD, mirror)
    );    
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (fragCoord -.5 * iResolution.xy) / iResolution.y;
    vec2 oscillation = vec2(
        sin(iTime * OSCILLATION_SPEED),
        0.
    ) * OSCILLATION_AMPLITUDE;
    vec2 uv1 = uv + oscillation;
    vec2 uv2 = uv - oscillation;
    float dist1 = length(uv1);
    float dist2 = length(uv2);
    vec3 color =
        getSwirl(uv1, dist1, MIRROR_TRUE)
        + getSwirl(uv2, dist2, MIRROR_FALSE);
    fragColor = vec4(color, 1.);
}
