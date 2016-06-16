//
//  SwiftppData.h
//  swiftpp
//
//  Created by Sandy Martel on 2014/08/28.
//  Copyright (c) 2014年. All rights reserved.
//

#ifndef H_SwiftppData
#define H_SwiftppData

#include <string>
#include <vector>
#include <clang/AST/Type.h>

/*!
   @brief A C++/Objective-C data converter.

       A type converter is a function that satisfies the following:
	   	-declared in the swift_converter namespace
		-take only 1 argument
		-return a result (non-void)
*/
class TypeConverter
{
  public:
	TypeConverter( const std::string &i_name, const clang::QualType &i_to, const clang::QualType &i_from );

	//! function name
	inline const std::string &name() const { return _name; }
	//! returned type
	inline const clang::QualType &to() const { return _to; }
	//! input type
	inline const clang::QualType &from() const { return _from; }

  private:
	std::string _name; //!< function name
	clang::QualType _to; //!< returned type
	clang::QualType _from; //!< input type

	std::string _body;
};

/*!
   @brief One parameters to a C++ method.

       A parameter has a type and name.  The type is still fully qualified (const, reference, etc).
*/
class CXXParam
{
  public:
	CXXParam( const clang::QualType &i_type, const std::string &i_name );
	
	//! parameter name
	inline const std::string &name() const { return _name; }
	//! parameter type
	inline const clang::QualType &type() const { return _type; }

	//! attempt to get a clean name, without any notation prefix
	std::string cleanName() const;

	bool operator<( const CXXParam &i_other ) const;
	bool operator==( const CXXParam &i_other ) const;

  private:
	clang::QualType _type; //!< parameter type
	std::string _name; //!< parameter name
};

/*!
   @brief A C++ method.

       Capture relevent info about a C++ method.
*/
class CXXMethod
{
  public:
	enum class type_t { kNormal, kVirtual, kPureVirtual, kStatic };
	enum class access_t { kPublic, kProtected };
	CXXMethod( type_t i_type, access_t i_access,
						bool i_isConst,
						const std::string &i_name,
						const clang::QualType &i_returnType );

	void addParam( const CXXParam &i_param );

	inline const std::string &name() const { return _name; }
	inline const clang::QualType &returnType() const { return _returnType; }
	inline const std::vector<CXXParam> &params() const { return _params; }

	inline bool isStatic() const { return _type == type_t::kStatic; }
	inline bool isVirtual() const { return _type == type_t::kVirtual or _type == type_t::kPureVirtual ; }
	inline bool isPureVirtual() const { return _type == type_t::kPureVirtual ; }
	inline bool isConst() const { return _isConst; }
	
	inline access_t access() const { return _access; }

	bool operator<( const CXXMethod &i_other ) const;
	bool operator==( const CXXMethod &i_other ) const;

  private:
	bool _isConst = false;
	type_t _type = type_t::kNormal; //!< see enum
	access_t _access = access_t::kPublic;
	std::string _name;
	clang::QualType _returnType;
	std::vector<CXXParam> _params;
};

/*!
   @brief A C++ class.

       Capture class name and methods.
*/
class CXXClass
{
  public:
	CXXClass( const std::string &i_name );

	void addMethod( const CXXMethod &i_method );
	void addMissingConstructor();
	void addBase( const std::string &i_base );

	inline const std::string &name() const { return _name; }
	
	inline const std::vector<std::string> &bases() const { return _bases; }

	const std::vector<const CXXMethod *> &constructors() const;
	const std::vector<const CXXMethod *> &methods() const;
	const std::vector<const CXXMethod *> &nonVirtualMethods() const;
	const std::vector<const CXXMethod *> &virtualMethods() const;

  private:
	std::string _name;
	std::vector<std::string> _bases;
	std::vector<CXXMethod> _allMethods;
	mutable bool _valid;
	mutable std::vector<const CXXMethod *> _methods;
	mutable std::vector<const CXXMethod *> _constructors;
	mutable std::vector<const CXXMethod *> _nonVirtualMethods;
	mutable std::vector<const CXXMethod *> _virtualMethods;
	void update() const;
};

class CXXEnum
{
  public:
	CXXEnum( const std::string &i_name, bool i_isSigned );

	void addValue( const std::string &i_name, int64_t i_value );

	inline const std::string &name() const { return _name; }
	inline bool isSigned() const { return _isSigned; }
	inline const std::vector<std::pair<std::string,int64_t>> &values() const { return _values; }

  private:
	std::string _name;
	bool _isSigned = false;
	std::vector<std::pair<std::string,int64_t>> _values;
};

/*!
   @brief options for a customised output.
*/
struct SwiftppOptions
{
	bool usedNamedParams = true; //!< should Objective-C methods have named parameters.
	bool usedFullPathForUserIncludes = true; //!< use fullpath for includes
	std::string output; //!< path to output directory
};

/*!
   @brief All collected data.

       All relevant data are collected in this class and
	   file output is generated from it.
*/
class SwiftppData
{
  public:
	SwiftppData( const SwiftppOptions &i_options );

	void addClass( const CXXClass &i_class );
	void addConverter( const TypeConverter &i_converter );
	void addCXXTypeIncludePath( const std::string &i_fn );
	void addObjCTypeIncludePath( const std::string &i_fn );
	void addMissingConstructors();
	void addEnum( const CXXEnum &i_enum );

	inline const std::vector<CXXClass> &classes() const { return _classes; }
	inline const std::vector<CXXEnum> &enums() const { return _enums; }
	inline const std::vector<TypeConverter> &converters() const { return _converters; }
	inline const std::vector<std::string> &includesForCXXTypes() const { return _includesForCXXTypes; }
	inline const std::vector<std::string> &objCFrameworks() const { return _objCFrameworks; }

	std::vector<std::string> allObjcTypes() const;
	bool anyObjcTypes() const;

	inline std::string outputFolder() const { return _options.output; }

	inline const SwiftppOptions &options() const { return _options; }

	/*!
	 @brief Format a file name for #include.
	 
	 @param[in] i_filepath file path
	 @return     suitable for #include
	*/
	std::string formatIncludeName( const std::string &i_filepath ) const;

  private:
	SwiftppOptions _options;
	std::vector<TypeConverter> _converters;
	std::vector<CXXClass> _classes;
	std::vector<CXXEnum> _enums;
	std::vector<std::string> _includesForCXXTypes; //!< include paths needed for some user's C++ type definition
	std::vector<std::string> _objCFrameworks; //!< Objective-C frameworks needed for some type definition
};

#endif
