#ifndef SETTINGS_H
#define SETTINGS_H

/* Stores the settings for the game */
typedef struct Settings{
    bool muted;
    bool fullscreen;
} Settings;

/* 
 * Reads settings from file if present, otherwise
 * creates default settings and a new file
 */
Settings readOrCreateSettingsFromFile(
    const char *fileName
);

/*
 * Writes the specified settings to the specified file
 */
void writeSettingsToFile(
    const Settings *settingsPtr,
    const char *fileName
);

#endif