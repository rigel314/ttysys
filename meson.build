project('ttysys', 'c')
ncurses_dep = dependency('ncurses')
cc = meson.get_compiler('c')
libdl = cc.find_library('dl')
libm = cc.find_library('m', required:false)
# explist_file = configure_file(
# 	input: 'src/ttysys_export_list.txt',
# 	output: 'ttysys_export_list.txt',
# 	configuration: configuration_data()
# )
explist_args = '-Wl,--dynamic-list=@0@'.format(join_paths(meson.current_source_dir(), 'src', 'ttysys_export_list.txt'))
# explist_args = '-Wl,--dynamic-list=@0@'.format(explist_file)
if(cc.has_link_argument(explist_args) != true)
	error('no support for dynamic-list on linker, ttysys plugins unavailable')
endif
# explist_dep = declare_dependency(
# 	sources: 'src/ttysys_export_list.txt',
# 	link_args: explist_args,
# )
executable('ttysys',
	[
		'src/main.c',
		'src/common.c',
		'src/ncurses-help.c',
		'src/signals.c',
		'src/ttysys_api.c',
		'src/windowlist.c',
	],
	link_args: explist_args,
	dependencies: [ncurses_dep, libdl, libm],
	install: true
)
shared_module('cpu',
    'src/libs/cpu.c',
    dependencies: ncurses_dep,
    install: true,
)
shared_module('mem',
    'src/libs/mem.c',
    dependencies: ncurses_dep,
    install: true,
)
shared_module('net',
    'src/libs/net.c',
    dependencies: ncurses_dep,
    install: true,
)
shared_module('val',
    'src/libs/val.c',
    dependencies: ncurses_dep,
    install: true,
)
shared_module('watch',
    'src/libs/watch.c',
    dependencies: ncurses_dep,
    install: true,
)
