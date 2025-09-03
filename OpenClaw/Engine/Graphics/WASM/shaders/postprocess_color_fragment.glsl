#version 300 es
precision mediump float;

uniform sampler2D u_texture;
uniform float u_brightness;
uniform float u_contrast;
uniform float u_saturation;

in vec2 v_texCoord;
out vec4 fragColor;

void main() {
    vec4 color = texture(u_texture, v_texCoord);
    
    // Apply brightness
    color.rgb *= u_brightness;
    
    // Apply contrast
    color.rgb = (color.rgb - 0.5) * u_contrast + 0.5;
    
    // Apply saturation
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    color.rgb = mix(vec3(gray), color.rgb, u_saturation);
    
    // Clamp to valid range
    color.rgb = clamp(color.rgb, 0.0, 1.0);
    
    fragColor = color;
}
