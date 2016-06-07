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
// generated cxx-bridge.h
//  pure C, cannot contain any C++

#ifndef H_CXX_BRIDGE
#define H_CXX_BRIDGE

<{objc_forward_decl}>
#ifdef __cplusplus
extern "C" {
#endif

<{#classes}>
<{#constructors}>
const void *<{class_name}>_proxy_allocate(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>);<{/constructors}>
void <{class_name}>_proxy_destroy(const void *i_cxxptr);
void <{class_name}>_proxy_init(const void *i_cxxptr, const void *i_self<{#virtual_methods:separator(, )prefix(, )}><{return_c_type}> (*i_<{name}>_callback)(const void * ,<{#params:separator(, )}><{param_c_type}><{/params}>)<{/virtual_methods}>);
<{#methods}>
<{return_c_type}> <{class_name}>_proxy_<{name}>(const void *i_cxxptr, <{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>);
<{/methods}><{/classes}>
#ifdef __cplusplus
}
#endif

#endif
)";

const char kCXX_BRIDGE_CPP_TEMPLATE[] = R"(
// generated cxx-bridge.cpp

#include "<{bridge_include}>"

<{#classes}>
class <{class_name}>_proxy_class : public <{class_name}>
{
public:
  const void *_self = nullptr;
<{#virtual_methods}>
  <{return_c_type}> (*_<{name}>_callback)(const void * ,<{#params:separator(, )}><{param_c_type}><{/params}>);<{/virtual_methods}>

<{#constructors}>
  <{class_name}>_proxy_class(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>) : <{class_name}>(<{#params:separator(, )}><{param_name}><{/params}>){}<{/constructors}>

<{#virtual_methods}>
  virtual <{return_cxx_type}> <{name}>(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>)
  {
    (*_<{name}>_callback)(_self, <{#params:separator(, )}><{param_as_c_type}><{/params}>);
  }
<{/virtual_methods}>
<{#methods}>
  inline <{return_cxx_type}> <{name}>_forward(<{#params:separator(, )}><{param_cxx_type}> <{param_name}><{/params}>)
  {
    <{class_name}>::<{name}>(<{#params:separator(, )}><{param_name}><{/params}>);
  }
<{/methods}>
};

// the c implementations

#ifdef __cplusplus
extern "C" {
#endif

<{#constructors}>
const void *<{class_name}>_proxy_allocate(<{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>)
{
  return new <{class_name}>_proxy_class(<{#params:separator(, )}><{param_name}><{/params}>);
}<{/constructors}>
void <{class_name}>_proxy_destroy(const void *i_cxxptr)
{
  delete ((<{class_name}>_proxy_class*)i_cxxptr);
}
void <{class_name}>_proxy_init(const void *i_cxxptr, const void *i_self<{#virtual_methods:separator(, )prefix(, )}><{return_c_type}> (*i_<{name}>_callback)(const void * ,<{#params:separator(, )}><{param_c_type}><{/params}>)<{/virtual_methods}>)
{
  ((<{class_name}>_proxy_class*)i_cxxptr)->_self = i_self;
<{#virtual_methods}>
  ((<{class_name}>_proxy_class*)i_cxxptr)->_<{name}>_callback = i_<{name}>_callback;
<{/virtual_methods}>}

<{#methods}>
<{return_c_type}> <{class_name}>_proxy_<{name}>(const void *i_cxxptr, <{#params:separator(, )}><{param_c_type}> <{param_name}><{/params}>)
{
	((<{class_name}>_proxy_class*)i_cxxptr)-><{name}>_forward(<{#params:separator(, )}><{param_as_cxx_type}><{/params}>);
}
<{/methods}>
#ifdef __cplusplus
}
#endif<{/classes}>
)";

const char kCXX_BRIDGE_SWIFT_TEMPLATE[] = R"(
// generated cxx-bridge.swift

import Cocoa // fixme

<{#classes}>
class <{class_name}>
{
  let _super : UnsafePointer<Void>
	
<{#constructors}>
  init(<{#params:separator(, )}><{param_clean_name}>: <{param_swift_type}><{/params}>)
  {
    self._super = <{class_name}>_proxy_allocate(<{#params:separator(, )}><{param_clean_name}><{/params}>)
	
    <{class_name}>_proxy_init( self._super,
      UnsafePointer(Unmanaged.passUnretained(self).toOpaque())<{#virtual_methods:prefix(,\n        )separator(,\n        )}>
{ ( i_self: UnsafePointer<Void><{#params:prefix(, )separator(, )}><{param_clean_name}>: <{param_swift_c_type}><{/params}>) in
          let _self = Unmanaged<<{class_name}>>.fromOpaque( COpaquePointer(i_self) ).takeUnretainedValue()
          _self.<{name}>(<{#params:separator(, )}><{param_as_swift_type}><{/params}>)
        }<{/virtual_methods}>
	  )
  }<{/constructors}>

  deinit
  {
    <{class_name}>_proxy_destroy(self._super)
  }

<{#methods}>
  func <{name}>(<{#params:separator(, )}><{param_clean_name}>: <{param_swift_type}><{/params}>)
  {
	  <{class_name}>_proxy_<{name}>(self._super<{#params:separator(, )prefix(, )}><{param_clean_name}><{/params}>)
  }
<{/methods}>
}<{/classes}>
)";

#endif
