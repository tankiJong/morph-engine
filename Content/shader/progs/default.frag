#version 420 core

// Uniforms ==============================================
#ifdef _LIT

#include "../inc/light.glsl"

// Scene related
layout (binding=UNIFORM_USER_1, std140) uniform cSpecBlock
{
   float SPECULAR_AMOUNT; 
   float SPECULAR_POWER;
   vec2 padding0;  
}; 

// uniform float SPECULAR_AMOUNT; // shininess (0 to 1)
// uniform float SPECULAR_POWER; // smoothness (1 to whatever)
#endif

// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;
layout(binding = 1) uniform sampler2D gTexNormal;
layout(binding = 2) uniform sampler2D gTexImmersive;
// // Suggest always manually setting bindings - again, consitancy with 
// // other rendering APIs and well as you can make assumptions in your
// // engine without having to query
// layout(binding = 0) uniform sampler2D gTexDiffuse;

// Attributes ============================================
in vec2 passUV; 
in vec4 passColor; 

#ifdef _LIT
in vec3 passWorldPos;   // new
in vec3 passWorldNormal;// new
in vec3 passEyeDir;
in vec3 passTangent;
in vec3 passBiTangent;
#endif

layout(location = 0)out vec4 outColor; 

// Entry Point ===========================================
void main( void )
{
    vec4 texColor = texture( gTexDiffuse, passUV );

    #ifdef _d_tex_color
    outColor = texColor;
    return;
    #endif

    #ifdef _d_vert_normal
    outColor = vec4((normalize(passWorldNormal) + vec3(1.f)) * .5f, 1.f);
    return;
    #endif    
    
    #ifdef _d_uv
    outColor = vec4(passUV, 0.f, 1.f);
    return;
    #endif
    
#ifdef _LIT
    vec4 normal = vec4(passWorldNormal, 0);

    #ifdef _d_tangent
    outColor = vec4((normalize(passTangent) + vec3(1.f)) * .5f, 1.f);
    return;
    #endif

    #ifdef _d_bitangent
    outColor = vec4((normalize(passBiTangent) + vec3(1.f)) * .5f, 1.f);
    return;
    #endif

    #ifndef _no_normalmap
    vec4 normalColor = texture(gTexNormal, passUV);
    
    #ifdef _d_normal_map
    outColor = normalColor;
    return;
    #endif

    mat4 tbn = mat4(
        vec4(passTangent, 0),
        vec4(passBiTangent, 0),
        normal,
        vec4(0,0,0,1)
    );

    normal = tbn * vec4(normalColor.xyz, 1);

    #ifdef _d_normal
    outColor = vec4((normalize(normal.xyz) + vec3(1,1,0)) *.5f, 1);
    return;
    #endif

    #endif

    outColor = PhongLighting(lights, 
                passWorldPos, 
                normalize(normal.xyz), 
                SPECULAR_AMOUNT, SPECULAR_POWER, 
                texColor,
                normalize(passEyeDir));
    
#else
    outColor = texColor * passColor;
#endif
}