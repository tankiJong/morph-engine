name: shader/default
pass:
-
  program:
    path: /$Content/shader/progs/default

  layer: opaque
  cull: back
  fill: solid
  frontface: ccw
  blend: false
  depth:
    write: true
    test: less
