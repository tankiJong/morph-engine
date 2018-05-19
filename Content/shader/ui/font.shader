name: shader/ui/font
pass:
-
  program:
    path: /$Content/shader/progs/font_sdf
    
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
