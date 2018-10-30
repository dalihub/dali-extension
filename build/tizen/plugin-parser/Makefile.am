extension_src_dir = ../../../dali-extension

include ../../../dali-extension/plugin-parser/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali-plugin-parser.la

libdali_plugin_parser_la_SOURCES = $(plugin_parser_src_files)

libdali_plugin_parser_la_DEPENDENCIES =

libdali_plugin_parser_la_CXXFLAGS = \
			-I../../../dali-extension/ \
			$(DLOG_CFLAGS) \
			$(DALI_CFLAGS) \
			-I/usr/include/glib-2.0 \
			-I/usr/lib/glib-2.0/include \
			-I/usr/include/boost \
			-I/usr/include/pkgmgr \
			-Wall -Werror

libdali_plugin_parser_la_LIBADD = \
			$(DLOG_LIBS)


libdali_plugin_parser_la_LDFLAGS = \
                      -rdynamic

