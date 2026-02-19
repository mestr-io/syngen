#ifndef FAKER_H
#define FAKER_H

#include <stdlib.h>
#include <time.h>
#include "models.h"

// Initialize random seed
void faker_init();

// ID Generation
void faker_get_id(char *buffer, const char *prefix); // Generates U... or C...
void faker_get_uuid(char *buffer); // Generates a full UUID if needed (or just long random string)

// Text Generation
char *faker_lorem_word();
char *faker_lorem_sentence(int min_words, int max_words);
char *faker_lorem_paragraph(int min_sentences, int max_sentences);

// User Generation
void faker_create_user(User *user);

// Channel Generation
void faker_create_channel(Channel *channel, const char *creator_id);

// Timestamp Generation
double faker_get_timestamp(time_t start_time, time_t end_time);

#endif // FAKER_H
