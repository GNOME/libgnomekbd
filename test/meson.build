test_names = [
  'gkbd-indicator-test',
  'gkbd-keyboard-drawing-test',
  'gkbd-status-test',
]

foreach test_name : test_names
  test_exec = executable(
    test_name,
    test_name + '.c',
    dependencies: libgnomekbdui_dep,
    c_args: [
      '-DGNOMELOCALEDIR="@0@"'.format (get_option('prefix') / get_option('datadir') / 'locale'),
    ],
    build_by_default: true,
  )
endforeach
