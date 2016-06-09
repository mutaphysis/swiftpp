//
//  SwiftBridgeTmpl.h
//  swiftpp
//
//  Created by Sandy Martel on 3/02/2015.
//  Copyright (c) 2015 Sandy Martel. All rights reserved.
//

#ifndef H_SwiftBridgeTmpl
#define H_SwiftBridgeTmpl

const char kCXX_BRIDGE_H_TEMPLATE[] = R"(
// generated c_impl.h
//  pure C, cannot contain any C++

#ifndef H_C_IMPL_BRIDGE
#define H_C_IMPL_BRIDGE
<{objc_forward_decl}>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  char storage[<{std_string_size}>];
} <{ns}>StringWrapper;
<{ns}>StringWrapper <{ns}>StringWrapper_create(const char *s);
void <{ns}>StringWrapper_destroy(<{ns}>StringWrapper);
const char *<{ns}>StringWrapperUTF8(const <{ns}>StringWrapper * const);

<{#classes}>
struct <{ns}><{class_name}>_t;
<{#constructors}>
const struct <{ns}><{class_name}>_t *<{ns}><{class_name}>_allocate(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>);<{/constructors}>
void <{ns}><{class_name}>_destroy(const struct <{ns}><{class_name}>_t *i_cxxptr);
void <{ns}><{class_name}>_setup_subclass(const struct <{ns}><{class_name}>_t *i_cxxptr, const void *i_self<{#virtual_methods:prefix(,\n     )separator(,\n     )}><{return_c_type}> (*i_<{name}>_callback)(const void *<{#params:prefix(, )separator(, )}><{param_c_type}><{/params}>)<{/virtual_methods}>);
<{#methods}>
<{return_c_type}> <{ns}><{class_name}>_<{name}>(const struct <{ns}><{class_name}>_t *i_cxxptr<{#params:prefix(, )separator(, )}><{param_c_type}> <{param_name}><{/params}>);
<{/methods}><{#static_methods}>
<{return_c_type}> <{ns}><{class_name}>_<{name}>(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>);
<{/static_methods}>
<{/classes}>
#ifdef __cplusplus
}
#endif

#endif
)";

const char kCXX_BRIDGE_CPP_TEMPLATE[] = R"(
// generated c_impl.cpp

#include "<{bridge_include}>"
#include "c_impl.h"
#include <string>

<{ns}>StringWrapper <{ns}>StringWrapper_create(const std::string &s)
{
  <{ns}>StringWrapper cs;
  memset(&cs, 0, sizeof(cs));
  auto l = s.size()+1;
  if (l < (sizeof(cs)-1))
  {
    memcpy(cs.storage, s.c_str(), l);
  	cs.storage[sizeof(cs)-1] = 1;
  }
  else
  {
    *((char **)cs.storage) = new char[l];
    memcpy(*((char **)cs.storage), s.c_str(), l);
  }
  return cs;
}
std::string <{ns}>StringWrapper2String(<{ns}>StringWrapper cs)
{
  std::string result(<{ns}>StringWrapperUTF8(&cs));
  <{ns}>StringWrapper_destroy(cs);
  return result;
}

<{#classes}>
class <{ns}><{class_name}>_subclass : public <{class_name}>
{
public:
  const void *_self = nullptr;
<{#virtual_methods}>
  <{return_c_type}> (*_<{name}>_callback)(const void *<{#params:prefix(, )separator(, )}><{param_c_type}><{/params}>);<{/virtual_methods}>

<{#constructors}>
  <{ns}><{class_name}>_subclass(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>) : <{class_name}>(<{#params:separator(, )}><{param_name}><{/params}>){}<{/constructors}>

<{#virtual_methods}>
  virtual <{return_cxx_type}> <{name}>(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>)
  {
    <{#has_return_value}>return <{return_converter_c_to_cxx}>(<{/has_return_value}>(*_<{name}>_callback)(_self<{#params:prefix(, )separator(, )}><{param_as_c_type}><{/params}>)<{#has_return_value}>)<{/has_return_value}>;
  }
<{/virtual_methods}>
<{#methods}>
  inline <{return_cxx_type}> <{name}>_<{ns}>forward(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>)
  {
    <{#has_return_value}>return <{/has_return_value}><{class_name}>::<{name}>(<{#params:separator(, )}><{param_name}><{/params}>);
  }
<{/methods}>
<{#static_methods}>
  inline static <{return_cxx_type}> <{name}>_<{ns}>forward(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>)
  {
    <{#has_return_value}>return <{/has_return_value}><{class_name}>::<{name}>(<{#params:separator(, )}><{param_name}><{/params}>);
  }
<{/static_methods}>
};

// the c implementations

#ifdef __cplusplus
extern "C" {
#endif

<{ns}>StringWrapper <{ns}>StringWrapper_create(const char *s)
{
  <{ns}>StringWrapper cs;
  memset(&cs, 0, sizeof(cs));
  if (s)
  {
    auto l = strlen(s)+1;
    if (l < (sizeof(cs)-1))
    {
      memcpy(cs.storage, s, l);
      cs.storage[sizeof(cs)-1] = 1;
    }
    else
    {
      *((char **)cs.storage) = new char[l];
      memcpy(*((char **)cs.storage), s, l);
    }
  }
  return cs;
}
void <{ns}>StringWrapper_destroy(<{ns}>StringWrapper cs)
{
  if (cs.storage[sizeof(cs)-1] != 1)
	  delete [] *((char **)cs.storage);
}
const char *<{ns}>StringWrapperUTF8(const <{ns}>StringWrapper * const cs)
{
  if (cs->storage[sizeof(*cs)-1] == 1)
    return cs->storage;
  else
    return *((const char **)cs->storage);
}

<{#constructors}>
const struct <{ns}><{class_name}>_t *<{ns}><{class_name}>_allocate(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>)
{
  return (const struct <{ns}><{class_name}>_t *)new <{ns}><{class_name}>_subclass(<{#params:separator(, )}><{param_name}><{/params}>);
}<{/constructors}>
void <{ns}><{class_name}>_destroy(const struct <{ns}><{class_name}>_t *i_cxxptr)
{
  delete ((<{ns}><{class_name}>_subclass*)i_cxxptr);
}
void <{ns}><{class_name}>_setup_subclass(const struct <{ns}><{class_name}>_t *i_cxxptr, const void *i_self<{#virtual_methods:prefix(, )separator(, )}><{return_c_type}> (*i_<{name}>_callback)(const void *<{#params:prefix(, )separator(, )}><{param_c_type}><{/params}>)<{/virtual_methods}>)
{
  ((<{ns}><{class_name}>_subclass*)i_cxxptr)->_self = i_self;
<{#virtual_methods}>
  ((<{ns}><{class_name}>_subclass*)i_cxxptr)->_<{name}>_callback = i_<{name}>_callback;
<{/virtual_methods}>}

<{#methods}>
<{return_c_type}> <{ns}><{class_name}>_<{name}>(const struct <{ns}><{class_name}>_t *i_cxxptr<{#params:prefix(, )separator(, )}><{param_c_type}> <{param_name}><{/params}>)
{
  <{#has_return_value}>return <{return_converter_cxx_to_c}>(<{/has_return_value}>((<{ns}><{class_name}>_subclass*)i_cxxptr)-><{name}>_<{ns}>forward(<{#params:separator(, )}><{param_as_cxx_type}><{/params}>)<{#has_return_value}>)<{/has_return_value}>;
}
<{/methods}>
<{#static_methods}>
<{return_c_type}> <{ns}><{class_name}>_<{name}>(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>)
{
  <{#has_return_value}>return <{return_converter_cxx_to_c}>(<{/has_return_value}><{ns}><{class_name}>_subclass::<{name}>_<{ns}>forward(<{#params:separator(, )}><{param_as_cxx_type}><{/params}>)<{#has_return_value}>)<{/has_return_value}>;
}
<{/static_methods}>
#ifdef __cplusplus
}
#endif<{/classes}>
)";

const char kCXX_BRIDGE_SWIFT_TEMPLATE[] = R"(
// generated bridge.swift

//import Cocoa // fixme

func <{ns}>StringWrapper2SwiftString( cs : <{ns}>StringWrapper ) -> String {
  var cs = cs
  let result = String.fromCString(<{ns}>StringWrapperUTF8(&cs))!
  <{ns}>StringWrapper_destroy(cs)
  return result
}

<{#classes}>
class <{class_name}> {
  let _super : COpaquePointer

<{#constructors}>
  init(<{#params:separator(, )}><{param_clean_name}>: <{param_swift_type}><{/params}>) {
    self._super = <{ns}><{class_name}>_allocate(<{#params:separator(, )}><{param_clean_name}><{/params}>)

    <{ns}><{class_name}>_setup_subclass( self._super,
      UnsafePointer(Unmanaged.passUnretained(self).toOpaque())<{#virtual_methods:prefix(,\n        )separator(,\n        )}>
{ ( i_self: UnsafePointer<Void><{#params:prefix(, )separator(, )}><{param_clean_name}>: <{param_swift_c_type}><{/params}>)<{return_swift_c_type:prefix( -> )}> in
          let _self = Unmanaged<<{class_name}>>.fromOpaque( COpaquePointer(i_self) ).takeUnretainedValue()
          <{#has_return_value}>return <{return_converter_swift_to_c}>(<{/has_return_value}>_self.<{name}>(<{#params:separator(, )}><{param_as_swift_type}><{/params}>)<{#has_return_value}>)<{/has_return_value}>
        }<{/virtual_methods}>
      )
  }<{/constructors}>

  deinit {
    <{ns}><{class_name}>_destroy(self._super)
  }

<{#methods}>
  func <{name}>(<{#params:separator(, )}><{param_clean_name}>: <{param_swift_type}><{/params}>)<{return_swift_type:prefix( -> )}> {
    <{#has_return_value}>return <{return_converter_c_to_swift}>(<{/has_return_value}><{ns}><{class_name}>_<{name}>(self._super<{#params:prefix(, )separator(, )}><{param_clean_name}><{/params}>)<{#has_return_value}>)<{/has_return_value}>
  }
<{/methods}>
<{#static_methods}>
  static func <{name}>(<{#params:separator(, )}><{param_clean_name}>: <{param_swift_type}><{/params}>)<{return_swift_type:prefix( -> )}> {
    <{#has_return_value}>return <{return_converter_c_to_swift}>(<{/has_return_value}><{ns}><{class_name}>_<{name}>(<{#params:separator(, )}><{param_clean_name}><{/params}>)<{#has_return_value}>)<{/has_return_value}>
  }
<{/static_methods}>
}
<{/classes}>
)";

#endif
