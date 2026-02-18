#pragma once

// WebGPU shader definitions for menu rendering

// Vertex shader for 2D sprite rendering
const char* const VERTEX_SHADER_SOURCE = R"(
struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) texCoord: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) texCoord: vec2<f32>,
}

struct Uniforms {
    transform: mat4x4<f32>,
    alpha: f32,
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(1) @binding(0) var textureSampler: texture_2d<f32>;
@group(1) @binding(1) var textureSamplerLinear: sampler;

@vertex
fn vertexMain(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    output.position = uniforms.transform * vec4<f32>(input.position, 0.0, 1.0);
    output.texCoord = input.texCoord;
    return output;
}
)";

// Fragment shader for 2D sprite rendering
const char* const FRAGMENT_SHADER_SOURCE = R"(
struct FragmentInput {
    @location(0) texCoord: vec2<f32>,
}

struct FragmentOutput {
    @location(0) color: vec4<f32>,
}

struct Uniforms {
    transform: mat4x4<f32>,
    alpha: f32,
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(1) @binding(0) var textureSampler: texture_2d<f32>;
@group(1) @binding(1) var textureSamplerLinear: sampler;

@fragment
fn fragmentMain(input: FragmentInput) -> FragmentOutput {
    var output: FragmentOutput;
    var textureColor = textureSample(textureSampler, textureSamplerLinear, input.texCoord);
    output.color = vec4<f32>(textureColor.rgb, textureColor.a * uniforms.alpha);
    return output;
}
)";

// Vertex shader for text rendering
const char* const TEXT_VERTEX_SHADER_SOURCE = R"(
struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) texCoord: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) texCoord: vec2<f32>,
}

struct Uniforms {
    transform: mat4x4<f32>,
    color: vec4<f32>,
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn vertexMain(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    output.position = uniforms.transform * vec4<f32>(input.position, 0.0, 1.0);
    output.texCoord = input.texCoord;
    return output;
}
)";

// Fragment shader for text rendering
const char* const TEXT_FRAGMENT_SHADER_SOURCE = R"(
struct FragmentInput {
    @location(0) texCoord: vec2<f32>,
}

struct FragmentOutput {
    @location(0) color: vec4<f32>,
}

struct Uniforms {
    transform: mat4x4<f32>,
    color: vec4<f32>,
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(1) @binding(0) var textureSampler: texture_2d<f32>;
@group(1) @binding(1) var textureSamplerLinear: sampler;

@fragment
fn fragmentMain(input: FragmentInput) -> FragmentOutput {
    var output: FragmentOutput;
    var textureColor = textureSample(textureSampler, textureSamplerLinear, input.texCoord);
    output.color = vec4<f32>(uniforms.color.rgb, textureColor.r * uniforms.color.a);
    return output;
}
)";
