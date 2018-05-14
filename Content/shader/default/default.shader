name: shader/default
pass:
-
  program:
    path: Data/shader/progs/default

  cull: back
  fill: solid
  frontface: ccw
  blend: false
  depth:
    write: true
    test: less
