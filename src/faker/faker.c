#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "faker.h"
#include "faker_data.h"

void faker_init() {
    srand((unsigned int)time(NULL));
}

// Helper to get random int in range [min, max]
static int rand_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Helper to generate a random alphanumeric string
static void rand_alphanum(char *buffer, int length) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        buffer[i] = charset[key];
    }
    buffer[length] = '\0';
}

void faker_get_id(char *buffer, const char *prefix) {
    // Slack IDs: U + 8-10 alphanumeric upper
    // E.g. U02M3TMTV9B
    // We'll standardise on Prefix + 10 chars
    strcpy(buffer, prefix);
    rand_alphanum(buffer + strlen(prefix), 10);
}

void faker_get_uuid(char *buffer) {
    // Simplified UUID-like string
    // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    // For now, just a random hex string or similar unique token
    // Slack often uses simple alphanumeric strings for file IDs etc.
    // Let's stick to a 12 char hex for now if needed, or implement full UUID v4
    // Implementation of full UUID v4:
    const char *hex_digits = "0123456789abcdef";
    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            buffer[i] = '-';
        } else if (i == 14) {
            buffer[i] = '4';
        } else if (i == 19) {
            buffer[i] = hex_digits[(rand() % 4) + 8];
        } else {
            buffer[i] = hex_digits[rand() % 16];
        }
    }
    buffer[36] = '\0';
}

char *faker_lorem_word() {
    int idx = rand() % faker_words_count;
    // Return a copy? Or const char*? 
    // The header defines returns as char*, implying ownership. 
    // Let's return a duplicate to be safe for modification/freeing.
    return strdup(faker_words[idx]);
}

char *faker_lorem_sentence(int min_words, int max_words) {
    int count = rand_range(min_words, max_words);
    size_t total_len = 0;
    
    // First pass: calculate length
    // We'll just alloc a safe large buffer or realloc. 
    // Let's guess average word length 6 + space.
    size_t capacity = (size_t)count * 10 + 1;
    char *sentence = malloc(capacity);
    sentence[0] = '\0';
    
    for (int i = 0; i < count; i++) {
        const char *word = faker_words[rand() % faker_words_count];
        size_t word_len = strlen(word);
        
        // Resize if needed
        if (total_len + word_len + 2 > capacity) {
            capacity *= 2;
            char *tmp = realloc(sentence, capacity);
            if (!tmp) {
                free(sentence);
                return NULL;
            }
            sentence = tmp;
        }
        
        if (i == 0) {
            // Capitalize first letter
            char temp[128];
            strncpy(temp, word, 127);
            temp[0] = (char)toupper((unsigned char)temp[0]);
            strcpy(sentence, temp);
            total_len += word_len;
        } else {
            strcat(sentence, " ");
            strcat(sentence, word);
            total_len += 1 + word_len;
        }
    }
    strcat(sentence, ".");
    return sentence;
}

char *faker_lorem_paragraph(int min_sentences, int max_sentences) {
    int count = rand_range(min_sentences, max_sentences);
    char *paragraph = malloc(1);
    paragraph[0] = '\0';
    
    for (int i = 0; i < count; i++) {
        char *sent = faker_lorem_sentence(4, 12);
        if (!sent) { // Handle failure
            free(paragraph);
            return NULL;
        }
        size_t new_len = strlen(paragraph) + strlen(sent) + 2;
        char *tmp = realloc(paragraph, new_len);
        if (!tmp) {
            free(paragraph);
            free(sent);
            return NULL;
        }
        paragraph = tmp;
        strcat(paragraph, sent);
        strcat(paragraph, " ");
        free(sent);
    }
    return paragraph;
}

void faker_create_user(User *user) {
    faker_get_id(user->id, "U");
    
    // Pick gender (0: male, 1: female)
    int gender = rand() % 2;
    const char *first;
    if (gender == 0) {
        first = faker_first_names_male[rand() % faker_first_names_male_count];
    } else {
        first = faker_first_names_female[rand() % faker_first_names_female_count];
    }
    const char *last = faker_last_names[rand() % faker_last_names_count];
    
    snprintf(user->real_name, sizeof(user->real_name), "%s %s", first, last);
    
    // Username: first.last (lowercase)
    snprintf(user->name, sizeof(user->name), "%s.%s", first, last);
    // Lowercase it
    for(char *p = user->name; *p; ++p) *p = (char)tolower((unsigned char)*p);
    
    snprintf(user->email, sizeof(user->email), "%s@example.com", user->name);
    
    // Defaults
    user->is_admin = false;
    user->is_bot = false;
    
    // Avatar hash (random hex)
    char hash[13];
    rand_alphanum(hash, 12); // Actually hex usually, but alphanum ok for placeholder
    // Using gravatar for simplicity as per AGENTS.md
    snprintf(user->image_original, sizeof(user->image_original), "https://www.gravatar.com/avatar/%s?d=identicon", hash);
}

void faker_create_channel(Channel *channel, const char *creator_id) {
    faker_get_id(channel->id, "C");
    
    // Channel name: random-word-random-word
    const char *w1 = faker_words[rand() % faker_words_count];
    const char *w2 = faker_words[rand() % faker_words_count];
    snprintf(channel->name, sizeof(channel->name), "%s-%s", w1, w2);
    
    channel->created = (long)time(NULL) - rand() % (365 * 24 * 3600); // Created within last year
    strcpy(channel->creator, creator_id);
    channel->members = NULL;
    channel->member_count = 0;
}

double faker_get_timestamp(time_t start_time, time_t end_time) {
    double range = difftime(end_time, start_time);
    double offset = ((double)rand() / RAND_MAX) * range;
    return (double)start_time + offset;
}
