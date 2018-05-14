name: shader/effect/bloom
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

      void main( void )
      {
         int idx = gl_VertexID; 
         passUV = UVS[idx]; 
         passColor = vec4(1,1,1,1); 
        gl_Position = vec4( POSITIONS[idx], 1.0f ); // we pass out a clip coordinate
      }
    frag: |
      #version 420 core
      layout(binding = 0) uniform sampler2D gTexDiffuse;

      in vec2 passUV; 

      out vec4 outColor; 

      const float GUASSIAN[5] = float[]  (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
      const float STEP_SIZE = 8.0f; 

      // Work
      void main( void )
      {
         // get pixel offset for first level  mip
         vec2 offset = vec2( 1.0f / textureSize( gTexDiffuse, 0).x, 0.0f ) * STEP_SIZE;

         vec4 result = texture( gTexDiffuse, passUV ) * GUASSIAN[0]; 
         vec2 item_offset = vec2(0.0f); 
         for (int i = 1; i < 5; ++i) {
            item_offset += offset; 
            result += texture( gTexDiffuse, passUV + item_offset ) * GUASSIAN[i]; 
            result += texture( gTexDiffuse, passUV - item_offset ) * GUASSIAN[i];
         } 

         outColor = result; 
      }

  depth:
    write: false
    test: always
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

      void main( void )
      {
         int idx = gl_VertexID; 
         passUV = UVS[idx]; 
         passColor = vec4(1,1,1,1); 
        gl_Position = vec4( POSITIONS[idx], 1.0f ); // we pass out a clip coordinate
      }
    frag: |
      #version 420 core

      // Suggest always manually setting bindings - again, consitancy with 
      // other rendering APIs and well as you can make assumptions in your
      // engine without having to query
      layout(binding = 0) uniform sampler2D gTexColor;

      // Passed values; 
      in vec2 passUV; 

      // Outputs
      out vec4 outColor; 

      const float GUASSIAN[5] = float[]  (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
      const float STEP_SIZE = 8.0f; 

      // Work
      void main( void )
      {
         // get pixel offset for first level  mip
         vec2 offset = vec2( 0.0f, 1.0f / textureSize( gTexColor, 0).y ) * STEP_SIZE; 

         vec4 result = texture( gTexColor, passUV ) * GUASSIAN[0]; 
         vec2 item_offset = vec2(0.0f); 
         for (int i = 1; i < 5; ++i) {
            item_offset += offset; 
            result += texture( gTexColor, passUV + item_offset ) * GUASSIAN[i]; 
            result += texture( gTexColor, passUV - item_offset ) * GUASSIAN[i];
         } 

         outColor = result; 
      }

  depth:
    write: false
    test: always
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

      void main( void )
      {
         int idx = gl_VertexID; 
         passUV = UVS[idx]; 
         passColor = vec4(1,1,1,1); 
        gl_Position = vec4( POSITIONS[idx], 1.0f ); // we pass out a clip coordinate
      }
    frag: |
      #version 420 core
      layout(binding = 0) uniform sampler2D gTexDiffuse;

      // Passed values; 
      in vec2 passUV; 

      // Outputs
      out vec4 outColor; 


      // Work
      void main( void )
      {
         outColor = texture(gTexDiffuse, passUV);
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
      dest: one