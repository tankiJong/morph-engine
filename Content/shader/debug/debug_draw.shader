name: shader/debug
pass:
- 
  program:
    path: /$Content/shader/progs/debug

  cull: back
  fill: solid
  frontface: ccw
  blend: false
  depth:
    write: false
    test: less
-
  program:
    vert: /$Content/shader/progs/debug.vert
    frag: /$Content/shader/progs/font_sdf.frag
        
  cull: none
  blend:
    color:
      op: add
      src: src_alpha
      dest: inv_src_alpha
    alpha:
      op: add
      src: one
      dest: zero
  depth:
    write: false
    test: always
