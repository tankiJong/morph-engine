name: shader/effect/fog
pass:
-
  layer: opaque
  program:
    vert: |
      #version 420 core
      // Attributes
      out vec2 passUV; 
      out vec4 passColor; 

      const vec3 POSITIONS[3] = vec3[] ( 
         vec3(-1.0f, -1.0f, 0.0f), 
         vec3( 3.0f, -1.0f, 0.0f),
         vec3(-1.0f,  3.0f, 0.0f) 
      );
      const vec2 UVS[3] = vec2[] ( 
         vec2( 0.0f, 0.0f ), 
         vec2( 2.0f, 0.0f ),
         vec2( 0.0f, 2.0f ) 
      );

      out vec4 outColor; 

      void main( void )
      {
         int idx = gl_VertexID; 
         passUV = UVS[idx]; 
         passColor = vec4(1,1,1,1); 
        gl_Position = vec4( POSITIONS[idx], 1.0f ); // we pass out a clip coordinate
      }
    frag: |
      #version 420 core

      layout(binding = 0) uniform sampler2D gTexDepth;
      in vec2 passUV; 
      out vec4 outColor;
      float minDistance = .95f;
      float maxDistance = .99f;
      void main( void )
      {
        float depth = texture(gTexDepth, passUV).r;

        depth = (depth - minDistance) / (maxDistance - minDistance);
        depth = clamp(depth, 0.f, 1.f);
        outColor = vec4(1.f) * depth;
      }
  depth:
    write: false
    test: always
  blend:
    color:
      op: add
      src: one
      dest: one
    alpha:
      op: add
      src: one
      dest: zero