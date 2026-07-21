import { defineConfig } from 'vite';
import { resolve } from 'path';

export default defineConfig({
  root: 'Build_Release',
  publicDir: false,

  server: {
    port: 5173,
    host: '0.0.0.0',
    headers: {
      'Cross-Origin-Opener-Policy': 'same-origin',
      'Cross-Origin-Embedder-Policy': 'require-corp',
    },
  },

  preview: {
    port: 5174,
    host: '0.0.0.0',
  },

  build: {
    outDir: resolve(import.meta.dirname, 'dist'),
    emptyOutDir: true,
    rollupOptions: {
      input: resolve(import.meta.dirname, 'Build_Release/captain-claw-web.html'),
    },
    assetsInlineLimit: 0,
  },

  assetsInclude: ['**/*.wasm', '**/*.data', '**/*.ZIP'],

  optimizeDeps: {
    exclude: ['openclaw.js'],
  },

  plugins: [
    {
      name: 'rewrite-root',
      configureServer(server) {
        server.middlewares.use((req, _res, next) => {
          if (req.url === '/') req.url = '/captain-claw-web.html';
          next();
        });
      },
    },
  ],
});
