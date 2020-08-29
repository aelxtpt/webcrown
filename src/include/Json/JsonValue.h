#ifndef JSON_JSONVALUE_H
#define JSON_JSONVALUE_H

namespace webcrown{
namespace json {

enum class JsonType{
  Null,
  Int32,
  UnsignedInt32,
  Int64,
  UnsignedInt64,
  Double,
  Bool,
  String,
  Array,
  Object
};

}}

#endif
