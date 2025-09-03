#version 300 es
precision mediump float;

uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform float u_intensity;
uniform float u_radius;

in vec2 v_texCoord;
out vec4 fragColor;

void main() {
    vec2 texelSize = 1.0 / u_resolution;
    vec4 color = vec4(0.0);
    float total = 0.0;
    
    // Gaussian blur kernel
    for (float x = -u_radius; x <= u_radius; x += 1.0) {
        for (float y = -u_radius; y <= u_radius; y += 1.0) {
            vec2 offset = vec2(x, y) * texelSize * u_intensity;
            vec2 sampleCoord = v_texCoord + offset;
            
            if (sampleCoord.x >= 0.0 && sampleCoord.x <= 1.0 && 
                sampleCoord.y >= 0.0 && sampleCoord.y <= 1.0) {
                float weight = exp(-(x*x + y*y) / (2.0 * u_radius * u_radius));
                color += texture(u_texture, sampleCoord) * weight;
                total += weight;
            }
        }
    }
    
    fragColor = color / total;
}
