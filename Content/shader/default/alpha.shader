name: shader/alpha
pass:
-
  program:
    path: /$Content/shader/progs/default
  layer: alpha
  cull: back
  fill: solid
  frontface: ccw
  blend:
    color:
      op: add
      src: src_alpha
      dest: inv_src_alpha
    alpha:
      op: add
      src: one
      dest: one
  depth:
    write: true
    test: less
