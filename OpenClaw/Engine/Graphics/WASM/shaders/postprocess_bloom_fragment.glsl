#version 300 es
precision mediump float;

uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform float u_intensity;
uniform float u_threshold;

in vec2 v_texCoord;
out vec4 fragColor;

void main() {
    vec4 color = texture(u_texture, v_texCoord);
    
    // Extract bright areas above threshold
    float brightness = (color.r * 0.299 + color.g * 0.587 + color.b * 0.114);
    float bloom = max(0.0, brightness - u_threshold);
    
    // Apply bloom intensity
    vec4 bloomColor = color * bloom * u_intensity;
    
    // Combine original color with bloom
    fragColor = color + bloomColor;
}
