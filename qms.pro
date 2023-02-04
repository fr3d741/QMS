TEMPLATE = subdirs
SUBDIRS = server_library \
          server_daemon

server_daemon.depends = server_library
