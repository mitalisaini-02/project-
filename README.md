# Library Management System

This is a C-based Library Management System that uses a skip graph data structure to manage books, genres, and borrow counts. It allows library staff to manage books, track borrowing history, provide genre-based recommendations, and perform various operations such as borrowing and returning books. A max-heap is used to enhance the recommendation feature, prioritizing popular books by borrow count.

## Features

1. **Load Books from File** - Load a list of books from a file and add them to the library database.
2. **Add a New Book** - Add a new book to the library with title, author, genres, and borrow count.
3. **Search for a Book** - Search for a specific book by title and genre.
4. **Print All Books** - Print a list of all books currently in the library.
5. **Recommend Books** - Get top recommendations for a specific genre based on popularity.
6. **Borrow a Book** - Borrow a book, updating its borrow count and availability status.
7. **Return a Book** - Return a borrowed book, updating its status to available.

## Data Structures

### Skip Graph
The skip graph allows for fast insertion, searching, and traversal of books. Books are stored in multiple levels, enabling efficient operations for larger libraries.

### Max-Heap
The max-heap stores top recommended books based on borrow count, providing library staff with quick access to popular books.

## Decay Mechanism for Borrow Counts
A decay rate is applied to the borrow counts over time. This prevents older books with high borrow counts from permanently dominating recommendations, ensuring that more recent popular books are prioritized.

## Code Structure

- **Structures**:
  - `Book`: Represents a single book with attributes like title, author, genres, borrow count, and borrow status.
  - `Library`: Manages a skip graph of books and handles the overall library operations.
  - `MaxHeap`: Manages the heap for recommending books based on popularity.

- **Functions**:
  - `create_library`: Initializes an empty library.
  - `add_book`: Adds a new book to the skip graph.
  - `search_book`: Searches for a book by title and genre, supporting gaps in title match.
  - `decay_borrow_counts`: Applies decay to borrow counts based on a fixed rate and time since last borrowed.
  - `print_books`: Displays all books in the library.
  - `recommend_books`: Provides recommendations based on genre and borrow count.
  - `borrow_book`: Allows a user to borrow a book, updating the status and borrow count.
  - `return_book`: Allows a user to return a borrowed book, updating its status.

## File Format for Book Loading

Books is loaded from a file in the following txt format:
