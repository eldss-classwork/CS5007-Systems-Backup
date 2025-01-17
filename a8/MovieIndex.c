/*  Modified by Evan Douglass, March 24 2019.
 *
 *  Created by Adrienne Slaughter
 *  CS 5007 Spring 2019
 *  Northeastern University, Seattle
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  See <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "MovieIndex.h"
#include "htll/LinkedList.h"
#include "htll/Hashtable.h"
#include "Movie.h"
#include "MovieSet.h"

//static void NullFree(void *freeme) {}

void DestroyMovieSetWrapper(void *movie_set) {
  DestroyMovieSet((MovieSet)movie_set);
}

void DestroySetOfMovieWrapper(void *set_movie) {
  DestroySetOfMovies((SetOfMovies)set_movie);
}

void toLower(char *str, int len) {
  for (int i = 0; i < len; i++) {
    str[i] = tolower(str[i]);
  }

}


Index CreateIndex() {
  Index ind = (Index)malloc(sizeof(struct index));
  // TODO: How big to make this hashtable? How to decide? What to think about?
  // Make this "appropriate".
  ind->ht = CreateHashtable(128);
  ind->movies = NULL; // TO BE NULL until it's populated/used.
  return ind;
}

int DestroyIndex(Index index, void (*destroyValue)(void *)) {
  DestroyHashtable(index->ht, destroyValue);

  if (index->movies != NULL) {
    DestroyLinkedList(index->movies, &DestroyMovieWrapper);
  } else {
  }
  free(index);
  return 0;
  }

// Destroy index that has an offsetlist as a value
int DestroyOffsetIndex(Index index) {
    return DestroyIndex(index, DestroyMovieSetWrapper);
}

// Destroy's index that has aSetOfMovies as a value
int DestroyTypeIndex(Index index) {
  return DestroyIndex(index, DestroySetOfMovieWrapper);
}


// Assumes Index is a hashtable with key=title word, and value=hashtable with key doc id and value linked list of rows
int AddMovieTitleToIndex(Index index,
                         Movie *movie,
                         uint64_t doc_id,
                         int row_id) {
  // Put in the index
  HTKeyValue kvp;

  // TODO: How to choose this number? What are the pros and cons of choosing the wrong number? (DONE)
  int numFields = 1000;

  char *token[numFields];
  char *rest = movie->title;

  int i = 0;
  token[i] = strtok_r(rest, " ", &rest);
  while (token[i] != NULL) {
    toLower(token[i], strlen(token[i]));
    i++;
    token[i] = strtok_r(rest, " ", &rest);
  }

  for (int j = 0; j < i; j++) {
    // If this key is already in the hashtable, get the MovieSet.
    // Otherwise, create a MovieSet and put it in.
    int result = LookupInHashtable(index->ht,
                          FNVHash64((unsigned char*)token[j],
                                    (unsigned int)strlen(token[j])), &kvp);
    HTKeyValue old_kvp;

    if (result < 0) {
      kvp.value = CreateMovieSet(token[j]);
      kvp.key = FNVHash64((unsigned char*)token[j], strlen(token[j]));
      PutInHashtable(index->ht, kvp, &old_kvp);
    }

    AddMovieToSet((MovieSet)kvp.value, doc_id, row_id);
  }

  return 0;
}

// Gets genres from the given movie and adds the movie
// to an index under each genre seperately
// Returns 0 if successful or a 1-indexed number representing
// which genre the error occured at.
int AddGenresToIndex(Index index, Movie *movie) {
  int num_genres = NumElementsInLinkedList(movie->genres);
  LLIter iter = CreateLLIter(movie->genres);
  HTKeyValue kvp, old_kvp;
  // Init index list if not started
  if (index->movies == NULL) {
    index->movies = CreateLinkedList();
  }

  for (int i = 0; i < num_genres; i++) {
    char* genre;
    int result;

    // Get a genre
    result = LLIterGetPayload(iter, (void**)&genre);
    if (result == 1) {
      DestroyLLIter(iter);
      return i + 1;  // return 1-indexed genre on fail
    }

    // Look for existing keys
    uint64_t key = FNVHash64((unsigned char*)genre, strlen(genre));
    result = LookupInHashtable(index->ht, key, &kvp);

    if (result < 0) {
      // Need to add a new kvp
      kvp.value = (void*)CreateSetOfMovies(genre);
      kvp.key = key;
      PutInHashtable(index->ht, kvp, &old_kvp);
    }
    result = AddMovieToSetOfMovies((SetOfMovies)kvp.value, movie);
    if (result == 1) {  // unsuccessful add
      DestroyLLIter(iter);
      DestroyMovie(movie);
      return 1;
    }

    LLIterNext(iter);
  }
  DestroyLLIter(iter);

  // Add the movie to the index LL if not a duplicate
  if (! (LookupMovieInLinkedList(index->movies, movie))) {  // movie not in list
    InsertLinkedList(index->movies, (void*)movie);
  }

  return 0;
}

int AddMovieToIndex(Index index, Movie *movie, enum IndexField field) {
  if (field == Genre) {
    return AddGenresToIndex(index, movie);
  }

  // Init index list if not started
  if (index->movies == NULL) {
    index->movies = CreateLinkedList();
  }

  // Put in the index
  HTKeyValue kvp;
  HTKeyValue old_kvp;

  // If this key is already in the hashtable, get the SetOfMovies.
  // Otherwise, create a SetOfMovies and put it in.
  int result = LookupInHashtable(index->ht,
                                 ComputeKey(movie, field),
                                 &kvp);

  if (result < 0) {
    char* doc_set_name;
    char year_str[10];
    switch (field) {
      case Type:
        doc_set_name = movie->type;
        break;
      case Year:
        snprintf(year_str, sizeof(year_str), "%d", movie->year);
        doc_set_name = year_str;
        break;
      case Id:
        doc_set_name = movie->id;
        break;
      case Genre:
        // TODO: What to do if there are multiple genres?
        // Handle genre indexing as a completely different case
        break;
    }
    kvp.value = (void*)CreateSetOfMovies(doc_set_name);
    kvp.key = ComputeKey(movie, field);
    PutInHashtable(index->ht, kvp, &old_kvp);
  }
  // TODO: Something needs to happen here
  result = AddMovieToSetOfMovies((SetOfMovies)kvp.value, movie);
  if (result == 1) {  // unsuccessful add
    DestroyMovie(movie);
    return 1;
  }

  // Add the movie to the index LL if not a duplicate
  if (! (LookupMovieInLinkedList(index->movies, movie))) {  // movie not in list
    InsertLinkedList(index->movies, (void*)movie);
  }

  return 0;
}

// This function has been modified so that calling it with Genre as
// the field will do nothing and return 1
uint64_t ComputeKey(Movie* movie, enum IndexField which_field) {
  switch(which_field) {
    case Year:
      return FNVHashInt64(movie->year);
      break;
    case Type:
      return FNVHash64((unsigned char*)movie->type, strlen(movie->type));
      break;
    case Id:
      return FNVHash64((unsigned char*)movie->id, strlen(movie->id));
      break;
    case Genre:
      // TODO: how to deal with multiple genres??
      // This is handled by a different function
      return -1u;
  }
  return -1u;
}


MovieSet GetMovieSet(Index index, const char *term) {
  HTKeyValue kvp;
  char lower[strlen(term)+1];
  strcpy(lower, term);
  toLower(lower, strlen(lower));
  int result = LookupInHashtable(index->ht,
                                 FNVHash64((unsigned char*)lower,
                                           (unsigned int)strlen(lower)),
                                 &kvp);
  if (result < 0) {
    printf("term couldn't be found: %s \n", term);
    return NULL;
  }
  printf("returning movieset\n");
  return (MovieSet)kvp.value;
}
