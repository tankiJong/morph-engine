name: shader/default
pass:
- 
  program:
    path: Data/shader/progs/debug

  cull: back
  fill: solid
  frontface: ccw
  blend: false
  depth:
    write: false
    test: less
