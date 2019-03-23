/*
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Movie.h"

Movie* CreateMovie() {
  Movie *mov = (Movie*)malloc(sizeof(Movie));
  if (mov == NULL) {
    printf("Couldn't allocate more memory to create a Movie\n"); 
    return NULL; 
  }
  // TODO: Populate/Initialize movie.
  
  return mov; 
}

void DestroyMovie(Movie* movie) {
  free(movie->id);
  free(movie->type);
  free(movie->title);
  free(movie->genres); 
  free(movie); 
}

char* CheckAndAllocateString(char* token) {
  if (strcmp("-", token) == 0) {
    return NULL; 
  } else {
    char *out = (char *) malloc((strlen(token) + 1) * sizeof(char));
    strcpy(out, token);
    return out; 
  }

}

int CheckInt(char* token) {
  if (strcmp("-", token) == 0) {
    return -1; 
  }
  else return atoi(token); 
}

Movie* CreateMovieFromRow(char *data_row) {
  Movie* mov = CreateMovie();
  if (mov == NULL) {
    printf("Couldn't create a Movie.\n");
    return NULL;
  }
  int num_fields = 9; 
  
  char *token[num_fields];
  char *rest = data_row;
  
  for (int i=0; i<num_fields; i++) {
    token[i] = strtok_r(rest, "|", &rest);
    if (token[i] == NULL) {
      fprintf(stderr, "Error reading row\n");
      return NULL; 
    }
  }
  
  mov->id = CheckAndAllocateString(token[0]);
  mov->type = CheckAndAllocateString(token[1]);
  mov->title = CheckAndAllocateString(token[2]); 
  mov->isAdult = CheckInt(token[4]); 
  mov->year = CheckInt(token[5]);
  mov->runtime = CheckInt(token[7]); 
  // TODO: Change such that genres is an array, not just a string. 
  mov->genres = CheckAndAllocateString(token[8]); 
  
  return mov; 
}
