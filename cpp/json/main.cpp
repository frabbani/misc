
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <optional>

#include "json.hpp"

using json = nlohmann::json;

struct JsonImpl;

struct Json {
  std::unique_ptr<JsonImpl> impl = nullptr;
  Json() = default;
  void operator=(json &j);
  Json(std::string_view s);
  template<class T> std::optional<T> get(std::string_view key);
  template<class T> std::optional<std::vector<T>> getArray(std::string_view key);

  template<class T> T getOr(std::string_view key, const T &other);
  template<class T> std::vector<T> getArrayOr(std::string_view key, const std::vector<T> &other);
};

template<class T> static constexpr bool matches(const json &j) {
  if (j.is_null())
    return false;
  if (std::is_same<T, int>::value)
    return j.is_number_integer();
  if (std::is_same<T, bool>::value)
    return j.is_boolean();
  if (std::is_same<T, float>::value)
    return j.is_number_float();
  if (std::is_same<T, std::string>::value)
    return j.is_string();
  if (std::is_same<T, Json>::value)
    return j.is_object();

  if (j.is_array() && j.size()) {
    const auto &v = j[0];
    if (std::is_same<T, std::vector<int>>::value)
      return v.is_number_integer();
    if (std::is_same<T, std::vector<float>>::value)
      return v.is_number_float();
    if (std::is_same<T, std::vector<std::string>>::value)
      return v.is_string();
    if (std::is_same<T, std::vector<Json>>::value)
      return v.is_object();
  }

  return false;
}

struct JsonImpl {
  json j;
  json v;
  ~JsonImpl() = default;
  explicit JsonImpl(json j_)
      :
      j(std::move(j_)) {
  }
  explicit JsonImpl(std::string_view content)
      :
      j(json::parse(content)) {
  }

  bool find(std::string_view key) {
    v = j[key];
    return !v.is_null();
  }

  template<class T> bool get(T &value) {
    if (matches<T>(v)) {
      value = v;
      return true;
    }
    return false;
  }

  template<class T> bool getArray(std::vector<T> &values) {
    if (matches<std::vector<T>>(v)) {
      values = std::vector<T>(v.size());
      for (size_t i = 0; i < v.size(); i++)
        values[i] = v[i];
      return true;
    }
    return false;
  }
};

Json::Json(std::string_view content)
    :
    impl(std::make_unique<JsonImpl>(content)) {
}

void Json::operator=(json &j) {
  impl = std::make_unique<JsonImpl>(j);
}

template<class T> std::optional<T> Json::get(std::string_view key) {
  T value;
  if (!impl->find(key) || !impl->get<T>(value))
    return std::nullopt;
  return value;
}

template<class T> std::optional<std::vector<T>> Json::getArray(std::string_view key) {
  std::vector<T> values;
  if (!impl->find(key) || !impl->getArray<T>(values))
    return std::nullopt;
  return values;
}

template<class T>
T Json::getOr(std::string_view key, const T &other) {
  T value;
  if (!impl->find(key) || !impl->get<T>(value))
    return other;
  return value;
}

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

int main() {
  /*
   json j = { { "pi", 3.14159 }, { "happy", true }, { "name", "who" }, { "nothing", nullptr }, { "answer", { { "everything", 42 } } }, { "list", { 1, 0, 2 } }, { "object", { { "currency", "USD" }, { "value", 66.66 } } } };

   // add new values
   j["new"]["key"]["value"] = { "another", "list" };

   // count elements
   auto s = j.size();
   j["size"] = s;

   // pretty print with indent of 4 spaces
   std::cout << std::setw(4) << j << '\n';
   */

  Json j(fileData("in.json"));

  auto menu = j.get<Json>("menu");
  if (menu.has_value()) {
    printf("id: '%s'\n", menu->getOr<std::string>("id", "?").c_str());
    printf("value: '%s'\n", menu->getOr<std::string>("value", "?").c_str());
    auto popup = menu->get<Json>("popup");
    if( popup.has_value()){
      printf( "popup:\n");
      auto items = popup->getArray<Json>( "menuitem");
      if( items.has_value()){
        printf( " * menu items:\n");
        for( auto& item : *items ){
          printf( "    - value/onclick: %s/%s\n",
                  item.getOr<std::string>("value", "?").c_str(),
                  item.getOr<std::string>("onclick", "?").c_str());
        }
      }
    }
  }

  return 0;

}
