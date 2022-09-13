pkg_data_dir = get_option('prefix') / get_option('datadir') / 'libgnomekbd'
ui_dir = pkg_data_dir / 'ui'
icons_dir = pkg_data_dir / 'icons'

pkg = import('pkgconfig')

install_data(
  'show-layout.ui',
  install_dir: ui_dir
)

install_data(
  'kbdled-caps-lock.svg',
  'kbdled-num-lock.svg',
  'kbdled-scroll-lock.svg',
  install_dir: icons_dir
)

gsettings_cfg = configuration_data()
gsettings_cfg.set('GETTEXT_PACKAGE', gettext_domain)

gsettings_files = files(
  'org.gnome.libgnomekbd.desktop.gschema.xml.in',
  'org.gnome.libgnomekbd.keyboard.gschema.xml.in',
  'org.gnome.libgnomekbd.gschema.xml.in',
)

foreach gsettings_file : gsettings_files
  configure_file(
    input: gsettings_file,
    output: '@BASENAME@',
    configuration: gsettings_cfg,
    install: true,
    install_dir: get_option('prefix') / get_option('datadir') / 'glib-2.0' / 'schemas'
)
endforeach

libgnomekbd_deps = [
  glib_dep,
  gobject_dep,
  gio_dep,
  gdk_dep,
  gtk_dep,
  x11_dep,
  libxklavier_dep,
]

libgnomekbd_sources = files(
  'gkbd-desktop-config.c',
  'gkbd-keyboard-config.c',
  'gkbd-util.c',
)

libgnomekbd_headers = files(
  'gkbd-desktop-config.h',
  'gkbd-keyboard-config.h',
  'gkbd-util.h',
)

libgnomekbd_lib = library(
  'gnomekbd',
  libgnomekbd_sources,
  config_h,
  dependencies: libgnomekbd_deps,
  include_directories: root_include,
  version: library_version,
  install: true,
)

pkg.generate(
  libgnomekbd_lib,
  name: 'libgnomekbd',
  description: 'GNOME keyboard shared library',
  version: meson.project_version(),
)

libgnomekbd_dep = declare_dependency(
  link_with: libgnomekbd_lib,
  include_directories: root_include,
  dependencies: libgnomekbd_deps,
  sources: config_h,
)

libgnomekbdui_sources = files(
  'gkbd-configuration.c',
  'gkbd-indicator-config.c',
  'gkbd-indicator.c',
  'gkbd-status.c',
  'gkbd-keyboard-drawing.c',
)

libgnomekbdui_headers = files(
  'gkbd-configuration.h',
  'gkbd-indicator.h',
  'gkbd-status.h',
  'gkbd-indicator-config.h',
  'gkbd-keyboard-drawing.h',
)

gkbd_indicator_marshal = gnome.genmarshal(
  'gkbd-indicator-marshal',
  prefix: 'gkbd_indicator',
  sources: 'gkbd-indicator-marshal.list',
)

gkbd_keyboard_drawing_marshal = gnome.genmarshal(
  'gkbd-keyboard-drawing-marshal',
  prefix: 'gkbd_keyboard_drawing',
  sources: 'gkbd-keyboard-drawing-marshal.list',
)


libgnomekbdui_deps = [
  libgnomekbd_dep,
  m_dep,
]

libgnomekbdui_lib = library(
  'gnomekbdui',
  libgnomekbdui_sources,
  config_h,
  gkbd_indicator_marshal,
  gkbd_keyboard_drawing_marshal,
  dependencies: libgnomekbdui_deps,
  include_directories: root_include,
  version: library_version,
  c_args: [
    '-DUIDIR="@0@"'.format(ui_dir),
    '-DDATADIR="@0@"'.format(pkg_data_dir),
    '-DICONSDIR="@0@"'.format(icons_dir),
  ],
  install: true,
)

pkg.generate(
  libgnomekbdui_lib,
  name: 'libgnomekbdui',
  description: 'GNOME keyboard shared library',
  version: meson.project_version(),
)

libgnomekbdui_dep = declare_dependency(
  link_with: libgnomekbdui_lib,
  include_directories: root_include,
  dependencies: libgnomekbdui_deps,
  sources: [config_h, gkbd_indicator_marshal, gkbd_keyboard_drawing_marshal],
)

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

install_headers(
  libgnomekbd_headers,
  libgnomekbdui_headers,
  subdir: 'libgnomekbd',
)

i18n.merge_file(
  input: 'gkbd-keyboard-display.desktop.in',
  output: 'gkbd-keyboard-display.desktop',
  po_dir: '../po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / 'applications',
)

install_data(
  'libgnomekbd.convert',
  install_dir: get_option('prefix') / get_option('datadir') / 'GConf' / 'gsettings'
)

if get_option('introspection')
  gir_dir = join_paths(get_option('prefix') / get_option('datadir'), '@0@-@1@'.format('gir', gnomekbd_gir_version))
  typelib_dir = join_paths(get_option('prefix') / get_option('libdir'), '@0@-@1@'.format('girepository', gnomekbd_gir_version))

  gnome.generate_gir(
    libgnomekbd_lib, libgnomekbdui_lib,
    sources: [
      libgnomekbd_sources,
      libgnomekbdui_sources,
      libgnomekbd_headers,
      libgnomekbdui_headers,
      gkbd_indicator_marshal,
      gkbd_keyboard_drawing_marshal,
    ],
    dependencies: [ libgnomekbd_deps, m_dep ],
    include_directories: root_include,
    namespace: 'Gkbd',
    nsversion: '3.0',
    includes: ['GObject-2.0', 'Gtk-3.0', 'xlib-2.0', 'Xkl-1.0'],
    install: true,
    install_dir_gir: gir_dir,
    install_dir_typelib: typelib_dir,
  )
endif
