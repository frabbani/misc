#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

struct Heap {
  struct Block {
    char tag[16];
    int space;
    int sizeOfLast;

    static int available( int space ) {
      space -= sizeof(Block);
      return space < 0 ? 0 : space;
    }

    static void clearChain( Block *b ) {
      if( b->used() )
        return;
      auto rewind = []( Block *b ) {
        while( b->last() && !b->last()->used() ){
          b = b->last();
        }
        return b;
      };

      Block *first = rewind( b );
      b = first->next();
      while( b->space && !b->used() ){
        Block *next = b->next();
        first->space += b->size();
        b->clear( false );
        b = next;
      }
    }

    void mark( const char *tag_ ) {
      strncpy( tag, tag_, sizeof(tag) - 1 );
      if( tag[0] == '_' )
        tag[0] = '*';
    }
    void unmark() {
      memset( tag, '_', sizeof(tag) );
    }

    void clear( bool unmarked ) {
      char v = unmarked ? '_' : '\0';
      memset( tag, v, sizeof(tag) );
      space = 0;
    }

    int size() const {
      return sizeof(Block) + space;
    }
    bool used() const {
      return tag[0] != '\0' && tag[0] != '_';
    }

    bool matches( const char *tag_ ) const {
      return 0 == strncmp( tag, tag_, sizeof(tag) );
    }

    Block* last() {
      if( 0 == sizeOfLast )
        return nullptr;
      auto p = reinterpret_cast< uint8_t* >( this );
      return reinterpret_cast< Block* >( &p[-sizeOfLast] );
    }
    Block* next() {
      auto p = reinterpret_cast< uint8_t* >( this );
      return reinterpret_cast< Block* >( &p[size()] );
    }

    void free() {
      if( !used() || !space )
        return;
      unmark();
      clearChain( this );
    }

    template< typename T >
      T* mem() {
        auto p = reinterpret_cast< uint8_t* >( this );
        return reinterpret_cast< T* >( &p[sizeof(Block)] );
      }

    void reserve( int needed, const char *tag_ ) {
      if( needed > space )
        return;
      mark( tag_ );
      int remaining = space - needed;
      if( remaining > ((int)sizeof(Block) + 8) ){
        space = needed;
        Block *b = next();
        b->unmark();
        b->space = available( remaining );
        b->sizeOfLast = size();
      }
    }
  };

  Heap() {
    memset( allocated, 0, size );
    printf( "Heap Block size is %zu bytes\n", sizeof(Block) );
  }
  ~Heap() {
    delete[] allocated;
    allocated = nullptr;
  }

  Block* first() {
    return reinterpret_cast< Block* >( allocated );
  }

  template< typename T >
    T* fetch( int needed, const char *tag ) {
      if( needed & 7 )
        needed = ((needed >> 3) + 1) << 3;

      Block *b = first();
      Block *lastb = nullptr;
      while( b->space ){
        if( b->used() ){
          lastb = b;
          b = b->next();
          continue;
        }
        if( b->space >= needed ){
          b->reserve( needed, tag );
          return b->mem< T >();
        }
        lastb = b;
        b = b->next();
      }
      b->mark( tag );
      b->space = needed;
      b->sizeOfLast = lastb ? lastb->size() : 0;
      return b->mem< T >();
    }

  void free( const char *tag ) {
    Block *b = first();
    while( b->space ){
      if( b->used() && b->matches( tag ) ){
        b->free();
        break;
      }
      b = b->next();
    }
  }
  const int size = 16 * 1024 * 1024; //16 megs
  uint8_t *allocated = new uint8_t[size];

  void dump() {
    printf( "****************************\n" );
    Block *b = first();
    while( b->space ){
      b->used() ? printf( "%s\n", b->tag ) : printf( "(unmarked)\n" );
      printf( " * size.: %d (%zu + %d)\n", b->size(), sizeof(Heap::Block), b->space );
      printf( " * space: %d\n", b->space );
      char line[128];
      int n = b->space < 128 ? b->space : 128;
      char *p = b->mem< char >();
      for( int i = 0; i < n; i++ ){
        char c = p[i];
        c = (int)c < (int)' ' ? '.' : c;
        c = c == ' ' || c == '\n' ? '_' : c;
        line[i] = c;
      }
      line[n] = '\0';
      printf( " * data.: %s\n", line );
      b = b->next();
    }
    printf( "****************************\n" );
  }

};

int main( int argc, char *args[] ) {
  printf( "hello world!\n" );
  Heap heap;
  char *p;
  p = heap.fetch< char >( 8, "# 1" );
  strcpy( p, "Heap" );

  p = heap.fetch< char >( 32, "# 2" );
  strcpy( p, "hello " );
  p = heap.fetch< char >( 32, "# 3" );
  strcpy( p, "world!" );

  p = heap.fetch< char >( 8, "# 4" );
  strcpy( p, "Demo" );
  heap.dump();
  printf( "Freeing block 3 & 3\n" );
  heap.free( "# 2" );
  heap.free( "# 3" );
  heap.dump();
  printf( "Allocating block 5\n" );
  p = heap.fetch< char >( 24, "# 5" );
  strcpy( p, "HELLO WORLD!" );

  heap.dump();

  printf( "goodbye!\n" );
  return 0;
}
