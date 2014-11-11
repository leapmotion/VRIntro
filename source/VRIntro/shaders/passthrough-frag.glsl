#version 120

//ray. This is specialized for overlays
varying vec2 frag_ray;

// original texture
uniform sampler2D texture;

// distortion maps
uniform sampler2D distortion;

// controls
uniform float gamma;
uniform float brightness;
uniform float use_color;
uniform float ir_mode;
uniform float cripple_mode;
uniform float stencil_mode;

// debayer
float width = 672.0;
float height = 600.0;
float rscale = 1.5;
float gscale = 1.0;
float bscale = 0.5;
float irscale = 1.2;

float corr_ir_g = 0.2;
float corr_ir_rb = 0.2;
float corr_r_b = 0.1;
float corr_g_rb = 0.1;

vec2 r_offset = vec2(0, -0.5);
vec2 g_offset = vec2(0.5, -0.5);
vec2 b_offset = vec2(0.5, 0);

void main(void) {
  vec2 texCoord = texture2D(distortion, frag_ray).xy;

  // 60 degree FOV 16:9 aspect ratio single camera setup         
  if (cripple_mode > 0.5 && (frag_ray.x < 0.4335 || frag_ray.x > 0.5665 || frag_ray.y < 0.4508 || frag_ray.y > 0.5492)) {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    return;
  }

  if (use_color > 0.5) {
    // Unwarp the point. Correct oculus distortion, if applicable
   
    float dx = 1.0/width;
    float dy = 1.0/height;
   
    vec2 redOffset = vec2(dx, dy)*r_offset;
    vec2 greenOffset = vec2(dx, dy)*g_offset;
    vec2 blueOffset = vec2(dx, dy)*b_offset;
   
    float ir_lf = texture2D(texture, texCoord).r;
    float ir_l = texture2D(texture, texCoord + vec2(-dx, 0)).r;
    float ir_t = texture2D(texture, texCoord + vec2(0, -dy)).r;
    float ir_r = texture2D(texture, texCoord + vec2(dx, 0)).r;
    float ir_b = texture2D(texture, texCoord + vec2(0, dy)).r;
    float ir_hf = ir_lf - 0.25*(ir_l + ir_t + ir_r + ir_b);
   
    float r_lf = texture2D(texture, texCoord + redOffset).b;
    float r_l = texture2D(texture, texCoord + redOffset + vec2(-dx, 0)).b;
    float r_t = texture2D(texture, texCoord + redOffset + vec2(0, -dy)).b;
    float r_r = texture2D(texture, texCoord + redOffset + vec2(dx, 0)).b;
    float r_b = texture2D(texture, texCoord + redOffset + vec2(0, dy)).b;
    float r_hf = r_lf - 0.25*(r_l + r_t + r_r + r_b);
   
    float g_lf = texture2D(texture, texCoord + greenOffset).a;
    float g_l = texture2D(texture, texCoord + greenOffset + vec2(-dx, 0)).a;
    float g_t = texture2D(texture, texCoord + greenOffset + vec2(0, -dy)).a;
    float g_r = texture2D(texture, texCoord + greenOffset + vec2(dx, 0)).a;
    float g_b = texture2D(texture, texCoord + greenOffset + vec2(0, dy)).a;
    float g_hf = g_lf - 0.25*(g_l + g_t + g_r + g_b);
   
    float b_lf = texture2D(texture, texCoord + blueOffset).g;
    float b_l = texture2D(texture, texCoord + blueOffset + vec2(-dx, 0)).g;
    float b_t = texture2D(texture, texCoord + blueOffset + vec2(0, -dy)).g;
    float b_r = texture2D(texture, texCoord + blueOffset + vec2(dx, 0)).g;
    float b_b = texture2D(texture, texCoord + blueOffset + vec2(0, dy)).g;
    float b_hf = b_lf - 0.25*(b_l + b_t + b_r + b_b);
   
    const mat4 transformation = mat4(5.6220, -1.5456, 0.3634, -0.1106, -1.6410, 3.1944, -1.7204, 0.0189, 0.1410, 0.4896, 10.8399, -0.1053, -3.7440, -1.9080, -8.6066, 1.0000);
    const mat4 conservative = mat4(5.6220, 0.0000, 0.3634, 0.0000, 0.0000, 3.1944, 0.0000, 0.0189, 0.1410, 0.4896, 10.8399, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000);

    const mat4 transformation_filtered = mat4(5.0670, -1.2312, 0.8625, -0.0507, -1.5210, 3.1104, -2.0194, 0.0017, -0.8310, -0.3000, 13.1744, -0.1052, -2.4540, -1.3848, -10.9618, 1.0000);
    const mat4 conservative_filtered = mat4(5.0670, 0.0000, 0.8625, 0.0000, 0.0000, 3.1104, 0.0000, 0.0017, 0.0000, 0.0000, 13.1744, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000);

    vec4 input_lf = vec4(r_lf, g_lf, b_lf, ir_lf);
   
    // input_lf = bilateral_a*bilateral(texCoord, input_lf) + (1-bilateral_a)*input_lf;
    input_lf.r += ir_hf*corr_ir_rb + g_hf*corr_g_rb + b_hf*corr_r_b;
    input_lf.g += ir_hf*corr_ir_g + r_hf*corr_g_rb + b_hf*corr_g_rb;
    input_lf.b += ir_hf*corr_ir_rb + r_hf*corr_r_b + g_hf*corr_g_rb;
   
    vec4 output_lf = transformation_filtered*input_lf;
    vec4 output_lf_fudge = conservative_filtered*input_lf;
    //vec4 output_lf_gray = gray*input_lf;
   
    float fudge_threshold = 0.5;
    float ir_fudge_threshold = 0.95;
    float ir_fudge_factor = 0.333*(r_lf + g_lf + b_lf);
   
    float rfudge = r_lf > fudge_threshold ? (r_lf - fudge_threshold)/(1.0 - fudge_threshold) : 0;
    float gfudge = g_lf > fudge_threshold ? (g_lf - fudge_threshold)/(1.0 - fudge_threshold) : 0;
    float bfudge = b_lf > fudge_threshold ? (b_lf - fudge_threshold)/(1.0 - fudge_threshold) : 0;
    float irfudge = ir_fudge_factor > ir_fudge_threshold ? (ir_fudge_factor - ir_fudge_threshold)/(1.0 - ir_fudge_threshold) : 0;
    rfudge *= rfudge;
    gfudge *= gfudge;
    bfudge *= bfudge;
    irfudge *= irfudge;
   
    gl_FragColor.r = rfudge*output_lf_fudge.r + (1-rfudge)*output_lf.r;
    gl_FragColor.g = gfudge*output_lf_fudge.g + (1-gfudge)*output_lf.g;
    gl_FragColor.b = bfudge*output_lf_fudge.b + (1-bfudge)*output_lf.b;
    float ir_out = irfudge*output_lf_fudge.a + (1-irfudge)*output_lf.a;
   
    gl_FragColor.r *= rscale;
    gl_FragColor.g *= gscale;
    gl_FragColor.b *= bscale;
    ir_out *= irscale;
   
    //float avgrgb = 0.33333*(input_lf.r + input_lf.g + input_lf.b) - 0.9*input_lf.a;
    //float threshold = min(1, avgrgb*100);
    //threshold *= threshold;
    //gl_FragColor.rgb = output_lf_gray.rgb*(1 - threshold) + gl_FragColor.rgb*(threshold);
    
    gl_FragColor.rgb = ir_mode > 0.5 ? vec3(ir_out) : gl_FragColor.rgb;
    gl_FragColor.rgb = 1.05*brightness*pow(gl_FragColor.rgb, vec3(gamma));
    vec3 dist = normalize(output_lf.rgb) - vec3(0.6968, 0.4856, 0.5279);
    gl_FragColor.a = stencil_mode > 0.5 ? ir_out*6.0 + 0.08/(1 + 25*dot(dist, dist)) : 1.0;
    
  } else {
    gl_FragColor.rgb = brightness*vec3(pow(texture2D(texture, texCoord).r, gamma));
    gl_FragColor.a = stencil_mode > 0.5 ? gl_FragColor.r : 1.0;
  }
}
