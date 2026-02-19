#ifndef EXPORT_MANAGER_H
#define EXPORT_MANAGER_H

#include "models.h"

// Initialize the export directory
void export_init(const char *base_path);

// Write users.json
void export_write_users(const char *base_path, const User *users, int count);

// Write channels.json
void export_write_channels(const char *base_path, const Channel *channels, int count);

// Write messages to channel/YYYY-MM-DD.json
void export_write_messages(const char *base_path, const Message *messages, int count, const Channel *channels, int channel_count);

// Create the ZIP archive
void export_finalize(const char *base_path, const char *output_filename);

#endif // EXPORT_MANAGER_H
