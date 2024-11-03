#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LEVEL 10
#define MAX_GENRES 5
#define MAX_TITLE_LENGTH 100
#define MAX_GROUPS 26 // A to Z
#define MAX_RECOMMENDATIONS 5 // Number of recommendations to show
#include <time.h>

#define MAX_LEVEL 10
#define MAX_GENRES 5
#define MAX_TITLE_LENGTH 100
#define MAX_GROUPS 26 // A to Z
#define MAX_RECOMMENDATIONS 5 // Number of recommendations to show

typedef struct Book {
    char title[MAX_TITLE_LENGTH];
    char author[MAX_TITLE_LENGTH];
    char genre[MAX_GENRES][MAX_TITLE_LENGTH];
    int borrow_count;
    time_t last_borrowed;
    struct Book *forward[MAX_LEVEL];
    struct Book *next; // Pointer to the next book in the alphabetical group
    char title[MAX_TITLE_LENGTH];
    char author[MAX_TITLE_LENGTH];
    char genre[MAX_GENRES][MAX_TITLE_LENGTH];
    int borrow_count;
    time_t last_borrowed;
    struct Book *forward[MAX_LEVEL];
    struct Book *next; // Pointer to the next book in the alphabetical group
} Book;

typedef struct Library {
    Book *genre_heads[MAX_GENRES];  // Array for different genres
    Book *title_groups[MAX_GROUPS];  // Grouping books alphabetically
    int level;                       // Maximum level in the skip graph
    int total_books;                 // Total number of books in the library
} Library;

// Initialize the library
void init_library(Library *library) {
    for (int i = 0; i < MAX_GENRES; i++) {
        library->genre_heads[i] = NULL;
    }
    for (int i = 0; i < MAX_GROUPS; i++) {
        library->title_groups[i] = NULL;
    }
    library->level = 0;
    library->total_books = 0;
}

// Function to determine the group index for a title
int get_group_index(const char *title) {
    char first_char = title[0];
    if (first_char >= 'A' && first_char <= 'Z') {
        return first_char - 'A'; // A=0, B=1, ..., Z=25
    } else if (first_char >= 'a' && first_char <= 'z') {
        return first_char - 'a'; // a=0, b=1, ..., z=25
    }
    return -1; // Invalid starting character
}

// Function to determine the genre group index
int get_genre_index(const char *genre) {
    const char *genres[] = {
        "Fiction", "Non-Fiction", "Fantasy", "Science Fiction",
        "Biography", "History", "Mystery", "Romance", "Horror", "Adventure"
    };
    for (int i = 0; i < 10; i++) {
        if (strcmp(genre, genres[i]) == 0) return i;
    }
    return -1;  // Invalid genre
}

// Function to add a book to the library
void add_book(Library *library, const char *title, const char *author, const char genres[MAX_GENRES][MAX_TITLE_LENGTH], int genre_count) {
    // Create a new book
    Book *new_book = (Book *)malloc(sizeof(Book));
    strcpy(new_book->title, title); 
    strcpy(new_book->author, author);
    for (int i = 0; i < genre_count; i++) {
        strcpy(new_book->genre[i], genres[i]);
    }
    new_book->borrow_count = 0; // Initialize borrow count to zero
    new_book->last_borrowed = 0;
    new_book->next = NULL; // Initialize the next pointer

    // Determine the skip level for the new book
    int level = 0;
    while ((rand() % 2) && (level < MAX_LEVEL - 1)) {
        level++;
    }
    if (level > library->level) {
        library->level = level;
    }

    // Insert the new book in the appropriate genre
    int genre_index = get_genre_index(genres[0]);
    if (genre_index < 0) {
        printf("Error: Invalid genre '%s'. Adding to library anyway.\n", genres[0]);
        genre_index = library->total_books % MAX_GENRES; // Assign to a random genre if invalid
    }

    // Insert the new book in the skip graph of the selected genre
    Book *current = library->genre_heads[genre_index];
    if (!current) {
        library->genre_heads[genre_index] = new_book;
        for (int i = 0; i < MAX_LEVEL; i++) {
            new_book->forward[i] = NULL;
        }
    } else {
        // Navigate to insert the new book
        Book *update[MAX_LEVEL];  
        for (int i = library->level; i >= 0; i--) {
            while (current->forward[i] != NULL && strcmp(current->forward[i]->title, title) < 0) {
                current = current->forward[i];
            }
            update[i] = current;  
        }

        // Insert the new book at appropriate levels
        for (int i = 0; i <= level; i++) {
            if (i > library->level) break; 
            new_book->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = new_book;
        }
    }

    // Update library's total book count
    library->total_books++;

    // Add the book to the alphabetical group
    int group_index = get_group_index(title);
    if (group_index >= 0 && group_index < MAX_GROUPS) {
        // Insert the book into the alphabetical linked list
        if (library->title_groups[group_index] == NULL) {
            library->title_groups[group_index] = new_book;
        } else {
            Book *group_current = library->title_groups[group_index];
            Book *prev = NULL;
            while (group_current != NULL && strcmp(group_current->title, title) < 0) {
                prev = group_current;
                group_current = group_current->next;
            }
            // Insert the new book in the appropriate place
            new_book->next = group_current;
            if (prev == NULL) {
                library->title_groups[group_index] = new_book; // Insert at the head
            } else {
                prev->next = new_book; // Insert in the middle or end
            }
        }
    } else {
        free(new_book); // Free if the group index is invalid
    }
}

// Function to load books from a file
void load_books_from_file(Library *library, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file %s\n", filename);
        return;
    }

    char line[256]; // Buffer to hold each line from the file
    while (fgets(line, sizeof(line), file)) {
        char title[MAX_TITLE_LENGTH];
        char author[MAX_TITLE_LENGTH];
        char genres[MAX_GENRES][MAX_TITLE_LENGTH];
        int genre_count = 0;

        // Parse the line
        char *token = strtok(line, ",");
        if (token) {
            strcpy(title, token);
            token = strtok(NULL, ",");
            if (token) {
                strcpy(author, token);
                token = strtok(NULL, ","); // Read the first genre
                while (token && genre_count < MAX_GENRES) {
                    // Trim whitespace
                    while (*token == ' ') token++;
                    strcpy(genres[genre_count++], token);
                    token = strtok(NULL, ",");  // Continue reading genres
                }
                add_book(library, title, author, genres, genre_count); // Add the book to the library
            }
        }
    }

    fclose(file); // Close the file after reading
}

// Function to search for a book by title within a specific genre
void search_book_in_genre(const Library *library, const char *title, const char *genre) {
    int genre_index = get_genre_index(genre);
    if (genre_index < 0) {
        printf("Invalid genre '%s'.\n", genre);
        return;
    }

    Book *current = library->genre_heads[genre_index];
    if (!current) {
        printf("No books available in the genre '%s'.\n", genre);
        return;
    }

    while (current != NULL) {
        int comparison = strcmp(current->title, title);
        if (comparison == 0) {
            printf("Found: Title: %s, Author: %s, Genre: %s\n", current->title, current->author, current->genre[0]);
            return; // Book found
        } else if (comparison > 0) {
            break; // Since books are sorted, if current title is greater, we can stop
        }
        current = current->forward[0]; // Move to the next book
    }
    printf("Book with title '%s' not found in genre '%s'.\n", title, genre);
}

// Function to search for a book by title, using grouping
void search_book(const Library *library, const char *title) {
    int group_index = get_group_index(title);
    if (group_index < 0 || group_index >= MAX_GROUPS) {
        printf("No group found for title '%s'.\n", title);
        return;
    }

    // Search in the title group
    Book *current = library->title_groups[group_index];
    while (current != NULL) {
        int comparison = strcmp(current->title, title);
        if (comparison == 0) {
            printf("Found in group: Title: %s, Author: %s\n", current->title, current->author);
            current->borrow_count++; // Increment borrow count
            current->last_borrowed = time(NULL); // Update last borrowed time
            return; // Book found
        } else if (comparison > 0) {
            break; // No need to continue if current title is greater
        }
        current = current->next; // Move to the next book in the group
    }
    printf("Book with title '%s' not found in group.\n", title);
}

// Function to recommend popular books based on borrow count
void recommend_books(const Library *library) {
    // Collect all books into an array for sorting
    Book *all_books[library->total_books];
    int count = 0;

    for (int i = 0; i < MAX_GENRES; i++) {
        Book *current = library->genre_heads[i];
        while (current != NULL) {
            all_books[count++] = current;
            current = current->forward[0]; // Move to the next book
        }
    }

    printf("Total books collected for recommendations: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("Book: %s, Borrow Count: %d\n", all_books[i]->title, all_books[i]->borrow_count);
    }

    // Sort the array based on borrow count (simple bubble sort for demonstration)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (all_books[j]->borrow_count < all_books[j + 1]->borrow_count) {
                // Swap
                Book *temp = all_books[j];
                all_books[j] = all_books[j + 1];
                all_books[j + 1] = temp;
            }
        }
    }

    // Print recommendations
    printf("\nRecommended Books (Top %d):\n", MAX_RECOMMENDATIONS);
    for (int i = 0; i < MAX_RECOMMENDATIONS && i < count; i++) {
        printf("Title: %s, Author: %s, Borrow Count: %d\n", all_books[i]->title, all_books[i]->author, all_books[i]->borrow_count);
    }
}

// Function to print the books in the library
void print_books(const Library *library) {
    const char *genre_names[MAX_GENRES] = {"Fiction", "Non-Fiction", "Fantasy", "Science Fiction", "Biography", "History", "Mystery", "Romance", "Horror", "Adventure"};
    
    for (int i = 0; i < MAX_GENRES; i++) {
        printf("Books in genre %s:\n", genre_names[i]);
        Book *current = library->genre_heads[i];
        while (current != NULL) {
            printf("Title: %s, Author: %s\n", current->title, current->author);
            current = current->forward[0];  // Move to the next book in the skip list
        }
        if (!library->genre_heads[i]) {
            printf("No books available in this genre.\n");
        }
    }
}

// Function to free allocated memory
void free_library(Library *library) {
    for (int i = 0; i < MAX_GENRES; i++) {
        Book *current = library->genre_heads[i];
        while (current != NULL) {
            Book *to_delete = current;
            current = current->forward[0];  // Move to the next book
            free(to_delete);  // Free the memory of the book
        }
    }
    for (int i = 0; i < MAX_GROUPS; i++) {
        Book *current = library->title_groups[i];
        while (current != NULL) {
            Book *to_delete = current;
            current = current->next;  // Move to the next book
            free(to_delete);  // Free the memory of the book
        }
    }
}

// Main function
int main() {
    Library library;
    init_library(&library);

    // Load books from the file
    load_books_from_file(&library, "data.txt");

    // Main menu
    int choice;
    do {
        printf("\nMenu:\n");
        printf("1. Print all books\n");
        printf("2. Search for a book by title\n");
        printf("3. Search for a book by title in a specific genre\n");
        printf("4. Get recommendations\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline after scanf

        switch (choice) {
            case 1:
                print_books(&library);
                break;
            case 2: {
                char search_title[MAX_TITLE_LENGTH];
                printf("Enter title to search: ");
                fgets(search_title, sizeof(search_title), stdin);
                search_title[strcspn(search_title, "\n")] = 0; // Remove newline
                search_book(&library, search_title); // Searching in the alphabetical grouping
                break;
            }
            case 3: {
                char search_title[MAX_TITLE_LENGTH];
                char search_genre[MAX_TITLE_LENGTH];
                printf("Enter title to search: ");
                fgets(search_title, sizeof(search_title), stdin);
                search_title[strcspn(search_title, "\n")] = 0; // Remove newline

                printf("Enter genre to search: ");
                fgets(search_genre, sizeof(search_genre), stdin);
                search_genre[strcspn(search_genre, "\n")] = 0; // Remove newline

                search_book_in_genre(&library, search_title, search_genre);
                break;
            }
            case 4:
                recommend_books(&library);
                break;
            case 0:
                free_library(&library); // Free allocated memory before exiting
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);

    return 0;
}