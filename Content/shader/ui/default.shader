name: shader/ui/default
pass:
-
  program:
    path: /$Content/shader/progs/solid
    
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
