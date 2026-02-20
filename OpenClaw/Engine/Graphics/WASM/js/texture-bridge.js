/**
 * Texture Bridge for WASM Module
 * 
 * This bridge provides texture functionality to the C++ WASM module
 * without requiring any SDL dependencies. It handles:
 * - Texture loading from various sources (URLs, data URLs, etc.)
 * - Texture caching and memory management
 * - WebGL and WebGPU texture binding
 * - Texture format detection and conversion
 */

class TextureBridge {
    constructor() {
        this.textures = new Map(); // textureId -> texture data
        this.textureNames = new Map(); // textureId -> name mapping
        this.isInitialized = false;
        this.defaultTexture = null; // Fallback texture
        this.textureCache = new Map(); // name -> texture data for caching
    }
    
    /**
     * Initialize the texture bridge
     */
    initialize() {
        if (this.isInitialized) {
            console.log('[TextureBridge] Already initialized');
            return true;
        }
        
        // Create a default fallback texture
        this.createDefaultTexture();
        
        this.isInitialized = true;
        console.log('[TextureBridge] Texture bridge initialized successfully');
        return true;
    }
    
    /**
     * Create a default fallback texture (checkerboard pattern)
     */
    createDefaultTexture() {
        const canvas = document.createElement('canvas');
        canvas.width = 64;
        canvas.height = 64;
        const ctx = canvas.getContext('2d');
        
        // Create checkerboard pattern
        const size = 8;
        for (let y = 0; y < canvas.height; y += size) {
            for (let x = 0; x < canvas.width; x += size) {
                const isEven = ((x / size) + (y / size)) % 2 === 0;
                ctx.fillStyle = isEven ? '#ffffff' : '#888888';
                ctx.fillRect(x, y, size, size);
            }
        }
        
        this.defaultTexture = {
            id: 0,
            name: 'default',
            width: 64,
            height: 64,
            format: 'RGBA',
            hasTransparency: false,
            canvas: canvas,
            webglTexture: null,
            webgpuTexture: null
        };
        
        console.log('[TextureBridge] Default texture created');
    }
    
    /**
     * Load a texture by name/path
     */
    async loadTexture(name, textureId) {
        try {
            // Check cache first
            if (this.textureCache.has(name)) {
                const cachedTexture = this.textureCache.get(name);
                const texture = this.cloneTexture(cachedTexture, textureId);
                this.textures.set(textureId, texture);
                this.textureNames.set(textureId, name);
                
                console.log('[TextureBridge] Texture loaded from cache:', name, 'ID:', textureId);
                return true;
            }
            
            // Try to load from various sources
            let texture = null;
            
            // Try loading as an image first
            if (name.startsWith('http') || name.startsWith('data:') || name.startsWith('/')) {
                texture = await this.loadImageTexture(name, textureId);
            } else {
                // Try to construct a path to game assets
                const assetPath = this.resolveAssetPath(name);
                texture = await this.loadImageTexture(assetPath, textureId);
            }
            
            if (texture) {
                this.textures.set(textureId, texture);
                this.textureNames.set(textureId, name);
                this.textureCache.set(name, texture);
                
                console.log('[TextureBridge] Texture loaded successfully:', name, 'ID:', textureId, 'Size:', texture.width, 'x', texture.height);
                return true;
            }
            
            // Fallback to default texture
            const defaultTexture = this.cloneTexture(this.defaultTexture, textureId);
            this.textures.set(textureId, defaultTexture);
            this.textureNames.set(textureId, name);
            
            console.log('[TextureBridge] Using default texture for:', name, 'ID:', textureId);
            return true;
            
        } catch (error) {
            console.error('[TextureBridge] Failed to load texture:', name, error);
            
            // Fallback to default texture
            const defaultTexture = this.cloneTexture(this.defaultTexture, textureId);
            this.textures.set(textureId, defaultTexture);
            this.textures.set(textureId, defaultTexture);
            this.textureNames.set(textureId, name);
            
            return true; // Return true since we have a fallback
        }
    }
    
    /**
     * Load texture from image source
     */
    async loadImageTexture(src, textureId) {
        return new Promise((resolve, reject) => {
            const img = new Image();
            img.crossOrigin = 'anonymous';
            
            img.onload = () => {
                const canvas = document.createElement('canvas');
                canvas.width = img.naturalWidth;
                canvas.height = img.naturalHeight;
                
                const ctx = canvas.getContext('2d');
                ctx.drawImage(img, 0, 0);
                
                // Check for transparency
                const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
                const hasTransparency = this.checkTransparency(imageData.data);
                
                const texture = {
                    id: textureId,
                    name: src,
                    width: canvas.width,
                    height: canvas.height,
                    format: 'RGBA',
                    hasTransparency: hasTransparency,
                    canvas: canvas,
                    webglTexture: null,
                    webgpuTexture: null
                };
                
                resolve(texture);
            };
            
            img.onerror = () => {
                reject(new Error(`Failed to load image: ${src}`));
            };
            
            img.src = src;
        });
    }
    
    /**
     * Check if image data has transparency
     */
    checkTransparency(imageData) {
        for (let i = 3; i < imageData.length; i += 4) {
            if (imageData[i] < 255) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Clone a texture with a new ID
     */
    cloneTexture(originalTexture, newId) {
        return {
            id: newId,
            name: originalTexture.name,
            width: originalTexture.width,
            height: originalTexture.height,
            format: originalTexture.format,
            hasTransparency: originalTexture.hasTransparency,
            canvas: originalTexture.canvas,
            webglTexture: null,
            webgpuTexture: null
        };
    }
    
    /**
     * Resolve asset path for game textures
     */
    resolveAssetPath(name) {
        // Try different possible asset locations
        const possiblePaths = [
            `assets/textures/${name}`,
            `assets/${name}`,
            `textures/${name}`,
            `data/${name}`,
            name
        ];
        
        // For now, return the first path - this can be enhanced later
        return possiblePaths[0];
    }
    
    /**
     * Get texture width
     */
    getTextureWidth(textureId) {
        const texture = this.textures.get(textureId);
        return texture ? texture.width : 0;
    }
    
    /**
     * Get texture height
     */
    getTextureHeight(textureId) {
        const texture = this.textures.get(textureId);
        return texture ? texture.height : 0;
    }
    
    /**
     * Get texture format
     */
    getTextureFormat(textureId) {
        const texture = this.textures.get(textureId);
        return texture ? texture.format : 'RGBA';
    }
    
    /**
     * Check if texture has transparency
     */
    hasTransparency(textureId) {
        const texture = this.textures.get(textureId);
        return texture ? texture.hasTransparency : false;
    }
    
    /**
     * Get texture canvas for WebGL rendering
     */
    getTextureCanvas(textureId) {
        const texture = this.textures.get(textureId);
        return texture ? texture.canvas : null;
    }
    
    /**
     * Bind texture to WebGL context
     */
    bindToWebGL(textureId, gl) {
        const texture = this.textures.get(textureId);
        if (!texture || !gl) return null;
        
        if (texture.webglTexture) {
            return texture.webglTexture;
        }
        
        // Create WebGL texture
        const webglTexture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, webglTexture);
        
        // Set texture parameters
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        
        // Upload texture data
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture.canvas);
        
        texture.webglTexture = webglTexture;
        return webglTexture;
    }
    
    /**
     * Bind texture to WebGPU context
     */
    bindToWebGPU(textureId, device, format) {
        const texture = this.textures.get(textureId);
        if (!texture || !device) return null;
        
        if (texture.webgpuTexture) {
            return texture.webgpuTexture;
        }
        
        // Create WebGPU texture
        const webgpuTexture = device.createTexture({
            size: [texture.width, texture.height, 1],
            format: format || 'rgba8unorm',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT
        });
        
        // Upload texture data
        device.queue.writeTexture(
            { texture: webgpuTexture },
            this.getImageData(texture.canvas),
            { bytesPerRow: texture.width * 4 },
            { width: texture.width, height: texture.height }
        );
        
        texture.webgpuTexture = webgpuTexture;
        return webgpuTexture;
    }
    
    /**
     * Get image data from canvas
     */
    getImageData(canvas) {
        const ctx = canvas.getContext('2d');
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
        return imageData.data;
    }
    
    /**
     * Unload a texture
     */
    unloadTexture(textureId) {
        const texture = this.textures.get(textureId);
        if (!texture) return;
        
        // Clean up WebGL texture
        if (texture.webglTexture) {
            // Note: In a real implementation, you'd need to pass the GL context
            // texture.webglTexture.delete();
            texture.webglTexture = null;
        }
        
        // Clean up WebGPU texture
        if (texture.webgpuTexture) {
            texture.webgpuTexture.destroy();
            texture.webgpuTexture = null;
        }
        
        // Remove from maps
        this.textures.delete(textureId);
        this.textureNames.delete(textureId);
        
        console.log('[TextureBridge] Texture unloaded:', textureId);
    }
    
    /**
     * Get texture statistics
     */
    getStats() {
        return {
            totalTextures: this.textures.size,
            cachedTextures: this.textureCache.size,
            defaultTexture: this.defaultTexture ? 1 : 0
        };
    }
    
    /**
     * Shutdown the texture bridge
     */
    shutdown() {
        // Unload all textures
        for (const textureId of this.textures.keys()) {
            this.unloadTexture(textureId);
        }
        
        this.textures.clear();
        this.textureNames.clear();
        this.textureCache.clear();
        this.defaultTexture = null;
        this.isInitialized = false;
        
        console.log('[TextureBridge] Texture bridge shutdown complete');
    }
}

// Export for ES modules and create global instance
export { TextureBridge };

// Keep window global for C++ access and create instance
if (typeof window !== 'undefined') {
  window.TextureBridge = TextureBridge;
  window.textureBridge = new TextureBridge();
}

console.log('[TextureBridge] Texture bridge loaded and ready');
