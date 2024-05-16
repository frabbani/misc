#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// https://github.com/protobuf-c/protobuf-c
// #  pkg-config.exe --libs 'libprotobuf-c >= 1.0.0'
// # protoc --c_out=. message.proto

#define TRUE 1
#define FALSE 0

#include "message.pb-c.h"

void unpack( int len, uint8_t *buf ){
  printf( "unpacking...\n");
  AMessage *msg = amessage__unpack( NULL, len, buf );
  if( msg ){
    printf( "a: %d\n", msg->a );
    if( msg->has_b )
      printf( "b: %d\n", msg->b );
    amessage__free_unpacked( msg, NULL );
  }
  else
    printf( "awww...\n" );
}

int main( int argc, char *args[] ){
  printf( "hello world...\n" );

  AMessage msg;
  amessage__init( &msg );

  msg.a = 0xaa;
  msg.has_b = FALSE;
  msg.b = 0xbb;

  int len = amessage__get_packed_size( &msg );
  printf( "packed length: %d\n", len );

  uint8_t *buf = malloc( len );
  amessage__pack( &msg, buf );

  printf( "serialized message:\n" );
  for( int i = 0; i < len; i++ ){
    printf( "%x", buf[i] );
  }
  printf( "\n" );

  unpack( len, buf );

  free( buf );
  printf( "goodbye!\n" );
  return 0;
}
