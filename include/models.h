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
    char avatar_hash[13];  // 12 chars + null
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
    char user[12];
    double ts;
} ReplyInfo;

typedef struct {
    char user[12];         // User ID
    char channel[12];      // Channel ID (Internal use for grouping)
    char type[16];         // message
    double ts;             // 1756191830.368749
    char *text;            // Dynamic text content
    
    // Threading
    double thread_ts;      // Parent timestamp (if 0, not a thread/reply)
    char parent_user_id[12]; // Only for replies
    
    // Parent metadata (if this message is a parent)
    int reply_count;
    ReplyInfo *replies;    // Dynamic array of replies
    int replies_capacity;  // Internal use for realloc
    double latest_reply;
} Message;

#endif // MODELS_H
