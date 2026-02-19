#ifndef GENERATOR_H
#define GENERATOR_H

#include "models.h"

// Generate N users
User *generate_users(int count);

// Generate N channels, assigning creators and members from the user list
Channel *generate_channels(int count, User *users, int user_count);

// Generate M messages, distributed across channels and users
// Returns an array of messages. The caller must free it.
// The messages are sorted by timestamp if possible, or we sort later.
Message *generate_messages(int count, Channel *channels, int channel_count, User *users, int user_count);

void free_users(User *users, int count);
void free_channels(Channel *channels, int count);
// Messages array is a single block, but text is dynamic
void free_messages(Message *messages, int count);

#endif // GENERATOR_H
