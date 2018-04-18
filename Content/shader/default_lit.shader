name: shader/default_lit
program:
  define: _LIT
  path: Data/shader/progs/default

cull: back
fill: solid
frontface: ccw
blend: false
depth:
  write: true
  test: less
