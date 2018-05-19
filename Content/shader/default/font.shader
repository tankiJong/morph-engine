name: shader/font
pass:
-
  program:
    path: /$Content/shader/progs/font_sdf
    
  cull: none
  fill: solid
  frontface: ccw
  blend: false
  depth:
    write: true
    test: less
