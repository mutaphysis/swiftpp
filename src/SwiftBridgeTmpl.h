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
// <{class_name}>
// ------------------------
typedef struct <{ns}><{class_name}>_t <{ns}><{class_name}>;

<{#constructors}>
<{ns}><{class_name}> *<{ns}><{class_name}>_allocate(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>);
<{/constructors}>

void <{ns}><{class_name}>_destroy(<{ns}><{class_name}> *i_cxxptr);
void <{ns}><{class_name}>_setup_subclass(<{ns}><{class_name}> *i_cxxptr, const void *i_self<{#virtual_methods:prefix(,)separator(,\n     )}>
     <{return_c_type}> (*i_<{name}>_callback)(const void *<{#params:prefix(, )separator(, )}><{param_c_type}><{/params}>)
<{/virtual_methods}>  );

<{#base_classes}>
<{ns}><{base_name}> *<{ns}><{class_name}>_as_<{base_name}>(<{ns}><{class_name}> *i_cxxptr);
<{/base_classes}>

<{#methods}>
<{#is_non_static}>
<{return_c_type}> <{ns}><{class_name}>_<{name}>(<{ns}><{class_name}> *i_cxxptr<{#params:prefix(, )separator(, )}><{param_c_type}> <{param_name}><{/params}>);
<{return_c_type}> <{ns}><{class_name}>_<{name}>_forward(<{ns}><{class_name}> *i_cxxptr<{#params:prefix(, )separator(, )}><{param_c_type}> <{param_name}><{/params}>);
<{/is_non_static}>
<{#is_static}>
<{return_c_type}> <{ns}><{class_name}>_<{name}>(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>);
<{/is_static}>
<{/methods}>

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

extern "C" {

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

}

<{#classes}>
// <{class_name}>
// ------------------------
class <{ns}><{class_name}>_forwarder : public <{class_name}>
{
  public:
<{#methods}>
<{#is_protected}>
    using <{class_name}>::<{name}>(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>);
<{/is_protected}>
<{/methods}>
};

class <{ns}><{class_name}>_overrider : public <{class_name}>
{
public:
  const void *_self = nullptr;
<{#virtual_methods}>
  <{return_c_type}> (*_<{name}>_callback)(const void *<{#params:prefix(, )separator(, )}><{param_c_type}><{/params}>);
<{/virtual_methods}>

<{#constructors}>
  <{ns}><{class_name}>_overrider(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>) : <{class_name}>(<{#params:separator(, )}><{param_name}><{/params}>){}
<{/constructors}>

<{#virtual_methods}>
  virtual <{return_cxx_type}> <{name}>(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>)
  {
    <{#has_return_value}>return <{return_converter_c_to_cxx}>(<{/has_return_value}>(*_<{name}>_callback)(_self<{#params:prefix(, )separator(, )}><{param_as_c_type}><{/params}>)<{#has_return_value}>)<{/has_return_value}>;
  }
<{/virtual_methods}>
};

// the c implementation

extern "C" {

<{#constructors}>
<{ns}><{class_name}> *<{ns}><{class_name}>_allocate(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>)
{
  return (<{ns}><{class_name}> *)new <{ns}><{class_name}>_overrider(<{#params:separator(, )}><{param_name}><{/params}>);
}
<{/constructors}>
void <{ns}><{class_name}>_destroy(<{ns}><{class_name}> *i_cxxptr)
{
  delete ((<{ns}><{class_name}>_overrider*)i_cxxptr);
}
void <{ns}><{class_name}>_setup_subclass(<{ns}><{class_name}> *i_cxxptr, const void *i_self<{#virtual_methods:prefix(, )separator(, )}><{return_c_type}> (*i_<{name}>_callback)(const void *<{#params:prefix(, )separator(, )}><{param_c_type}><{/params}>)<{/virtual_methods}>)
{
  ((<{ns}><{class_name}>_overrider*)i_cxxptr)->_self = i_self;
<{#virtual_methods}>
  ((<{ns}><{class_name}>_overrider*)i_cxxptr)->_<{name}>_callback = i_<{name}>_callback;
<{/virtual_methods}>}

<{#methods}>
<{#is_non_static}>
<{return_c_type}> <{ns}><{class_name}>_<{name}>(<{ns}><{class_name}> *i_cxxptr<{#params:prefix(, )separator(, )}><{param_c_type}> <{param_name}><{/params}>)
{
  <{#has_return_value}>return <{return_converter_cxx_to_c}>(<{/has_return_value}>((<{ns}><{class_name}>_forwarder*)i_cxxptr)-><{name}>(<{#params:separator(, )}><{param_as_cxx_type}><{/params}>)<{#has_return_value}>)<{/has_return_value}>;
}
<{return_c_type}> <{ns}><{class_name}>_<{name}>_forward(<{ns}><{class_name}> *i_cxxptr<{#params:prefix(, )separator(, )}><{param_c_type}> <{param_name}><{/params}>)
{
  <{#has_return_value}>return <{return_converter_cxx_to_c}>(<{/has_return_value}>((<{ns}><{class_name}>_forwarder*)i_cxxptr)-><{class_name}>::<{name}>(<{#params:separator(, )}><{param_as_cxx_type}><{/params}>)<{#has_return_value}>)<{/has_return_value}>;
}
<{/is_non_static}>
<{#is_static}>
<{return_c_type}> <{ns}><{class_name}>_<{name}>(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>)
{
  <{#has_return_value}>return <{return_converter_cxx_to_c}>(<{/has_return_value}><{ns}><{class_name}>_overrider::<{name}>(<{#params:separator(, )}><{param_as_cxx_type}><{/params}>)<{#has_return_value}>)<{/has_return_value}>;
}
<{/is_static}>
<{/methods}>

}
<{/classes}>
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
// <{class_name}>
// ------------------------
class <{class_name}> {
  let _super : COpaquePointer

<{#constructors}>
  init(<{#params:separator(, )}><{param_clean_name}>: <{param_swift_type}><{/params}>) {
    self._super = <{ns}><{class_name}>_allocate(<{#params:separator(, )}><{param_clean_name}><{/params}>)

    <{ns}><{class_name}>_setup_subclass( self._super,
      UnsafePointer(Unmanaged.passUnretained(self).toOpaque())<{#virtual_methods:prefix(,)separator(,\n        )}>
        { ( i_self: UnsafePointer<Void><{#params:prefix(, )separator(, )}><{param_clean_name}>: <{param_swift_c_type}><{/params}>)<{return_swift_c_type:prefix( -> )}> in
          let _self = Unmanaged<<{class_name}>>.fromOpaque( COpaquePointer(i_self) ).takeUnretainedValue()
          <{#has_return_value}>return <{return_converter_swift_to_c}>(<{/has_return_value}>_self.<{name}>(<{#params:separator(, )}><{param_as_swift_type}><{/params}>)<{#has_return_value}>)<{/has_return_value}>
        }
<{/virtual_methods}>
      )
  }
<{/constructors}>

  deinit {
    <{ns}><{class_name}>_destroy(self._super)
  }

<{#methods}>
<{#is_non_static}>
  func <{name}>(<{#params:separator(, )}><{param_clean_name}>: <{param_swift_type}><{/params}>)<{return_swift_type:prefix( -> )}> {
    <{#has_return_value}>return <{return_converter_c_to_swift}>(<{/has_return_value}><{ns}><{class_name}>_<{name}>_forward(self._super<{#params:prefix(, )separator(, )}><{param_clean_name}><{/params}>)<{#has_return_value}>)<{/has_return_value}>
  }
<{/is_non_static}>
<{#is_static}>
  static func <{name}>(<{#params:separator(, )}><{param_clean_name}>: <{param_swift_type}><{/params}>)<{return_swift_type:prefix( -> )}> {
    <{#has_return_value}>return <{return_converter_c_to_swift}>(<{/has_return_value}><{ns}><{class_name}>_<{name}>(<{#params:separator(, )}><{param_clean_name}><{/params}>)<{#has_return_value}>)<{/has_return_value}>
  }
<{/is_static}>
<{/methods}>
}
<{/classes}>
)";

#endif
