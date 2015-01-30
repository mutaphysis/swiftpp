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
#include <set>
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
		const std::string &name() const;
		//! returned type
		const clang::QualType &to() const;
		//! input type
		const clang::QualType &from() const;
	
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
		const std::string &name() const;
		//! parameter type
		const clang::QualType &type() const;
	
		//! attempt to get a clean name, without any notation prefix
		std::string cleanName() const;

		bool operator<( const CXXParam &i_other ) const;
	
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
	
		const std::string &name() const;
		const clang::QualType &returnType() const;
		const std::vector<CXXParam> &params() const;
	
		bool isConstructor() const;
		void setIsConstructor() const;
	
		bool isStatic() const;
		bool isVirtual() const;
		bool isPureVirtual() const;
		bool isConst() const;
	
		bool operator<( const CXXMethod &i_other ) const;
	
	private:
		mutable bool _isConstructor = false;
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
	
		const std::string &name() const;
		const std::set<CXXMethod> &methods() const;
	
	private:
		std::string _name;
		std::set<CXXMethod> _methods;
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
		void addMissingConstructors();
	
		const std::vector<CXXClass> &classes() const;
		const std::vector<TypeConverter> &converters() const;
		const std::vector<std::string> &includesForCXXTypes() const;
	
		std::string outputFolder() const;
	
		const SwiftppOptions &options() const;
	
		/*!
		 @brief Format a file name for #include.
		 
		 @param[in] i_filepath file path
		 @return     suitable for #include
		*/
		std::string formatIncludeFileName( const std::string &i_filepath ) const;

	private:
		SwiftppOptions _options;
		std::vector<TypeConverter> _converters;
		std::vector<CXXClass> _classes;
		std::vector<std::string> _includesForCXXTypes; //!< include paths needed for some user's C++ type definition
};

inline const std::string &TypeConverter::name() const { return _name; }
inline const clang::QualType &TypeConverter::to() const { return _to; }
inline const clang::QualType &TypeConverter::from() const { return _from; }

inline const std::string &CXXParam::name() const { return _name; }
inline const clang::QualType &CXXParam::type() const { return _type; }

inline const std::string &CXXMethod::name() const { return _name; }
inline const clang::QualType &CXXMethod::returnType() const { return _returnType; }
inline const std::vector<CXXParam> &CXXMethod::params() const { return _params; }

inline bool CXXMethod::isConstructor() const { return _isConstructor; }
inline void CXXMethod::setIsConstructor() const { _isConstructor = true; }

inline bool CXXMethod::isStatic() const { return _type == type_t::kStatic; }
inline bool CXXMethod::isVirtual() const { return _type == type_t::kVirtual or _type == type_t::kPureVirtual ; }
inline bool CXXMethod::isPureVirtual() const { return _type == type_t::kPureVirtual ; }
inline bool CXXMethod::isConst() const { return _isConst; }

inline const std::string &CXXClass::name() const { return _name; }
inline const std::set<CXXMethod> &CXXClass::methods() const { return _methods; }

inline const std::vector<CXXClass> &SwiftppData::classes() const { return _classes; }
inline const std::vector<TypeConverter> &SwiftppData::converters() const { return _converters; }
inline const std::vector<std::string> &SwiftppData::includesForCXXTypes() const { return _includesForCXXTypes; }

inline std::string SwiftppData::outputFolder() const { return _options.output; }

inline const SwiftppOptions &SwiftppData::options() const { return _options; }

#endif
