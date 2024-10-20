#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEVEL 5
#define MAX_GENRES 10

//intialize what info book relate

typedef struct Book {
    char serialNo[20];
    char title[100];
    char author[50];
    char genre[30];
    int priority;           // the basic thing to level up in the skip graph
    struct Book* next[MAX_LEVEL];  // Pointers for skip graph
} Book;

// create a new book node
Book* createBook(const char* serialNo, const char* title, const char* author, const char* genre, int priority) {
    Book* newBook = (Book*)malloc(sizeof(Book));
    strcpy(newBook->serialNo, serialNo);
    strcpy(newBook->title, title);
    strcpy(newBook->author, author);
    strcpy(newBook->genre, genre);
    newBook->priority = priority;
    for (int i = 0; i < MAX_LEVEL; i++) {
        newBook->next[i] = NULL;
    }
    return newBook;
}



// add a new book in library 
void insertBook(Book* head, Book* newBook) {
    Book* current = head;
    // Traverse through each level and insert based on priority
    for (int level = MAX_LEVEL - 1; level >= 0; level--) {
        while (current->next[level] != NULL && strcmp(current->next[level]->serialNo, newBook->serialNo) < 0) {
            current = current->next[level];
        }
        // Insert the book at the correct position for the current level
        if (level < newBook->priority / 10) {
            newBook->next[level] = current->next[level];
            current->next[level] = newBook;
        }
    }
}