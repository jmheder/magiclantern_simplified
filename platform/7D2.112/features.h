#define FEATURE_VRAM_RGBA

//#define CONFIG_HELLO_WORLD

// Don't Click Me menu looks to be intended as a place
// for devs to put custom code in debug.c run_test(),
// and allowing triggering from a menu context.
#define FEATURE_DONT_CLICK_ME
#define FEATURE_SHOW_SHUTTER_COUNT
#define FEATURE_SHOW_FREE_MEMORY
#define FEATURE_SCREENSHOT

//#define CONFIG_TSKMON
#define FEATURE_SHOW_TASKS
//#define FEATURE_SHOW_CPU_USAGE
//#define FEATURE_SHOW_GUI_EVENTS

// enable global draw
//#define FEATURE_GLOBAL_DRAW
//#define FEATURE_CROPMARKS
//#define FEATURE_POWERSAVE_LIVEVIEW

#define FEATURE_INTERVALOMETER
#define FEATURE_BULB_TIMER
#define FEATURE_FPS_OVERRIDE

// explicitly disable stuff that don't work or may break things
#undef CONFIG_AUTOBACKUP_ROM
#undef CONFIG_ADDITIONAL_VERSION

