name: shader/default_lit
pass:
- 
  program: 
    define: _LIT;
    path: /$Content/shader/progs/default

  cull: back
  fill: solid
  frontface: ccw
  blend: false
  depth:
    write: true
    test: less
