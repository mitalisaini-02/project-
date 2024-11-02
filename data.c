#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 50
#define MAX_GENRES 10
#define TOTAL_BOOKS 5000
#define MAX_UNIQUE_TITLES 10000 // Increased limit for unique titles

typedef struct {
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char genre[MAX_TITLE_LENGTH];
    int borrow_count;
} Book;

char unique_titles[MAX_UNIQUE_TITLES][MAX_TITLE_LENGTH]; // Array to hold unique titles
int unique_title_count = 0; // Counter for unique titles

int is_title_unique(const char *title) {
    for (int i = 0; i < unique_title_count; i++) {
        if (strcmp(unique_titles[i], title) == 0) {
            return 0; // Title is not unique
        }
    }
    return 1; // Title is unique
}

void generate_random_title(char *title) {
    const char *adjectives[] = {"The", "A", "An", "Mystery of", "Secrets of", "Journey to", "Chronicles of", "Tales of", "Legend of", "Rise of"};
    const char *nouns[] = {"Lost City", "Great Adventure", "Hidden Treasure", "Ancient Secrets", "Wanderers", "Forgotten World", "Endless Dream", "Silent Forest", "Moonlit Path", "Shattered Realm"};
    const char *subtitles[] = {"the Enchanted Lands", "the Dark Waters", "the Eternal Flame", "the Last Stand", "the Hidden Valley", "Destiny’s Path", "the Forbidden Kingdom", "Beyond the Stars", "of Broken Promises", "the Final Hour"};
    
    int adjective_count = sizeof(adjectives) / sizeof(adjectives[0]);
    int noun_count = sizeof(nouns) / sizeof(nouns[0]);
    int subtitle_count = sizeof(subtitles) / sizeof(subtitles[0]);
    
    const char *adjective = adjectives[rand() % adjective_count];
    const char *noun = nouns[rand() % noun_count];
    const char *subtitle = subtitles[rand() % subtitle_count];
    
    snprintf(title, MAX_TITLE_LENGTH, "%s %s: %s", adjective, noun, subtitle);
}

void generate_random_author(char *author) {
    const char *first_names[] = {"John", "Jane", "Alex", "Emily", "Michael", "Sarah", "Robert", "Jessica", "David", "Laura", "Chris", "Olivia", "Ethan", "Sophia", "Daniel", "Isabella"};
    const char *middle_names[] = {"A.", "B.", "C.", "D.", "E.", "F.", "G.", "H.", "I.", "J."};
    const char *last_names[] = {"Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis", "Martinez", "Lopez", "Clark", "Lee", "Walker", "Perez", "Hall"};
    
    int first_count = sizeof(first_names) / sizeof(first_names[0]);
    int middle_count = sizeof(middle_names) / sizeof(middle_names[0]);
    int last_count = sizeof(last_names) / sizeof(last_names[0]);
    
    const char *first_name = first_names[rand() % first_count];
    const char *middle_name = middle_names[rand() % middle_count];
    const char *last_name = last_names[rand() % last_count];
    
    snprintf(author, MAX_AUTHOR_LENGTH, "%s %s %s", first_name, middle_name, last_name);
}

void generate_random_genre(char *genre) {
    const char *genres[] = {"Fiction", "Non-Fiction", "Fantasy", "Science Fiction", "Biography", "History", "Mystery", "Romance", "Horror", "Adventure"};
    int genre_count = sizeof(genres) / sizeof(genres[0]);
    strcpy(genre, genres[rand() % genre_count]);
}

void generate_books(Book library[], int total_books) {
    for (int i = 0; i < total_books; i++) {
        char title[MAX_TITLE_LENGTH];
        
        // Keep generating a new title until we find a unique one
        do {
            generate_random_title(title);
        } while (!is_title_unique(title) && unique_title_count < MAX_UNIQUE_TITLES);
        
        // Store the unique title in the array
        if (unique_title_count < MAX_UNIQUE_TITLES) {
            strcpy(unique_titles[unique_title_count++], title);
        }
        
        strcpy(library[i].title, title);
        generate_random_author(library[i].author);
        generate_random_genre(library[i].genre);
        library[i].borrow_count = rand() % 50; // Random borrow count for demonstration
    }
}

void print_books_to_file(Book library[], int total_books, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not open file %s for writing.\n", filename);
        return;
    }

    for (int i = 0; i < total_books; i++) {
        fprintf(file, "%s, %s, %s, %d\n",
                library[i].title, library[i].author, library[i].genre, library[i].borrow_count);
    }

    fclose(file);
    printf("Books successfully written to %s.\n", filename);
}

int main() {
    srand(time(NULL)); // Seed random number generator

    Book library[TOTAL_BOOKS];
    generate_books(library, TOTAL_BOOKS); // Generate dataset

    // Write all books to "data.txt"
    print_books_to_file(library, TOTAL_BOOKS, "data.txt");

    return 0;
}