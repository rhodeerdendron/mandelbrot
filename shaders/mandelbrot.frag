#version 330 core

precision highp float;


in vec2 f_st;

uniform vec2 center = vec2(0.0, 0.0);
uniform float zoom = 0.4;
uniform float aspect = 1.0;


uniform float expon = 2.0;
uniform float thresh = 2.0;

uniform uint max_steps = 1024u;


// complex number operations

vec2 compl_as_polar(vec2 c)
{
    float r = length(c);
    float theta = acos(c.x / r);
    if (c.y < 0.0) theta = -theta;
    return vec2( r, theta );
}

vec2 polar_as_compl(vec2 p)
{
    float real = p.x * cos(p.y);
    float imag = p.x * sin(p.y);
    return vec2( real, imag );
}

vec2 compl_pow(vec2 c, float e)
{
    vec2 p = compl_as_polar(c);
    vec2 powed = vec2( pow(p.x, e), e*p.y ); // { pow(p.r, e), e*pow.theta }
    return polar_as_compl(powed);
}


const vec3 palette[16] = vec3[16](
    vec3( 66,  30,  15), // brown 3
    vec3( 25,   7,  26), // dark violett
    vec3(  9,   1,  47), // darkest blue
    vec3(  4,   4,  73), // blue 5
    vec3(  0,   7, 100), // blue 4
    vec3( 12,  44, 138), // blue 3
    vec3( 24,  82, 177), // blue 2
    vec3( 57, 125, 209), // blue 1
    vec3(134, 181, 229), // blue 0
    vec3(211, 236, 248), // lightest blue
    vec3(241, 233, 191), // lightest yellow
    vec3(248, 201,  95), // light yellow
    vec3(255, 170,   0), // dirty yellow
    vec3(204, 128,   0), // brown 0
    vec3(153,  87,   0), // brown 1
    vec3(106,  52,   3)  // brown 2
);


vec4 color_for_depth(uint i)
{
    const uint N = uint(palette.length());
    i = i % N;
    return vec4(palette[i] / 255.0, 1.0);
}


void main()
{
    // apply center translation, aspect, and zoom
    vec2 aspect_mul = vec2(aspect, 1.0);
    vec2 st = aspect_mul * f_st / zoom + center;

    // iterate
    uint i = 0u;
    vec2 z = st;
    float sqthresh = thresh * thresh;
    while (dot(z, z) < sqthresh && i < max_steps)
    {
        // // burning ship
        // vec2 z_abs = vec2(abs(z.x), abs(z.y));
        // z = compl_pow(z_abs, expon) + st;

        // mandelbrot set
        z = compl_pow(z, expon) + st;

        i++;
    }

    // color based on i
    gl_FragColor = color_for_depth(i);
}

