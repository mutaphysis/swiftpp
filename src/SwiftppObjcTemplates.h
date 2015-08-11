//
//  SwiftppObjcTemplates.h
//  swiftpp
//
//  Created by Sandy Martel on 3/02/2015.
//  Copyright (c) 2015 Sandy Martel. All rights reserved.
//

#ifndef H_SwiftppObjcTemplates
#define H_SwiftppObjcTemplates

const char kCXX_OBJC_PROTOCOLS_H_TEMPLATE[] = R"(
// generated cxx-objc-protocols.h
//  pure Objective-C, cannot contain any C++

#ifndef H_CXX_OBJC_PROTOCOLS
#define H_CXX_OBJC_PROTOCOLS

#import <Foundation/Foundation.h>

<{#objc_class_proto}>@class <{objc_class_proto_name}>;
<{/objc_class_proto}>
<{#has_enums}>// enums definition
<{#enums}>
typedef NS_ENUM(<{enum_name}>_,<{enum_type}>)
{
<{#enum_values}>
<{enum_name}>_<{enum_value_name}>,
<{/enum_values}>
};<{/enums}><{/has_enums}>

// Objective-C proxy protocols for each classes

<{#classes}>
@protocol <{class_name}>_protocol
<{#methods}>
<{objc_method_decl}>;
<{/methods}>
@end
<{/classes}>
#endif
)";

const char kCXX_OBJC_PROXIES_H_TEMPLATE[] = R"(
// generated cxx-objc-proxies.h
//  pure Objective-C, cannot contain any C++

#ifndef H_CXX_OBJC_PROXIES
#define H_CXX_OBJC_PROXIES

#import "cxx-objc-protocols.h"

// Objective-C proxies for each classes

<{#classes}>
@interface <{class_name}> : NSObject<<{class_name}>_protocol>
{ void *_ptr; }
@end
<{/classes}>
#endif
)";

const char kCXX_OBJC_PROXIES_MM_TEMPLATE[] = R"(
// generated cxx-objc-proxies.mm

#import "cxx-objc-proxies.h"
#include <string>
#include <vector>
#include <map>
#include <cassert>

<{#includes_for_cxx_types}>
#include "<{include_name}>"
<{/includes_for_cxx_types}>
namespace swift_converter
{
<{#converters}>  <{converter_decl}>;
<{/converters}>}

<{#classes}>
//********************************
// <{class_name}>
//********************************

// C-style

class <{class_name}>_subclass;
void <{class_name}>_subclass_delete( <{class_name}>_subclass *i_this );
<{#methods}>
<{c_proxy_method_decl}>;
<{/methods}>
// Objective-C proxy

#define _this ((<{class_name}>_subclass *)_ptr)

@implementation <{class_name}>

- (void)dealloc
{
  <{class_name}>_subclass_delete( _this );
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}
<{#methods}>
<{objc_method_impl}>

<{/methods}>
@end

#undef _this
<{/classes}>
)";

const char kCXX_SUBCLASSES_MM_TEMPLATE[] = R"(
// generated cxx-subclasses.mm

#import "cxx-objc-protocols.h"
#include "<{bridge_include}>"

template<typename T>
struct LinkSaver
{
  T saved, &link;
  LinkSaver( T &i_link ) : saved( i_link ), link( i_link ) { link = nil; }
  ~LinkSaver() { link = saved; }
};

// the wrapping sub-classes

<{#classes}>
class <{class_name}>_subclass : public <{class_name}>
{
public:
  id<<{class_name}>_protocol> _link;

<{#methods}>
<{cpp_method_impl}>
<{/methods}>};

<{/classes}>
// the c implementations

<{#classes}>
void <{class_name}>_subclass_delete( <{class_name}>_subclass *i_this )
{
  delete i_this;
}

<{#methods}>
<{c_proxy_method_impl}>

<{/methods}>
<{/classes}>
)";


#endif
