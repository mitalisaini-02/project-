#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_LEVEL 16
#define MAX_GENRES 10
#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 100
#define DECAY_RATE 0.9
#define RECOMMENDATION_COUNT 5
#define MAX_USER_NAME 100
#define MAX_PASSWORD 100

// Structure to represent a user
typedef struct User {
    char username[MAX_USER_NAME];
    char password[MAX_PASSWORD];
    int user_type;  // 1 for Visitor, 2 for Staff
    struct User *next;  // Linked list for multiple users
} User;

typedef struct Book
{
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char genre[MAX_GENRES][MAX_TITLE_LENGTH];
    int gen_count;
    int borrow_count;
    time_t last_borrowed;
    char status[MAX_TITLE_LENGTH];
    struct Book *forward[MAX_LEVEL];
} Book;

// Structure to represent the library containing the skip graph
typedef struct Library
{
    Book *header;
    int level;
    int total_books;
    Book *recommendations[MAX_LEVEL];
} Library;

// Structure to represent a max-heap for recommendations
typedef struct MaxHeap
{
    Book **books;
    int size;
    int capacity;
} MaxHeap;

// Heap functions for recommendations
MaxHeap *create_heap(int capacity);
void insert_heap(MaxHeap *heap, Book *book);
Book *extract_max(MaxHeap *heap);
void heapify_down(MaxHeap *heap, int index);
void heapify_up(MaxHeap *heap, int index);

// Function prototypes
Library *create_library();
void add_book(Library *library, const char *title, const char *author, const char genres[MAX_GENRES][MAX_TITLE_LENGTH], const int genre_count, const int borrow_count);
Book *search_book_by_genre_then_title(Library *library, const char *title, const char *genre);
void decay_borrow_counts(Library *library);
void print_books(Library *library);
void free_library(Library *library);
void read_books_from_file(Library *library, const char *filename);

// Heap functions
MaxHeap *create_heap(int capacity)
{
    MaxHeap *heap = (MaxHeap *)malloc(sizeof(MaxHeap));
    heap->books = (Book **)malloc(capacity * sizeof(Book *));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void insert_heap(MaxHeap *heap, Book *book)
{
    if (heap->size < heap->capacity)
    {
        heap->books[heap->size] = book;
        heapify_up(heap, heap->size);
        heap->size++;
    }
    else if (book->borrow_count > heap->books[0]->borrow_count)
    {
        heap->books[0] = book;
        heapify_down(heap, 0);
    }
    else if (book->borrow_count == heap->books[0]->borrow_count)
    {
        // Add to the end if there is a tie
        heap->books[heap->size] = book;
        heap->size++;
        heapify_up(heap, heap->size - 1);
    }
}

Book *extract_max(MaxHeap *heap)
{
    if (heap->size == 0)
        return NULL;
    Book *max = heap->books[0];
    heap->books[0] = heap->books[--heap->size];
    heapify_down(heap, 0);
    return max;
}

void heapify_up(MaxHeap *heap, int index)
{
    while (index > 0)
    {
        int parent = (index - 1) / 2;
        if (heap->books[parent]->borrow_count >= heap->books[index]->borrow_count)
            break;
        Book *temp = heap->books[parent];
        heap->books[parent] = heap->books[index];
        heap->books[index] = temp;
        index = parent;
    }
}

void heapify_down(MaxHeap *heap, int index)
{
    while (2 * index + 1 < heap->size)
    {
        int left = 2 * index + 1;
        int right = left + 1;
        int largest = left;
        if (right < heap->size && heap->books[right]->borrow_count > heap->books[left]->borrow_count)
        {
            largest = right;
        }
        if (heap->books[index]->borrow_count >= heap->books[largest]->borrow_count)
            break;
        Book *temp = heap->books[index];
        heap->books[index] = heap->books[largest];
        heap->books[largest] = temp;
        index = largest;
    }
}

// Modify the recommend_books function to handle ties
void recommend_books(Library *library, const char *genre)
{
    MaxHeap *heap = create_heap(RECOMMENDATION_COUNT);

    printf("Gathering books in genre '%s'...\n", genre); // Debugging: Print genre

    for (Book *current = library->header->forward[0]; current != NULL; current = current->forward[0])
    {
        // Check if the book contains the specified genre
        int genre_found = 0;
        for (int j = 0; j < current->gen_count; j++)
        {
            if (strcmp(current->genre[j], genre) == 0)
            {
                genre_found = 1;
                break;
            }
        }

        // Debugging: Print each book's genre and borrow count
        if (genre_found)
        {
            printf("Found Book: %s (Borrow Count: %d)\n", current->title, current->borrow_count);
            insert_heap(heap, current); // Add to heap only if it matches the genre
        }
    }

    // Display the top recommendations
    printf("\nTop Recommended Books in Genre '%s':\n", genre);
    if (heap->size == 0)
    {
        printf("No recommendations available.\n");
    }
    else
    {
        int last_borrow_count = -1;
        for (int i = 0; i < RECOMMENDATION_COUNT && heap->size > 0; i++)
        {
            Book *recommended = extract_max(heap);
            if (recommended)
            { // Ensure recommended book exists
                if (recommended->borrow_count != last_borrow_count)
                {
                    last_borrow_count = recommended->borrow_count; // Update the last borrow count
                    printf("Title: %s, Author: %s, Borrow Count: %d\n",
                           recommended->title, recommended->author, recommended->borrow_count);
                }
                // To handle additional books with the same borrow count
                while (heap->size > 0 && heap->books[0]->borrow_count == last_borrow_count)
                {
                    recommended = extract_max(heap);
                    printf("Title: %s, Author: %s, Borrow Count: %d\n",
                           recommended->title, recommended->author, recommended->borrow_count);
                }
            }
        }
    }

    free(heap->books);
    free(heap);
}

// Function to create a new library
Library *create_library()
{
    Library *library = (Library *)malloc(sizeof(Library));
    library->header = (Book *)malloc(sizeof(Book));
    strcpy(library->header->title, "");
    library->header->borrow_count = 0;
    library->level = 0;
    library->total_books = 0;

    for (int i = 0; i < MAX_LEVEL; i++)
    {
        library->header->forward[i] = NULL;
    }
    return library;
}

// Linked list to store users (both staff and visitors)
User *user_list = NULL;

// Function to register a new user
void register_user() {
    User *new_user = (User *)malloc(sizeof(User));
    
    printf("Enter username: ");
    fgets(new_user->username, MAX_USER_NAME, stdin);
    new_user->username[strcspn(new_user->username, "\n")] = '\0';  // Remove the newline

    printf("Enter password: ");
    fgets(new_user->password, MAX_PASSWORD, stdin);
    new_user->password[strcspn(new_user->password, "\n")] = '\0';  // Remove the newline

    printf("Registering as:\n");
    printf("1. Visitor\n");
    printf("2. Staff\n");
    printf("Enter your choice: ");
    scanf("%d", &new_user->user_type);
    getchar();  // To consume the newline after scanf
    
    new_user->next = user_list;  // Add to the list of users
    user_list = new_user;

    printf("Registration successful!\n");
}

// Function to login a user
User* login_user() {
    char username[MAX_USER_NAME], password[MAX_PASSWORD];
    
    printf("Enter username: ");
    fgets(username, MAX_USER_NAME, stdin);
    username[strcspn(username, "\n")] = '\0';  // Remove the newline

    printf("Enter password: ");
    fgets(password, MAX_PASSWORD, stdin);
    password[strcspn(password, "\n")] = '\0';  // Remove the newline

    // Traverse the user list and check credentials
    User *current = user_list;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 && strcmp(current->password, password) == 0) {
            printf("Login successful!\n");
            return current;  // Return the user if found
        }
        current = current->next;
    }
    
    printf("Invalid username or password.\n");
    return NULL;  // Return NULL if login failed
}

// Function to add a new book to the library
void add_book(Library *library, const char *title, const char *author, const char genres[MAX_GENRES][MAX_TITLE_LENGTH], int genre_count, int borrow_count)
{
    Book *new_book = (Book *)malloc(sizeof(Book));
    strcpy(new_book->title, title);
    strcpy(new_book->author, author);

    for (int i = 0; i < genre_count; i++)
    {
        strcpy(new_book->genre[i], genres[i]);
    }
    new_book->borrow_count = borrow_count;
    new_book->gen_count = genre_count;
    int level = 0;
    while ((rand() % 2) && (level < MAX_LEVEL - 1))
    {
        level++;
    }
    if (level > library->level)
    {
        library->level = level;
    }
    strcpy(new_book->status, "available");
    Book *current = library->header;
    for (int i = library->level; i >= 0; i--)
    {
        while (current->forward[i] != NULL && strcmp(current->forward[i]->title, title) < 0)
        {
            current = current->forward[i];
        }
        if (i <= level)
        {
            new_book->forward[i] = current->forward[i];
            current->forward[i] = new_book;
        }
    }

    library->total_books++;
}

void read_books_from_file(Library *library, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Error opening file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        char title[MAX_TITLE_LENGTH];
        char author[MAX_AUTHOR_LENGTH];
        char genres[MAX_GENRES][MAX_TITLE_LENGTH];
        int borrow_count;
        int genre_count = 0;

        // Remove newline character from the line
        line[strcspn(line, "\n")] = '\0';

        // Parse title
        char *token = strtok(line, ",");
        if (token == NULL)
            continue;
        strncpy(title, token, MAX_TITLE_LENGTH);

        // Parse author
        token = strtok(NULL, ",");
        if (token == NULL)
            continue;
        strncpy(author, token, MAX_AUTHOR_LENGTH);

        // Parse genres
        while ((token = strtok(NULL, ",")) != NULL)
        {
            // Check if this token is the borrow count (last token)
            if (sscanf(token, "%d", &borrow_count) == 1)
            {
                break;
            }
            // Otherwise, add the genre to the array
            strncpy(genres[genre_count], token, MAX_TITLE_LENGTH);
            genre_count++;
        }

        // Add the book to the library
        add_book(library, title, author, genres, genre_count, borrow_count);
    }

    fclose(file);
    printf("Books loaded successfully from file.\n");
}

Book *search_book_by_genre_then_title(Library *library, const char *title, const char *genre)
{

    Book *current = library->header;

    while (current != NULL)
    {

        for (int j = 0; j < current->gen_count; j++)
        {
            if (strcmp(current->genre[j], genre) == 0)
            {
                if (strncmp(current->title, title, strlen(title)) == 0)
                {
                    return current; //  the book with matching title and genre
                }
            }
        }
        current = current->forward[0]; // Move to next book in the skip list
    }

    return NULL;
}

// Structure to represent a genre shelf (sorted list of books)
typedef struct GenreShelf
{
    char genre[MAX_TITLE_LENGTH]; // Genre name
    Book *head;                   // Head of the list for this genre
    struct GenreShelf *next;      // Link to the next genre shelf
} GenreShelf;

int find_book_position_in_genre(Library *library, const char *title, const char *genre)
{
    Book *current = library->header;
    int position = 1;

    while (current != NULL)
    {

        for (int j = 0; j < current->gen_count; j++)
        {
            if (strcmp(current->genre[j], genre) == 0)
            {

                if (strcmp(current->title, title) == 0)
                {
                    return position;
                }

                position++;
            }
        }

        current = current->forward[0]; // Move to the next book in the skip list
    }

    return -1;
}

// Function to decay borrow counts over time
void decay_borrow_counts(Library *library)
{
    time_t current_time = time(NULL);
    for (Book *current = library->header->forward[0]; current != NULL; current = current->forward[0])
    {
        double decay_factor = difftime(current_time, current->last_borrowed) / (30 * 24 * 60 * 60);
        current->borrow_count = (int)(current->borrow_count * pow(DECAY_RATE, decay_factor));
    }
}

// Function to print all books in the library
void print_books(Library *library)
{
    printf("Books in Library:\n");
    for (Book *current = library->header->forward[0]; current != NULL; current = current->forward[0])
    {
        printf("Title: %s, Author: %s, Genres: ", current->title, current->author);
        for (int j = 0; j < current->gen_count; j++)
        {                                                                                // Changed to use gen_count
            printf("%s%s", current->genre[j], (j < current->gen_count - 1) ? ", " : ""); // Use a conditional to manage commas
        }
        printf(", Borrow Count: %d\n", current->borrow_count);
    }
}

// Function to free memory allocated for the library
void free_library(Library *library)
{
    Book *current = library->header;
    while (current != NULL)
    {
        Book *next = current->forward[0];
        free(current);
        current = next;
    }
    free(library);
}

int main() {
    srand(time(NULL));
    Library *library = create_library();
    int user_type = 0;
    int choice;

    do {
        printf("Welcome to the Library Management System!\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // to consume newline

        if (choice == 1) {  // Register a new user
            register_user();
        } else if (choice == 2) {  // Login an existing user
            User *logged_in_user = login_user();

            if (logged_in_user != NULL) {
                user_type = logged_in_user->user_type;

                if (user_type == 1) { // Visitor options
                    do {
                        printf("\nVisitor Menu:\n");
                        printf("1. Search for a book\n");
                        printf("2. Print all books\n");
                        printf("3. Recommend books\n");
                        printf("4. Borrow a book\n");
                        printf("5. Return a book\n");
                        printf("6. Exit to Main Menu\n");
                        printf("Enter your choice: ");
                        scanf("%d", &choice);
                        getchar(); // to consume newline

                        if (choice == 1) {
                            char title[MAX_TITLE_LENGTH], genre[MAX_TITLE_LENGTH];
                            printf("Enter book title to search: ");
                            fgets(title, MAX_TITLE_LENGTH, stdin);
                            title[strcspn(title, "\n")] = '\0';

                            printf("Enter genre to search: ");
                            fgets(genre, MAX_TITLE_LENGTH, stdin);
                            genre[strcspn(genre, "\n")] = '\0';

                            Book *book = search_book_by_genre_then_title(library, title, genre);
                            if (book) {
                                printf("Book found: Title: %s, Author: %s\n", book->title, book->author);
                            } else {
                                printf("Book not found.\n");
                            }
                        } else if (choice == 2) {
                            print_books(library);
                        } else if (choice == 3) {
                            char genre[MAX_TITLE_LENGTH];
                            printf("Enter genre to search: ");
                            fgets(genre, MAX_TITLE_LENGTH, stdin);
                            genre[strcspn(genre, "\n")] = '\0';
                            recommend_books(library, genre);
                        } else if (choice == 4) { // Borrow a book
                            char title[MAX_TITLE_LENGTH], genre[MAX_TITLE_LENGTH];
                            printf("Enter book title to borrow: ");
                            fgets(title, MAX_TITLE_LENGTH, stdin);
                            title[strcspn(title, "\n")] = '\0';

                            printf("Enter genre: ");
                            fgets(genre, MAX_TITLE_LENGTH, stdin);
                            genre[strcspn(genre, "\n")] = '\0';

                            Book *book = search_book_by_genre_then_title(library, title, genre);
                            if (book && strcmp(book->status, "available") == 0) {
                                strcpy(book->status, "borrowed");
                                book->last_borrowed = time(NULL);
                                book->borrow_count++;
                                printf("You have borrowed: %s by %s\n", book->title, book->author);
                            } else {
                                printf("Book is not available for borrowing.\n");
                            }
                        } else if (choice == 5) { // Return a book
                            char title[MAX_TITLE_LENGTH], genre[MAX_TITLE_LENGTH];
                            printf("Enter book title to return: ");
                            fgets(title, MAX_TITLE_LENGTH, stdin);
                            title[strcspn(title, "\n")] = '\0';

                            printf("Enter genre: ");
                            fgets(genre, MAX_TITLE_LENGTH, stdin);
                            genre[strcspn(genre, "\n")] = '\0';

                            Book *book = search_book_by_genre_then_title(library, title, genre);
                            if (book && strcmp(book->status, "borrowed") == 0) {
                                strcpy(book->status, "available");
                                printf("You have returned: %s by %s\n", book->title, book->author);
                            } else {
                                printf("This book was not borrowed or does not exist in the library.\n");
                            }
                        }
                    } while (choice != 6); // Exit to Main Menu
                } else if (user_type == 2) { // Staff options
                    do {
                        printf("\nStaff Menu:\n");
                        printf("1. Load books from file\n");
                        printf("2. Add a new book\n");
                        printf("3. Search for a book\n");
                        printf("4. Print all books\n");
                        printf("5. Exit to Main Menu\n");
                        printf("6. Position of book\n ");
                        printf("Enter your choice: ");
                        scanf("%d", &choice);
                        getchar(); // to consume newline

                        if (choice == 1) {
                            char filename[100];
                            printf("Enter the filename to load books from: ");
                            fgets(filename, sizeof(filename), stdin);
                            filename[strcspn(filename, "\n")] = '\0';
                            read_books_from_file(library, filename);
                        } else if (choice == 2) {
                            char title[MAX_TITLE_LENGTH], author[MAX_AUTHOR_LENGTH];
                            char genres[MAX_GENRES][MAX_TITLE_LENGTH];
                            int genre_count;
                            int borrow_count = 0;
                            printf("Enter book title: ");
                            fgets(title, MAX_TITLE_LENGTH, stdin);
                            title[strcspn(title, "\n")] = '\0';

                            printf("Enter author name: ");
                            fgets(author, MAX_AUTHOR_LENGTH, stdin);
                            author[strcspn(author, "\n")] = '\0';

                            printf("Enter number of genres: ");
                            scanf("%d", &genre_count);
                            getchar();
                            for (int i = 0; i < genre_count; i++) {
                                printf("Enter genre %d: ", i + 1);
                                fgets(genres[i], MAX_TITLE_LENGTH, stdin);
                                genres[i][strcspn(genres[i], "\n")] = '\0';
                            }
                            add_book(library, title, author, genres, genre_count, borrow_count);
                            printf("Book added successfully.\n");
                        } else if (choice == 3) {
                            char title[MAX_TITLE_LENGTH], genre[MAX_TITLE_LENGTH];
                            printf("Enter book title to search: ");
                            fgets(title, MAX_TITLE_LENGTH, stdin);
                            title[strcspn(title, "\n")] = '\0';

                            printf("Enter genre to search: ");
                            fgets(genre, MAX_TITLE_LENGTH, stdin);
                            genre[strcspn(genre, "\n")] = '\0';

                            Book *book = search_book_by_genre_then_title(library, title, genre);
                            if (book) {
                                printf("Book found: Title: %s, Author: %s\n", book->title, book->author);
                            } else {
                                printf("Book not found.\n");
                            }
                        } else if (choice == 4) {
                            print_books(library);
                        } else if (choice == 6) { // New option to find book position in genre
                            char title[MAX_TITLE_LENGTH], genre[MAX_TITLE_LENGTH];
                            printf("Enter book title to search for position: ");
                            fgets(title, MAX_TITLE_LENGTH, stdin);
                            title[strcspn(title, "\n")] = '\0';

                            printf("Enter genre to search: ");
                            fgets(genre, MAX_TITLE_LENGTH, stdin);
                            genre[strcspn(genre, "\n")] = '\0';

                            int position = find_book_position_in_genre(library, title, genre);
                            if (position != -1) {
                                printf("The book '%s' is located at position %d on the '%s' shelf.\n", title, position, genre);
                            } else {
                                printf("Book not found in the '%s' genre shelf.\n", genre);
                            }
                        }

                    } while (choice != 5); // Exit to Main Menu
                }
            }
        } 
    }
    while (user_type != 3); // Exit the program
    free_library(library);
    return 0;
}
