project('libgnomekbd', 'c',
        version: '3.26.1',
        default_options: [
          'buildtype=debugoptimized',
          'warning_level=1',
          # We only need c99, but glib needs GNU-specific features
          # https://github.com/mesonbuild/meson/issues/2289
          'c_std=gnu99',
        ],
        meson_version : '>= 0.59',
        license: 'LGPL-2.1-or-later')

library_version = '8.0.0'
gettext_domain = 'libgnomekbd'

gnome = import('gnome')
i18n = import('i18n')

cc = meson.get_compiler('c')

glib_req        = '>= 2.18.0'
gdk_req         = '>= 2.91.7'
gtk_req         = '>= 2.90'
libxklavier_req = '>= 5.2'

glib_dep = dependency('glib-2.0', version: glib_req)
gobject_dep = dependency('gobject-2.0', version: glib_req)
gio_dep = dependency('gio-2.0', version: glib_req)
gdk_dep = dependency('gdk-3.0', version: gdk_req)
gtk_dep = dependency('gtk+-3.0', version: gtk_req)
x11_dep = dependency('x11')
libxklavier_dep = dependency('libxklavier', version: libxklavier_req)
m_dep = cc.find_library('m', required : false)

add_project_arguments('-DGETTEXT_PACKAGE="@0@"'.format(gettext_domain), language: 'c')
add_project_arguments('-DHAVE_CONFIG_H=1', language: 'c')


config_cfg = configuration_data()
config_cfg.set_quoted('VERSION', meson.project_version())

config_h = configure_file(
  output: 'config.h',
  configuration: config_cfg,
)

root_include = include_directories('.')

subdir('libgnomekbd')

subdir('po')

if get_option('tests')
  subdir('test')
endif