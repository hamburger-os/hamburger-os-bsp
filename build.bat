del .config
copy %1 .config

call scons --useconfig=.config
call scons -c

call scons -j 4

call scons --target=ua -s

