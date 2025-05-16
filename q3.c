#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_WORD_LEN 100
#define HASH_SIZE 10000

char *delimiters = "-',.!?:";

// Word count object in the entry table.
typedef struct word_count {
    char *word;
    int count;
    struct word_count *next; // Next item in table.
} word_count;


// Hash function to make key of hash table.
unsigned int hash(const char *str) {
    unsigned int hash = 0;
    int c;
    while ((c = *str++))
        hash = (hash << 5) + hash + tolower(c);
    return hash % HASH_SIZE;
}

// Normalize(uncapitalize) the word.
char *normalize_word(const char *input) {
    // Initialize memory
    char *normalized = malloc(strlen(input) + 1);
    if (!normalized) { // Check for malloc failure
        perror("malloc failed in normalize_word");
        exit(EXIT_FAILURE);
    }
    int j = 0;
    // Add character to the array until we find ending char.
    for (int i = 0; input[i]; i++) {
        if (isalnum((unsigned char)input[i])) {
            normalized[j++] = tolower((unsigned char)input[i]);
        }
    }
    // ending char
    normalized[j] = '\0';

    return normalized;
}

// Insert a word to the hash table or incerment. If word exist in the hash table, increment. If not, insert.
word_count *insert_or_increment(word_count **table, const char *word) {
    unsigned int h = hash(word);
    word_count *curr = table[h];
    while (curr) {
        if (strcmp(curr->word, word) == 0) {
            curr->count++;
            return curr;
        }
        curr = curr->next; // Advance to the next node in the linked list
    }

    // New word, make a new entry.
    word_count *new_entry = malloc(sizeof(word_count));
    if (!new_entry) { // Check for malloc failure
        perror("malloc failed for new_entry");
        exit(EXIT_FAILURE);
    }
    new_entry->word = strdup(word);
    if (!new_entry->word) { // Check for strdup failure
         perror("strdup failed for new_entry word");
         free(new_entry); // Clean up the entry struct
         exit(EXIT_FAILURE);
    }
    new_entry->count = 1;
    new_entry->next = table[h]; // Insert at the head of the list
    table[h] = new_entry;
    return new_entry;
}


// Compare two words counts.
int compare_word_counts(const void *a, const void *b) {
    word_count *wa = *(word_count **)a;
    word_count *wb = *(word_count **)b;
    return wb->count - wa->count; // descending
}

// Function to find the most frequent words
char **find_frequent_words(const char *path, int n) {
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Error opening file"); // Print error message
        return NULL; // Return NULL on failure
    }

    word_count *table[HASH_SIZE] = {NULL};
    char buffer[MAX_WORD_LEN];

    // Use a temporary buffer for strtok to avoid modifying the original buffer
    char temp_buffer[MAX_WORD_LEN];

    while (fscanf(file, "%99s", buffer) == 1) { //Read a word up to 99.
        // Copy buffer to temp_buffer because strtok modifies the string
        strncpy(temp_buffer, buffer, MAX_WORD_LEN - 1);
        temp_buffer[MAX_WORD_LEN - 1] = '\0'; // Ensure null termination

        // Split by delimiters.
        char *token = strtok(temp_buffer, delimiters);
        while (token != NULL) {
            char *word = normalize_word(token); // Normalize the word
            if (strlen(word) > 0) { // If the length of the word is greater than 0, insert or increment
                insert_or_increment(table, word);
            }
            free(word); // Free memory allocated by normalize_word
            token = strtok(NULL, delimiters); // Next word;
        }
    }
    fclose(file);

    // Flatten hash table into array
    int total = 0;
    // First pass to count total unique words
    for (int i = 0; i < HASH_SIZE; ++i) {
        word_count *curr = table[i];
        while (curr) {
            total++;
            curr = curr->next;
        }
    }

    // Allocate array based on actual total count
    word_count **all_words = malloc(total * sizeof(word_count *));
     if (!all_words) { // Check for malloc failure
        perror("malloc failed for all_words");
        // Need to clean up the hash table before exiting
        for (int i = 0; i < HASH_SIZE; i++) {
            word_count *curr = table[i];
            while (curr) {
                word_count *tmp = curr;
                curr = curr->next;
                free(tmp->word);
                free(tmp);
            }
        }
        return NULL; // Return NULL on failure
    }

    // Second pass to populate the array
    int current_index = 0;
    for (int i = 0; i < HASH_SIZE; ++i) {
        word_count *curr = table[i];
        while (curr) {
            all_words[current_index++] = curr;
            curr = curr->next; // Advance to the next node
        }
    }


    qsort(all_words, total, sizeof(word_count *), compare_word_counts);

    int result_size = (n < total) ? n : total; // Pick the minimum of n and total.
    char **result = malloc(result_size * sizeof(char *));
    if (!result) { // Check for malloc failure
        perror("malloc failed for result");
        // Need to clean up all_words array and hash table before exiting
        free(all_words);
         for (int i = 0; i < HASH_SIZE; i++) {
            word_count *curr = table[i];
            while (curr) {
                word_count *tmp = curr;
                curr = curr->next;
                free(tmp->word);
                free(tmp);
            }
        }
        return NULL; // Return NULL on failure
    }

    for (int i = 0; i < result_size; i++) {
        result[i] = strdup(all_words[i]->word);
        if (!result[i]) { // Check for strdup failure
            perror("strdup failed for result word");
            // Clean up already allocated result strings and result array
            for (int j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
             // Need to clean up all_words array and hash table before exiting
            free(all_words);
             for (int k = 0; k < HASH_SIZE; k++) {
                word_count *curr = table[k];
                while (curr) {
                    word_count *tmp = curr;
                    curr = curr->next;
                    free(tmp->word);
                    free(tmp);
                }
            }
            return NULL; // Return NULL on failure
        }
    }

    // Cleanup hash table (nodes and words within them)
    for (int i = 0; i < HASH_SIZE; i++) {
        word_count *curr = table[i];
        while (curr) {
            word_count *tmp = curr;
            curr = curr->next;
            free(tmp->word);
            free(tmp);
        }
    }
    free(all_words); // Free the array of pointers

    return result; // Return the array of top words
}


// Run
int main(int argc, char** argv) {
    if (argc < 3) { // Need filename and top_count
        fprintf(stderr, "Usage: %s <filename> <top_count>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int top_count = atoi(argv[2]); // Get top_count from second argument

    if (top_count <= 0) {
        fprintf(stderr, "Error: top_count must be a positive integer.\n");
        return 1;
    }

    char **top_words = find_frequent_words(filename, top_count);

    if (top_words == NULL) { // Check if find_frequent_words failed
        fprintf(stderr, "Could not find frequent words.\n");
        return 1;
    }

    // Print the top words
    printf("Top %d frequent words in %s:\n", top_count, filename);
    for (int i = 0; i < top_count; i++) {
        if (top_words[i]) { // Check if the word string is not NULL
            printf("%s\n", top_words[i]);
            free(top_words[i]); // Free each word string
        }
    }

    free(top_words); // Free the array of pointers
    return 0;
}
