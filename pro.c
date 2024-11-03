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


// Structure to represent a book in the library
typedef struct Book {
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
typedef struct Library {
    Book *header;                          
    int level;                             
    int total_books;              
    Book *recommendations[MAX_LEVEL];           
} Library;

// Structure to represent a max-heap for recommendations
typedef struct MaxHeap {
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
void add_book(Library *library, const char *title, const char *author, const char genres[MAX_GENRES][MAX_TITLE_LENGTH], const int genre_count,const int borrow_count);
Book *search_book(Library *library, const char *title, const char *genre);
void decay_borrow_counts(Library *library);
void print_books(Library *library);
void free_library(Library *library);
void read_books_from_file(Library *library, const char *filename);



// Heap functions
MaxHeap *create_heap(int capacity) {
    MaxHeap *heap = (MaxHeap *)malloc(sizeof(MaxHeap));
    heap->books = (Book **)malloc(capacity * sizeof(Book *));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void insert_heap(MaxHeap *heap, Book *book) {
    if (heap->size < heap->capacity) {
        heap->books[heap->size] = book;
        heapify_up(heap, heap->size);
        heap->size++;
    } else if (book->borrow_count > heap->books[0]->borrow_count) {
        heap->books[0] = book;
        heapify_down(heap, 0);
    } else if (book->borrow_count == heap->books[0]->borrow_count) {
        // Add to the end if there is a tie
        heap->books[heap->size] = book;
        heap->size++;
        heapify_up(heap, heap->size - 1);
    }
}

Book *extract_max(MaxHeap *heap) {
    if (heap->size == 0) return NULL;
    Book *max = heap->books[0];
    heap->books[0] = heap->books[--heap->size];
    heapify_down(heap, 0);
    return max;
}

void heapify_up(MaxHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap->books[parent]->borrow_count >= heap->books[index]->borrow_count) break;
        Book *temp = heap->books[parent];
        heap->books[parent] = heap->books[index];
        heap->books[index] = temp;
        index = parent;
    }
}

void heapify_down(MaxHeap *heap, int index) {
    while (2 * index + 1 < heap->size) {
        int left = 2 * index + 1;
        int right = left + 1;
        int largest = left;
        if (right < heap->size && heap->books[right]->borrow_count > heap->books[left]->borrow_count) {
            largest = right;
        }
        if (heap->books[index]->borrow_count >= heap->books[largest]->borrow_count) break;
        Book *temp = heap->books[index];
        heap->books[index] = heap->books[largest];
        heap->books[largest] = temp;
        index = largest;
    }
}

// Modify the recommend_books function to handle ties
void recommend_books(Library *library, const char *genre) {
    MaxHeap *heap = create_heap(RECOMMENDATION_COUNT);

    printf("Gathering books in genre '%s'...\n", genre);  // Debugging: Print genre

    for (Book *current = library->header->forward[0]; current != NULL; current = current->forward[0]) {
        // Check if the book contains the specified genre
        int genre_found = 0;
        for (int j = 0; j < current->gen_count; j++) {
            if (strcmp(current->genre[j], genre) == 0) {
                genre_found = 1;
                break;
            }
        }

        // Debugging: Print each book's genre and borrow count
        if (genre_found) {
            printf("Found Book: %s (Borrow Count: %d)\n", current->title, current->borrow_count);
            insert_heap(heap, current);  // Add to heap only if it matches the genre
        }
    }

    // Display the top recommendations
    printf("\nTop Recommended Books in Genre '%s':\n", genre);
    if (heap->size == 0) {
        printf("No recommendations available.\n");
    } else {
        int last_borrow_count = -1;
        for (int i = 0; i < RECOMMENDATION_COUNT && heap->size > 0; i++) {
            Book *recommended = extract_max(heap);
            if (recommended) {  // Ensure recommended book exists
                if (recommended->borrow_count != last_borrow_count) {
                    last_borrow_count = recommended->borrow_count;  // Update the last borrow count
                    printf("Title: %s, Author: %s, Borrow Count: %d\n",
                        recommended->title, recommended->author, recommended->borrow_count);
                }
                // To handle additional books with the same borrow count
                while (heap->size > 0 && heap->books[0]->borrow_count == last_borrow_count) {
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
Library *create_library() {
    Library *library = (Library *)malloc(sizeof(Library));
    library->header = (Book *)malloc(sizeof(Book));
    strcpy(library->header->title, ""); 
    library->header->borrow_count = 0; 
    library->level = 0;                 
    library->total_books = 0;           

    for (int i = 0; i < MAX_LEVEL; i++) {
        library->header->forward[i] = NULL;
    }
    return library; 
}

// Function to add a new book to the library
void add_book(Library *library, const char *title, const char *author, const char genres[MAX_GENRES][MAX_TITLE_LENGTH], int genre_count, int borrow_count ) {
    Book *new_book = (Book *)malloc(sizeof(Book)); 
    strcpy(new_book->title, title);
    strcpy(new_book->author, author);

    for (int i = 0; i < genre_count; i++) {
        strcpy(new_book->genre[i], genres[i]);
    }
    new_book->borrow_count=borrow_count;
    new_book->gen_count = genre_count;
    int level = 0;
    while ((rand() % 2) && (level < MAX_LEVEL - 1)) {
        level++;
    }
    if (level > library->level) {
        library->level = level;
    }
    strcpy(new_book->status,"available");
    Book *current = library->header;
    for (int i = library->level; i >= 0; i--) {
        while (current->forward[i] != NULL && strcmp(current->forward[i]->title, title) < 0) {
            current = current->forward[i];
        }
        if (i <= level) {
            new_book->forward[i] = current->forward[i];
            current->forward[i] = new_book;
        }
    }

    library->total_books++;
}

void read_books_from_file(Library *library, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char title[MAX_TITLE_LENGTH];
        char author[MAX_AUTHOR_LENGTH];
        char genres[MAX_GENRES][MAX_TITLE_LENGTH];
        int borrow_count;
        int genre_count = 0;

        // Remove newline character from the line
        line[strcspn(line, "\n")] = '\0';

        // Parse title
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        strncpy(title, token, MAX_TITLE_LENGTH);

        // Parse author
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(author, token, MAX_AUTHOR_LENGTH);

        // Parse genres
        while ((token = strtok(NULL, ",")) != NULL) {
            // Check if this token is the borrow count (last token)
            if (sscanf(token, "%d", &borrow_count) == 1) {
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
// Enhanced search function
Book *search_book(Library *library, const char *title, const char *genre)
{
    Book *current = library->header;

    // Traverse through the levels of the skip list, starting from the highest
    for (int i = library->level; i >= 0; i--)
    {
        while (current->forward[i] != NULL && strcmp(current->forward[i]->title, title) < 0)
        {
            current = current->forward[i];
        }
    }

    // Move to the next node at level 0 where the title could be located
    current = current->forward[0];

    // Check for gaps in the title search
    int title_length = strlen(title);
    int gap_found = 0;

    // Loop through the current node to find matches with gaps
    while (current != NULL)
    {
        if (strncmp(current->title, title, title_length) == 0)
        {
            // If the current title matches the search title with allowed gaps
            for (int j = 0; j < current->gen_count; j++)
            {
                if (strcmp(current->genre[j], genre) == 0)
                {
                    return current; // Found book with matching title and genre
                }
            }
        }
        // Check for gaps in the title
        if (current->title[title_length] == ' ' || current->title[title_length] == '\0')
        {
            gap_found = 1;
            for (int j = 0; j < current->gen_count; j++)
            {
                if (strcmp(current->genre[j], genre) == 0)
                {
                    return current; // Found book with matching genre after a gap
                }
            }
        }
        current = current->forward[0];
    }

    // Return NULL if no match found
    return NULL;
}

// Function to decay borrow counts over time
void decay_borrow_counts(Library *library) {
    time_t current_time = time(NULL);
    for (Book *current = library->header->forward[0]; current != NULL; current = current->forward[0]) {
        double decay_factor = difftime(current_time, current->last_borrowed) / (30 * 24 * 60 * 60);
        current->borrow_count = (int)(current->borrow_count * pow(DECAY_RATE, decay_factor));
    }
}

// Function to print all books in the library
void print_books(Library *library) {
    printf("Books in Library:\n");
    for (Book *current = library->header->forward[0]; current != NULL; current = current->forward[0]) {
        printf("Title: %s, Author: %s, Genres: ", current->title, current->author);
        for (int j = 0; j < current->gen_count; j++) { // Changed to use gen_count
            printf("%s%s", current->genre[j], (j < current->gen_count - 1) ? ", " : ""); // Use a conditional to manage commas
        }
        printf(", Borrow Count: %d\n", current->borrow_count);
    }
}

// Function to free memory allocated for the library
void free_library(Library *library) {
    Book *current = library->header;
    while (current != NULL) {
        Book *next = current->forward[0];
        free(current);
        current = next;
    }
    free(library);
}

int main()
{
    srand(time(NULL));
    Library *library = create_library();

    int user_type;
    printf("Welcome to the Library Management System!\n");
    printf("Are you a:\n");
    printf("1. Visitor\n");
    printf("2. Staff\n");
    printf("Enter your choice: ");
    scanf("%d", &user_type);
    getchar(); // to consume newline

    int choice;
    do
    {
        if (user_type == 1) // Visitor options
        {
            printf("\nVisitor Menu:\n");
            printf("1. Search for a book\n");
            printf("2. Print all books\n");
            printf("3. Recommend books\n");
            printf("4. Borrow a book\n");
            printf("5. Return a book\n");
            printf("6. Exit\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);
            getchar(); // to consume newline

            if (choice == 1)
            {
                char title[MAX_TITLE_LENGTH], genre[MAX_TITLE_LENGTH];
                printf("Enter book title to search: ");
                fgets(title, MAX_TITLE_LENGTH, stdin);
                title[strcspn(title, "\n")] = '\0';

                printf("Enter genre to search: ");
                fgets(genre, MAX_TITLE_LENGTH, stdin);
                genre[strcspn(genre, "\n")] = '\0';

                Book *book = search_book(library, title, genre);
                if (book)
                {
                    printf("Book found: Title: %s, Author: %s\n", book->title, book->author);
                }
                else
                {
                    printf("Book not found.\n");
                }
            }
            else if (choice == 2)
            {
                print_books(library);
            }
            else if (choice == 3)
            {
                char genre[MAX_TITLE_LENGTH];
                printf("Enter genre to search: ");
                fgets(genre, MAX_TITLE_LENGTH, stdin);
                genre[strcspn(genre, "\n")] = '\0';
                recommend_books(library, genre);
            }
            else if (choice == 4) // Borrow a book
            {
                char title[MAX_TITLE_LENGTH], genre[MAX_TITLE_LENGTH];
                printf("Enter book title to borrow: ");
                fgets(title, MAX_TITLE_LENGTH, stdin);
                title[strcspn(title, "\n")] = '\0';

                printf("Enter genre: ");
                fgets(genre, MAX_TITLE_LENGTH, stdin);
                genre[strcspn(genre, "\n")] = '\0';

                Book *book = search_book(library, title, genre);
                if (book && strcmp(book->status, "available") == 0)
                {
                    strcpy(book->status, "borrowed");
                    book->last_borrowed = time(NULL);
                    book->borrow_count++;
                    printf("You have borrowed: %s by %s\n", book->title, book->author);
                }
                else
                {
                    printf("Book is not available for borrowing.\n");
                }
            }
            else if (choice == 5) // Return a book
            {
                char title[MAX_TITLE_LENGTH], genre[MAX_TITLE_LENGTH];
                printf("Enter book title to return: ");
                fgets(title, MAX_TITLE_LENGTH, stdin);
                title[strcspn(title, "\n")] = '\0';

                printf("Enter genre: ");
                fgets(genre, MAX_TITLE_LENGTH, stdin);
                genre[strcspn(genre, "\n")] = '\0';

                Book *book = search_book(library, title, genre);
                if (book && strcmp(book->status, "borrowed") == 0)
                {
                    strcpy(book->status, "available");
                    printf("You have returned: %s by %s\n", book->title, book->author);
                }
                else
                {
                    printf("This book was not borrowed or does not exist in the library.\n");
                }
            }
        }
        else if (user_type == 2) // Staff options
        {
            printf("\nStaff Menu:\n");
            printf("1. Load books from file\n");
            printf("2. Add a new book\n");
            printf("3. Search for a book\n");
            printf("4. Print all books\n");
            printf("5. Exit\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);
            getchar(); // to consume newline

            if (choice == 1)
            {
                char filename[100];
                printf("Enter the filename to load books from: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0';
                read_books_from_file(library, filename);
            }
            else if (choice == 2)
            {
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
                for (int i = 0; i < genre_count; i++)
                {
                    printf("Enter genre %d: ", i + 1);
                    fgets(genres[i], MAX_TITLE_LENGTH, stdin);
                    genres[i][strcspn(genres[i], "\n")] = '\0';
                }
                add_book(library, title, author, genres, genre_count, borrow_count);
                printf("Book added successfully.\n");
            }
            else if (choice == 3)
            {
                char title[MAX_TITLE_LENGTH], genre[MAX_TITLE_LENGTH];
                printf("Enter book title to search: ");
                fgets(title, MAX_TITLE_LENGTH, stdin);
                title[strcspn(title, "\n")] = '\0';

                printf("Enter genre to search: ");
                fgets(genre, MAX_TITLE_LENGTH, stdin);
                genre[strcspn(genre, "\n")] = '\0';

                Book *book = search_book(library, title, genre);
                if (book)
                {
                    printf("Book found: Title: %s, Author: %s\n", book->title, book->author);
                }
                else
                {
                    printf("Book not found.\n");
                }
            }
            else if (choice == 4)
            {
                print_books(library);
            }
        }

    } while (choice != 5); // For both visitors and staff, exit option

    free_library(library);
    return 0;
}
