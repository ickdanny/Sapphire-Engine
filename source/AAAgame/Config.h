#ifndef CONFIG_H
#define CONFIG_H

/* The name of the game window */
#define config_windowName "OYCN"

/* The file name of the settings file */
#define config_settingsFileName "oycn.cfg"

/* The width of the game window */
#define config_windowWidth 640

/* The height of the game window */
#define config_windowHeight 480

/* The pixel width of the game */
#define config_graphicsWidth (config_windowWidth / 2)

/* The pixel height of the game */
#define config_graphicsHeight (config_windowHeight / 2)

/* The number of game updates per second */
#define config_updatesPerSecond 60

/* The maximum number of updates between frames */
#define config_maxUpdatesPerFrame 5

#endif