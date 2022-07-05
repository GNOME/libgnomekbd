test_exec = executable(
  'gkbd-keyboard-drawing-test',
  'gkbd-keyboard-drawing-test.c',
  dependencies: libgnomekbdui_dep,
  c_args: [
    '-DGNOMELOCALEDIR="@0@"'.format (get_option('prefix') / get_option('datadir') / 'locale'),
  ],
  build_by_default: true,
)
