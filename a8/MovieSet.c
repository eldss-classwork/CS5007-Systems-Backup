/*  Modified by Evan Douglass, March 25 2019.
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

#include "htll/Hashtable.h"
#include "htll/LinkedList.h"
#include "MovieSet.h"

void SimpleFree(void *payload) {
  free(payload);
}

void NullFree(void* item) {}

// Returns 1 if item in list, 0 otherwise
int LookupMovieInLinkedList(LinkedList list, MoviePtr mov) {
  // No elements in list
  if (NumElementsInLinkedList(list) == 0) {
    return 0;
  }

  // Test each existing string
  LLIter iter = CreateLLIter(list);
  MoviePtr payload;
  while (1) {
    LLIterGetPayload(iter, (void**)&payload);
    if (strcmp(mov->id, payload->id) == 0) {
      DestroyLLIter(iter);
      return 1;
    }
    // Move to next movie if available
    if (LLIterHasNext(iter)) {
      LLIterNext(iter);
    } else {
      break;
    }
  }
  DestroyLLIter(iter);
  return 0;
}

// Returns 0 if successful, non-zero otherwise
int AddMovieToSetOfMovies(SetOfMovies set, MoviePtr mov) {
  // Ensure movie not in set already
  int result = 1;  // defaults to unsuccessful
  if (! (LookupMovieInLinkedList(set->movies, mov))) {  // movie not in list
    result = InsertLinkedList(set->movies, (void*)mov);
  }
  return result;
}

int AddMovieToSet(MovieSet set,  uint64_t docId, int rowId) {
  Hashtable docInd = set->doc_index;

  HTKeyValue kvp;
  HTKeyValue old_kvp;
  int result = LookupInHashtable(docInd, docId, &kvp);

  // If the docId is in docInd, use it.
  // Otherwise, create a new entry for this docId in docInd.
  if (result < 0) {
    kvp.key = docId;
    kvp.value = CreateLinkedList();
    PutInHashtable(docInd, kvp, &old_kvp);
  }

  // add rowId to the linked list.
  void *val = malloc(sizeof(int));

  if (val == NULL) {
    // Out of mem
    printf("Out of memory adding movie to set: %s\n", set->desc);
    return -1;
  }

  *((int*)val) = rowId;
  result = InsertLinkedList((LinkedList)kvp.value, val);

  return result;
}

int MovieSetContainsDoc(MovieSet set, uint64_t docId) {
  HTKeyValue kvp;
  return LookupInHashtable(set->doc_index, docId, &kvp);
}

void PrintOffsetList(LinkedList list) {
  printf("Printing offset list\n");
  LLIter iter = CreateLLIter(list);
  int* payload;
  while (LLIterHasNext(iter) != 0) {
    LLIterGetPayload(iter, (void**)&payload);
    printf("%d\t", *((int*)payload));
    LLIterNext(iter);
   }
 }

MovieSet CreateMovieSet(char *desc) {
  MovieSet set = (MovieSet)malloc(sizeof(struct movieSet));
  if (set == NULL) {
    // Out of memory
    printf("Couldn't malloc for movieSet %s\n", desc);
    return NULL;
  }
  set->desc = (char*)malloc(strlen(desc) *  sizeof(char) + 1);
  if (set->desc == NULL) {
    printf("Couldn't malloc for movieSet->desc\n");
    return NULL;
  }
  strcpy(set->desc, desc);
  set->doc_index = CreateHashtable(16);
  return set;
}

SetOfMovies CreateSetOfMovies(char *desc) {
  SetOfMovies set = (SetOfMovies)malloc(sizeof(struct setOfMovies));
  if (set == NULL) {
    // Out of memory
    printf("Couldn't malloc for movieSet %s\n", desc);
    return NULL;
  }
  set->desc = (char*)malloc(strlen(desc) * sizeof(char) + 1);
  if (set->desc == NULL) {
    free(set);
    printf("Couldn't malloc for SetOfMovies->desc\n");
    return NULL;
  }
  strcpy(set->desc, desc);
  set->movies = CreateLinkedList();
  return set;
}

void DestroyOffsetList(void *val) {
  LinkedList list = (LinkedList)val;
  DestroyLinkedList(list, &SimpleFree);
}

void DestroyMovieSet(MovieSet set) {
  // Free desc
  free(set->desc);
  // Free doc_index
  DestroyHashtable(set->doc_index, &DestroyOffsetList);
  // Free set
  free(set);
}

void DestroySetOfMovies(SetOfMovies set) {
  free(set->desc);
  DestroyLinkedList(set->movies, &NullFree);
  free(set);
}
