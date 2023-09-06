copy /Y %1 .config

call scons --useconfig=.config
call scons -c

call scons -j %NUMBER_OF_PROCESSORS%

call scons --target=ua -s

