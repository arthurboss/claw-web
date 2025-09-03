/**
 * WebGPU Graphics Bridge for WASM Module
 * 
 * This bridge provides WebGPU functionality to the C++ WASM module
 * without requiring any SDL dependencies. It handles:
 * - WebGPU context creation and management
 * - WGSL shader compilation and management
 * - Buffer management and rendering pipeline
 * - Performance monitoring and optimization
 */

class WebGPUBridge {
    constructor() {
        this.adapter = null;
        this.device = null;
        this.queue = null;
        this.pipeline = null;
        this.vertexBuffer = null;
        this.uniformBuffer = null;
        this.bindGroup = null;
        this.canvas = null;
        this.context = null;
        this.isInitialized = false;
        
        // WGSL shader sources for 2D rendering
        this.vertexShaderSource = `
            struct VertexInput {
                @location(0) position: vec2<f32>,
                @location(1) color: vec4<f32>,
            };

            struct VertexOutput {
                @builtin(position) position: vec4<f32>,
                @location(0) color: vec4<f32>,
            };

            @vertex
            fn vertexMain(input: VertexInput) -> VertexOutput {
                var output: VertexOutput;
                output.position = vec4<f32>(input.position, 0.0, 1.0);
                output.color = input.color;
                return output;
            }
        `;
        
        this.fragmentShaderSource = `
            @fragment
            fn fragmentMain(@location(0) color: vec4<f32>) -> @location(0) vec4<f32> {
                return color;
            }
        `;
    }
    
    /**
     * Check if WebGPU is available in this browser
     */
    isWebGPUAvailable() {
        return 'gpu' in navigator;
    }
    
    /**
     * Initialize WebGPU context and basic resources
     */
    async initializeWebGPU() {
        try {
            if (!this.isWebGPUAvailable()) {
                console.error('[WebGPUBridge] WebGPU not supported in this browser');
                return false;
            }
            
            // Get the canvas from the Emscripten module
            if (typeof Module !== 'undefined' && Module.canvas) {
                this.canvas = Module.canvas;
            } else {
                console.error('[WebGPUBridge] Module.canvas not found');
                return false;
            }
            
            // Request WebGPU adapter
            this.adapter = await navigator.gpu.requestAdapter({
                powerPreference: 'high-performance'
            });
            
            if (!this.adapter) {
                console.error('[WebGPUBridge] Failed to get WebGPU adapter');
                return false;
            }
            
            console.log('[WebGPUBridge] WebGPU adapter:', this.adapter.name);
            
            // Request WebGPU device
            this.device = await this.adapter.requestDevice({
                requiredFeatures: [],
                requiredLimits: {
                    maxStorageBufferBindingSize: 1024 * 1024 * 1024, // 1GB
                    maxBufferSize: 1024 * 1024 * 1024, // 1GB
                }
            });
            
            if (!this.device) {
                console.error('[WebGPUBridge] Failed to get WebGPU device');
                return false;
            }
            
            console.log('[WebGPUBridge] WebGPU device created successfully');
            
            // Get the command queue
            this.queue = this.device.queue;
            
            // Create shader modules
            const vertexShader = this.device.createShaderModule({
                code: this.vertexShaderSource
            });
            
            const fragmentShader = this.device.createShaderModule({
                code: this.fragmentShaderSource
            });
            
            // Create render pipeline
            this.pipeline = this.device.createRenderPipeline({
                layout: 'auto',
                vertex: {
                    module: vertexShader,
                    entryPoint: 'vertexMain',
                    buffers: [{
                        arrayStride: 24, // 2 floats (position) + 4 floats (color)
                        attributes: [
                            {
                                format: 'float32x2',
                                offset: 0,
                                shaderLocation: 0
                            },
                            {
                                format: 'float32x4',
                                offset: 8,
                                shaderLocation: 1
                            }
                        ]
                    }]
                },
                fragment: {
                    module: fragmentShader,
                    entryPoint: 'fragmentMain',
                    targets: [{
                        format: 'bgra8unorm'
                    }]
                },
                primitive: {
                    topology: 'triangle-list'
                }
            });
            
            // Create vertex buffer
            this.vertexBuffer = this.device.createBuffer({
                size: 1024 * 1024, // 1MB buffer
                usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST,
                mappedAtCreation: false
            });
            
            // Create uniform buffer for transformations
            this.uniformBuffer = this.device.createBuffer({
                size: 64, // 4x4 matrix
                usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
            });
            
            // Create bind group
            this.bindGroup = this.device.createBindGroup({
                layout: this.pipeline.getBindGroupLayout(0),
                entries: [{
                    binding: 0,
                    resource: {
                        buffer: this.uniformBuffer
                    }
                }]
            });
            
            // Get WebGPU context from canvas
            this.context = this.canvas.getContext('webgpu');
            if (!this.context) {
                console.error('[WebGPUBridge] Failed to get WebGPU context from canvas');
                return false;
            }
            
            // Configure swap chain
            this.context.configure({
                device: this.device,
                format: 'bgra8unorm',
                alphaMode: 'premultiplied'
            });
            
            this.isInitialized = true;
            console.log('[WebGPUBridge] WebGPU initialization complete');
            return true;
            
        } catch (error) {
            console.error('[WebGPUBridge] Initialization failed:', error);
            return false;
        }
    }
    
    /**
     * Begin rendering frame
     */
    beginFrame() {
        if (!this.isInitialized || !this.context) return;
        
        // Clear the canvas
        const commandEncoder = this.device.createCommandEncoder();
        const renderPass = commandEncoder.beginRenderPass({
            colorAttachments: [{
                view: this.context.getCurrentTexture().createView(),
                clearValue: { r: 0.0, g: 0.0, b: 0.0, a: 1.0 },
                loadOp: 'clear',
                storeOp: 'store'
            }]
        });
        
        renderPass.setPipeline(this.pipeline);
        renderPass.setVertexBuffer(0, this.vertexBuffer);
        renderPass.end();
        
        this.device.queue.submit([commandEncoder.finish()]);
    }
    
    /**
     * End rendering frame
     */
    endFrame() {
        if (!this.isInitialized) return;
        // Present the frame
        this.context.getCurrentTexture().present();
    }
    
    /**
     * Set viewport dimensions
     */
    setViewport(x, y, width, height) {
        if (!this.isInitialized) return;
        this.viewportX = x;
        this.viewportY = y;
        this.viewportWidth = width;
        this.viewportHeight = height;
    }
    
    /**
     * Clear the screen with specified color
     */
    clear(r, g, b, a) {
        if (!this.isInitialized || !this.context) return;
        
        const commandEncoder = this.device.createCommandEncoder();
        const renderPass = commandEncoder.beginRenderPass({
            colorAttachments: [{
                view: this.context.getCurrentTexture().createView(),
                clearValue: { r: r, g: g, b: b, a: a },
                loadOp: 'clear',
                storeOp: 'store'
            }]
        });
        
        renderPass.end();
        this.device.queue.submit([commandEncoder.finish()]);
    }
    
    /**
     * Present the current frame
     */
    present() {
        if (!this.isInitialized || !this.context) return;
        // Frame is automatically presented in endFrame
    }
    
    /**
     * Render a colored quad using WebGPU
     */
    renderQuad(x, y, width, height, r, g, b, a) {
        if (!this.isInitialized || !this.device || !this.pipeline) return;
        
        try {
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
                x1, y1, r, g, b, a,  // bottom-left
                x2, y2, r, g, b, a,  // top-right
                x1, y2, r, g, b, a   // top-left
            ]);
            
            // Upload vertex data
            this.device.queue.writeBuffer(this.vertexBuffer, 0, vertices);
            
            // Create command encoder for this frame
            const commandEncoder = this.device.createCommandEncoder();
            const renderPass = commandEncoder.beginRenderPass({
                colorAttachments: [{
                    view: this.context.getCurrentTexture().createView(),
                    loadOp: 'load',
                    storeOp: 'store'
                }]
            });
            
            renderPass.setPipeline(this.pipeline);
            renderPass.setVertexBuffer(0, this.vertexBuffer);
            renderPass.draw(6, 1, 0, 0);
            renderPass.end();
            
            // Submit commands
            this.device.queue.submit([commandEncoder.finish()]);
            
        } catch (error) {
            console.error('[WebGPUBridge] Render quad failed:', error);
        }
    }
    
    /**
     * Clean up resources
     */
    shutdownWebGPU() {
        if (this.vertexBuffer) {
            this.vertexBuffer.destroy();
            this.vertexBuffer = null;
        }
        
        if (this.uniformBuffer) {
            this.uniformBuffer.destroy();
            this.uniformBuffer = null;
        }
        
        this.adapter = null;
        this.device = null;
        this.queue = null;
        this.pipeline = null;
        this.bindGroup = null;
        this.context = null;
        this.isInitialized = false;
        
        console.log('[WebGPUBridge] WebGPU shutdown complete');
    }
    
    // C++ interop methods
    getAdapterId() { return this.adapter ? 1 : -1; }
    getDeviceId() { return this.device ? 1 : -1; }
    getQueueId() { return this.queue ? 1 : -1; }
    getPipelineId() { return this.pipeline ? 1 : -1; }
    getBufferId() { return this.vertexBuffer ? 1 : -1; }
}

// Create global instance
window.webGPUBridge = new WebGPUBridge();

console.log('[WebGPUBridge] WebGPU bridge loaded and ready');
