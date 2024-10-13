SET(SOURCES screens/ui_ScreenMenu.c
    screens/ui_ScreenCFIG.c
    screens/ui_ScreenArm.c
    screens/ui_ScreenTask.c
    screens/ui_Screen1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    fonts/ui_font_bigNum.c)

add_library(ui ${SOURCES})
