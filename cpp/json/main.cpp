
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>
#include <vector>
#include <optional>

extern "C" {
#include <jq.h>
#include <jv.h>
}

#include "json.h"


std::string fileData(std::string_view filename) {
  char buffer[1024];
  FILE *fp = fopen(filename.data(), "r");
  if (!fp) {
    printf("failed to open file '%s'\n", filename.data());
    return "";
  }
  std::stringstream ss;
  while (fgets(buffer, sizeof(buffer), fp))
    ss << buffer;

  fclose(fp);
  return ss.str();
}

void format( std::string_view content ){
  std::string filter( "[.menu.popup.menuitem[].onclick]");
  printf( "*********\n");
  printf( "filter:\n");
  printf( "%s\n", filter.c_str() );

  auto jq = jq_init();
  if( !jq ){
    printf( "oops..\n");
    return;
  }
  jq_compile( jq, filter.c_str());
  jv input = jv_parse(content.data());
  if( jv_is_valid(input)){
    jq_start(jq, input, 0);
    jv next = jq_next(jq);
    printf( "*********\n");
    printf( "output:\n");
    next = jv_dump_string(next, 0);
    printf( "'%s'\n", jv_string_value(next));
    jv_free(next);
  }

  jv_free(input);
  jq_teardown(&jq);
  printf( "*********\n");
}

void create(){
  Json j;
  Json empty;
  auto o = Json("{\"father\": \"me\"}");
  j.set( "family", o.getObjectOr("", empty) );
  std::vector<std::string> fam = { "wife", "oldest child", "youngest child" };
  Json f;
  f.set( "", j.getObjectOr("family", empty) );
  f.setArray( "members", fam );
  j.set( "family", f );
  o = empty;
  o.set( "city", std::string("Allen") );
  o.set( "state", std::string("Texas") );
  o.set( "address", std::string("1234 Something Dr."));
  f.set( "residence", o );
  j.set( "family", f );
  printf( "output:\n'%s'\n", j.serialize().c_str());
}

int main() {

  auto content = fileData("in.json");
  Json j(content);
  printf( "*********\n");
  printf( "JSON information:\n");
  auto menu = j.getObject("menu");
  if (menu.has_value()) {
    printf("id: '%s'\n", menu->getStringOr("id", "?").c_str());
    printf("value: '%s'\n", menu->getStringOr("value", "?").c_str());
    auto popup = menu->getObject("popup");
    if( popup.has_value()){
      printf( "popup:\n");
      auto items = popup->getObjectArray( "menuitem");
      if( items.has_value()){
        printf( " * menu items:\n");
        for( auto& item : *items ){
          printf( "    - value/onclick: %s/%s\n",
                  item.getStringOr("value", "?").c_str(),
                  item.getStringOr("onclick", "?").c_str());
        }
      }
    }
  }

  format(content);
  create();

  return 0;

}
