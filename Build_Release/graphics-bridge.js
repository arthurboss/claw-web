/**
 * WebGL Graphics Bridge for WASM Module
 * 
 * This bridge provides WebGL functionality to the C++ WASM module
 * without requiring any SDL dependencies. It handles:
 * - WebGL context creation and management
 * - Basic shader compilation and management
 * - Buffer management (VBO, IBO)
 * - Simple 2D rendering operations
 */

class WebGLBridge {
    constructor() {
        this.gl = null;
        this.canvas = null;
        this.program = null;
        this.vertexBuffer = null;
        this.indexBuffer = null;
        this.isInitialized = false;
        
        // Basic vertex shader for 2D rendering
        this.vertexShaderSource = `
            attribute vec2 a_position;
            attribute vec4 a_color;
            varying vec4 v_color;
            
            void main() {
                gl_Position = vec4(a_position, 0.0, 1.0);
                v_color = a_color;
            }
        `;
        
        // Basic fragment shader for 2D rendering
        this.fragmentShaderSource = `
            precision mediump float;
            varying vec4 v_color;
            
            void main() {
                gl_FragColor = v_color;
            }
        `;
    }
    
    /**
     * Initialize WebGL context and basic resources
     */
    initialize() {
        try {
            // Get the canvas from the Emscripten module
            if (typeof Module !== 'undefined' && Module.canvas) {
                this.canvas = Module.canvas;
            } else {
                console.error('[WebGLBridge] Module.canvas not found');
                return false;
            }
            
            // Create WebGL context
            this.gl = this.canvas.getContext('webgl2') || 
                      this.canvas.getContext('webgl') ||
                      this.canvas.getContext('experimental-webgl');
            
            if (!this.gl) {
                console.error('[WebGLBridge] WebGL not supported');
                return false;
            }
            
            console.log('[WebGLBridge] WebGL context created successfully');
            
            // Set basic WebGL state
            this.gl.enable(this.gl.BLEND);
            this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
            this.gl.clearColor(0.0, 0.0, 0.0, 1.0);
            
            // Create shaders and program
            if (!this.createShaders()) {
                return false;
            }
            
            // Create basic buffers
            if (!this.createBuffers()) {
                return false;
            }
            
            this.isInitialized = true;
            console.log('[WebGLBridge] Initialization complete');
            return true;
            
        } catch (error) {
            console.error('[WebGLBridge] Initialization failed:', error);
            return false;
        }
    }
    
    /**
     * Create and compile shaders
     */
    createShaders() {
        try {
            // Create vertex shader
            const vertexShader = this.gl.createShader(this.gl.VERTEX_SHADER);
            this.gl.shaderSource(vertexShader, this.vertexShaderSource);
            this.gl.compileShader(vertexShader);
            
            if (!this.gl.getShaderParameter(vertexShader, this.gl.COMPILE_STATUS)) {
                console.error('[WebGLBridge] Vertex shader compilation failed:', 
                            this.gl.getShaderInfoLog(vertexShader));
                return false;
            }
            
            // Create fragment shader
            const fragmentShader = this.gl.createShader(this.gl.FRAGMENT_SHADER);
            this.gl.shaderSource(fragmentShader, this.fragmentShaderSource);
            this.gl.compileShader(fragmentShader);
            
            if (!this.gl.getShaderParameter(fragmentShader, this.gl.COMPILE_STATUS)) {
                console.error('[WebGLBridge] Fragment shader compilation failed:', 
                            this.gl.getShaderInfoLog(fragmentShader));
                return false;
            }
            
            // Create program and link shaders
            this.program = this.gl.createProgram();
            this.gl.attachShader(this.program, vertexShader);
            this.gl.attachShader(this.program, fragmentShader);
            this.gl.linkProgram(this.program);
            
            if (!this.gl.getProgramParameter(this.program, this.gl.LINK_STATUS)) {
                console.error('[WebGLBridge] Program linking failed:', 
                            this.gl.getProgramInfoLog(this.program));
                return false;
            }
            
            console.log('[WebGLBridge] Shaders created and linked successfully');
            return true;
            
        } catch (error) {
            console.error('[WebGLBridge] Shader creation failed:', error);
            return false;
        }
    }
    
    /**
     * Create basic vertex and index buffers
     */
    createBuffers() {
        try {
            // Create vertex buffer for a quad
            this.vertexBuffer = this.gl.createBuffer();
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertexBuffer);
            
            // Create index buffer for a quad
            this.indexBuffer = this.gl.createBuffer();
            this.gl.bindBuffer(this.gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
            
            console.log('[WebGLBridge] Buffers created successfully');
            return true;
            
        } catch (error) {
            console.error('[WebGLBridge] Buffer creation failed:', error);
            return false;
        }
    }
    
    /**
     * Set viewport dimensions
     */
    setViewport(x, y, width, height) {
        if (this.gl && this.isInitialized) {
            this.gl.viewport(x, y, width, height);
        }
    }
    
    /**
     * Clear the screen with specified color
     */
    clear(r, g, b, a) {
        if (this.gl && this.isInitialized) {
            this.gl.clearColor(r, g, b, a);
            this.gl.clear(this.gl.COLOR_BUFFER_BIT);
        }
    }
    
    /**
     * Present the frame
     */
    present() {
        // In WebGL, rendering is immediate, so this is mostly a no-op
        // But we can use it for any frame-finalization logic
        if (this.gl && this.isInitialized) {
            // Ensure all commands are processed
            this.gl.flush();
        }
    }
    
    /**
     * Render a colored quad
     */
    renderQuad(x, y, width, height, r, g, b, a) {
        if (!this.gl || !this.isInitialized || !this.program) {
            return;
        }
        
        try {
            // Use the shader program
            this.gl.useProgram(this.program);
            
            // Convert screen coordinates to normalized device coordinates
            const x1 = (x / this.canvas.width) * 2 - 1;
            const y1 = (y / this.canvas.height) * 2 - 1;
            const x2 = ((x + width) / this.canvas.width) * 2 - 1;
            const y2 = ((y + height) / this.canvas.height) * 2 - 1;
            
            // Quad vertices (position + color)
            const vertices = new Float32Array([
                x1, y1, r, g, b, a,  // bottom-left
                x2, y1, r, g, b, a,  // bottom-right
                x2, y2, r, g, b, a,  // top-right
                x1, y2, r, g, b, a   // top-left
            ]);
            
            // Quad indices
            const indices = new Uint16Array([
                0, 1, 2,  // first triangle
                0, 2, 3   // second triangle
            ]);
            
            // Upload vertex data
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertexBuffer);
            this.gl.bufferData(this.gl.ARRAY_BUFFER, vertices, this.gl.STATIC_DRAW);
            
            // Upload index data
            this.gl.bindBuffer(this.gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
            this.gl.bufferData(this.gl.ELEMENT_ARRAY_BUFFER, indices, this.gl.STATIC_DRAW);
            
            // Set up vertex attributes
            const positionLocation = this.gl.getAttribLocation(this.program, 'a_position');
            const colorLocation = this.gl.getAttribLocation(this.program, 'a_color');
            
            this.gl.enableVertexAttribArray(positionLocation);
            this.gl.enableVertexAttribArray(colorLocation);
            
            // Position attribute (2 floats)
            this.gl.vertexAttribPointer(positionLocation, 2, this.gl.FLOAT, false, 24, 0);
            
            // Color attribute (4 floats)
            this.gl.vertexAttribPointer(colorLocation, 4, this.gl.FLOAT, false, 24, 8);
            
            // Draw the quad
            this.gl.drawElements(this.gl.TRIANGLES, 6, this.gl.UNSIGNED_SHORT, 0);
            
        } catch (error) {
            console.error('[WebGLBridge] Render quad failed:', error);
        }
    }
    
    /**
     * Clean up resources
     */
    cleanup() {
        if (this.gl) {
            if (this.vertexBuffer) {
                this.gl.deleteBuffer(this.vertexBuffer);
                this.vertexBuffer = null;
            }
            
            if (this.indexBuffer) {
                this.gl.deleteBuffer(this.indexBuffer);
                this.indexBuffer = null;
            }
            
            if (this.program) {
                this.gl.deleteProgram(this.program);
                this.program = null;
            }
            
            this.gl = null;
        }
        
        this.isInitialized = false;
        console.log('[WebGLBridge] Cleanup complete');
    }
}

// Export for ES modules and create global instance
export { WebGLBridge };

// Keep window global for C++ access and create instance
if (typeof window !== 'undefined') {
  window.WebGLBridge = WebGLBridge;
  window.webglBridge = new WebGLBridge();
}

console.log('[WebGLBridge] Graphics bridge loaded and ready');
