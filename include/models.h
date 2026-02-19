#ifndef MODELS_H
#define MODELS_H

#include <stdbool.h>
#include <time.h>

// User Model
typedef struct {
    char id[12];           // U02M3TMTV9B
    char name[64];         // smyslov (username)
    char real_name[64];    // Marc
    char email[128];       // smyslov@gmail.com
    char image_original[256];
    bool is_admin;
    bool is_bot;
} User;

// Channel Model
typedef struct {
    char id[12];           // C02TZQX58FJ
    char name[64];         // general
    long created;          // timestamp
    char creator[12];      // User ID
    char **members;        // Array of User IDs
    int member_count;
} Channel;

// Message Model
typedef struct {
    char user[12];         // User ID
    char channel[12];      // Channel ID (Internal use for grouping)
    char type[16];         // message
    double ts;             // 1756191830.368749
    char *text;            // Dynamic text content
} Message;

#endif // MODELS_H
