for %%i in (.\configs\.config.*)do (
    echo %%i
    copy /Y %%i .config

    call menuconfig --silent

    copy /Y .config %%i
)
