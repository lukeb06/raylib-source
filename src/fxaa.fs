#version 330

// Input vertex attributes (from raylib vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec2 renderResolution; // Pass your screen resolution here!

// Output fragment color
out vec4 finalColor;

void main()
{
    // The inverse of the resolution determines the size of one pixel step
    vec2 inverseResolution = vec2(1.0 / renderResolution.x, 1.0 / renderResolution.y);
    
    // Simple FXAA approximation for performance and low-overhead
    vec4 rgbNW = texture(texture0, fragTexCoord + (vec2(-1.0, -1.0) * inverseResolution));
    vec4 rgbNE = texture(texture0, fragTexCoord + (vec2(1.0, -1.0) * inverseResolution));
    vec4 rgbSW = texture(texture0, fragTexCoord + (vec2(-1.0, 1.0) * inverseResolution));
    vec4 rgbSE = texture(texture0, fragTexCoord + (vec2(1.0, 1.0) * inverseResolution));
    vec4 rgbM  = texture(texture0, fragTexCoord);
    
    // Convert to luma (brightness) to find high-contrast edges
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW.rgb, luma);
    float lumaNE = dot(rgbNE.rgb, luma);
    float lumaSW = dot(rgbSW.rgb, luma);
    float lumaSE = dot(rgbSE.rgb, luma);
    float lumaM  = dot(rgbM.rgb,  luma);
    
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    // Choose a blur direction based on local contrast gradient
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * 0.125), 0.0078);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    
    dir = min(vec2(8.0, 8.0), max(vec2(-8.0, -8.0), dir * rcpDirMin)) * inverseResolution;
    
    // Sample along the edge gradient and blend
    vec4 rgbA = 0.5 * (
        texture(texture0, fragTexCoord + dir * (1.0 / 3.0 - 0.5)) +
        texture(texture0, fragTexCoord + dir * (2.0 / 3.0 - 0.5)));
    vec4 rgbB = rgbA * 0.5 + 0.25 * (
        texture(texture0, fragTexCoord + dir * -0.5) +
        texture(texture0, fragTexCoord + dir * 0.5));
        
    float lumaB = dot(rgbB.rgb, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        finalColor = rgbA * fragColor;
    } else {
        finalColor = rgbB * fragColor;
    }
}
