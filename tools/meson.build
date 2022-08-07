gkbd_keyboard_display_exe = executable(
  'gkbd-keyboard-display',
  'gkbd-keyboard-display.c',
  dependencies: libgnomekbdui_dep,
  include_directories: root_include,
  c_args: [
    '-DGNOMELOCALEDIR="@0@"'.format(get_option('prefix') / get_option('datadir') / 'locale')
  ],
  install: true,
)

gkbd_keyboard_display_cfg = configuration_data()
gkbd_keyboard_display_cfg.set('VERSION', meson.project_version())

gkbd_keyboard_display_in = configure_file(
  input: 'gkbd-keyboard-display.desktop.in.in',
  output: 'gkbd-keyboard-display.desktop.in',
  configuration: gkbd_keyboard_display_cfg,
)

i18n.merge_file(
  input: gkbd_keyboard_display_in,
  output: 'gkbd-keyboard-display.desktop',
  po_dir: '../po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / 'applications',
)
