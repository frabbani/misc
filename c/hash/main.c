#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "mtwister.h"
#include "hash.h"

size_t myrand( size_t size ){
  static MTRand r;
  static int first = 1;
  if( first ){
    r = seedRand( time(0) );
    first = 0;
  }
  return (size_t)genRandLong( &r ) % (size == 0 ? 1 : size );
}

size_t test( hashmap_t *map, size_t *vals, size_t nvals ){
  size_t collisions = 0;
  for( size_t i = 0; i < nvals; i++ ){
    int c = hashmap_add( map, vals[i] );
    if( c ){
      //printf( "collision...\n" );
      collisions++;
    }
  }

  int finds = 0;
  for( size_t i = 0; i < nvals; i++ ){
    if( !hashmap_found( map, vals[i] ) )
      printf( "OH NO ITS ALL WENT WRONG!\n" );
    else
      finds++;
  }
  return collisions;
}


int main(){
	const size_t maxval = 300000;
  const size_t ntests = 5000;

	hashmap_t map;
  hashmap_init( &map, maxval );

	size_t  nvals = 100;
	size_t *vals = malloc( nvals * sizeof(size_t) );

	size_t total = 0;
	for( size_t n = 0; n < ntests; n++ ){
	  hashmap_reset( &map );
	  for( size_t i = 0; i < nvals; i++ )
	    vals[i] = myrand( maxval + 1 );
	  total += test( &map, vals, nvals );
	}

	float avg = (float)( (double)total/(double)ntests );
	printf( "average # of collisions: %d out of %d  (%f%%)\n",
	        (int)avg,
	        HASH_TABLE_SIZE,
	        avg / HASH_TABLE_SIZE * 100.0f  );
	printf( "goodbye!\n" );
}
