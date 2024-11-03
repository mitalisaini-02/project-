#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 50
#define MAX_GENRES 10
#define TOTAL_BOOKS 100

typedef struct {
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char genre[MAX_TITLE_LENGTH];
    int borrow_count;
} Book;

void generate_random_title(char *title) {
    const char *adjectives[] = {"The", "A", "An", "Mystery of", "Secrets of", "Journey to", "Chronicles of"};
    const char *nouns[] = {"Lost City", "Great Adventure", "Hidden Treasure", "Ancient Secrets", "Wanderers", "Forgotten World", "Endless Dream"};
    int adjective_count = sizeof(adjectives) / sizeof(adjectives[0]);
    int noun_count = sizeof(nouns) / sizeof(nouns[0]);
    const char *adjective = adjectives[rand() % adjective_count];
    const char *noun = nouns[rand() % noun_count];
    snprintf(title, MAX_TITLE_LENGTH, "%s %s", adjective, noun);
}

void generate_random_author(char *author) {
    const char *first_names[] = {"John", "Jane", "Alex", "Emily", "Michael", "Sarah", "Robert", "Jessica", "David", "Laura"};
    const char *last_names[] = {"Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis", "Martinez", "Lopez"};
    int first_count = sizeof(first_names) / sizeof(first_names[0]);
    int last_count = sizeof(last_names) / sizeof(last_names[0]);
    const char *first_name = first_names[rand() % first_count];
    const char *last_name = last_names[rand() % last_count];
    snprintf(author, MAX_AUTHOR_LENGTH, "%s %s", first_name, last_name);
}

void generate_random_genre(char *genre) {
    const char *genres[] = {"Fiction", "Non-Fiction", "Fantasy", "Science Fiction", "Biography", "History", "Mystery", "Romance", "Horror", "Adventure"};
    int genre_count = sizeof(genres) / sizeof(genres[0]);
    strcpy(genre, genres[rand() % genre_count]);
}

void generate_books(Book library[], int total_books) {
    for (int i = 0; i < total_books; i++) {
        generate_random_title(library[i].title);
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
        fprintf(file,"%s,%s,%s,%d\n",
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