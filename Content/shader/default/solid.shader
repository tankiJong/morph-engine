name: shader/default/solid
pass:
-
  program:
    path: /$Content/shader/progs/solid
    
  cull: none
  fill: solid
  frontface: ccw
  blend: false
  depth:
    write: true
    test: less
