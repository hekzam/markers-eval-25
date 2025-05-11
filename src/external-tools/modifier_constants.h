// external-tools/modifier_constants.h
#ifndef MODIFIER_CONSTANTS_H
#define MODIFIER_CONSTANTS_H

// Rotation (degrés)
constexpr float MIN_ROTATE = -5.0f;
constexpr float MAX_ROTATE = 5.0f;

// Translation (pixels)
constexpr int MIN_TRANS = -5;
constexpr int MAX_TRANS = 5;

// Poivre et sel (pourcentages)
constexpr float MIN_SALT = 0.00f;
constexpr float MAX_SALT = 0.10f;
constexpr float MIN_PEPPER = 0.00f;
constexpr float MAX_PEPPER = 0.10f;

// Bruit gaussien
constexpr float MIN_OFFSET = 1.0f;
constexpr float MAX_OFFSET = 5.0f;
constexpr float MIN_DISP = 1.0f;
constexpr float MAX_DISP = 5.0f;

// Contraste / luminosité
constexpr int MIN_CONTRAST = -10;
constexpr int MAX_CONTRAST = 10;
constexpr int MIN_BRIGHT = -5;
constexpr int MAX_BRIGHT = 5;

// Taches d’encre
constexpr int MIN_NB_SPOT = 0;
constexpr int MAX_NB_SPOT = 8;
constexpr int MIN_RMIN = 4;
constexpr int MAX_RMIN = 8;
constexpr int MIN_RMAX = 9;
constexpr int MAX_RMAX = 35;

#endif // MODIFIER_CONSTANTS_H
