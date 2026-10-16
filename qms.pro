TEMPLATE = subdirs
SUBDIRS = server_library \
          server_daemon \
          tests

server_daemon.depends = server_library
tests.depends = server_library
