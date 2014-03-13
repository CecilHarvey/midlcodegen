#ifndef _IDL_H_
#define _IDL_H_

#include <string.h>

typedef enum tagIDL_IdentifierType
{
  IDENTIFIER_UNKNOWN = 0,

  IDENTIFIER_ENUM,
  IDENTIFIER_STRUCT,
  IDENTIFIER_UNION,
  IDENTIFIER_INTERFACE,
  IDENTIFIER_COCLASS
} IDL_IdentifierType;

typedef enum tagIDL_ParamDir
{
  IN = 1, OUT
} IDL_ParamDir;

#ifdef __cplusplus

#include <vector>
#include <map>
#include <string>
#include <set>
#include <algorithm>

typedef struct tagIDL_Enum_Member
{
  std::string       name;
  int               value;
} IDL_Enum_Member;

typedef struct tagIDL_Enum
{
  std::string       name;
  std::string       type_name;
  std::vector< IDL_Enum_Member > members;
} IDL_Enum;

typedef struct tagIDL_Struct_Member
{
  std::string    name;
  std::string    type;
  std::string    size_is;
  int            ptr_level;
  int            array_size;
} IDL_Struct_Member;

typedef struct tagIDL_Struct
{
  std::string    name;
  std::string    type_name;
  std::string    ptr_type_name;
  std::vector< IDL_Struct_Member > members;

  const IDL_Struct_Member *GetMember(const char *szName) const
  {
    int i;
    for (i = 0; i < members.size(); i++)
      {
	if (strcmp(members[i].name.c_str(), szName) == 0)
	  {
	    return &members[i];
	  }
      }
    return NULL;
  }
} IDL_Struct;

typedef struct tagIDL_CoClass
{
  std::string     name;
  std::string     uuid;
} IDL_CoClass;

typedef struct tagIDL_Union_Member
{
  std::string    name;
  std::string    type;
  std::string    size_is;
  int            ptr_level;
  int            array_size;
  std::vector< int >            case_value;
} IDL_Union_Member;

typedef struct tagIDL_Union
{
  std::string    name;
  std::string    type_name;
  std::vector< IDL_Union_Member > members;
} IDL_Union;

typedef struct tagIDL_Interface_Method_Param
{
  std::string    name;
  std::string    type;
  std::string    size_is;
  int            size_is_ptr_level;
  std::string    iid_is;
  int            ptr_level;
  int            array_size;
  IDL_ParamDir   direction;
} IDL_Interface_Method_Param;

typedef struct tagIDL_Interface_Method
{
  std::string    name;
  std::vector< IDL_Interface_Method_Param > params;

  const IDL_Interface_Method_Param *GetParam(const char *szName) const
  {
    int i;
    for (i = 0; i < params.size(); i++)
      {
	if (strcmp(params[i].name.c_str(), szName) == 0)
	  {
	    return &params[i];
	  }
      }
    return NULL;
  }

  const IDL_Interface_Method_Param *GetWhoseSizeIs(const char *szName) const
  {
    int i;
    for (i = 0; i < params.size(); i++)
      {
	if (strcmp(params[i].size_is.c_str(), szName) == 0)
	  {
	    return &params[i];
	  }
      }
    return NULL;
  }
} IDL_Interface_Method;

typedef struct tagIDL_Interface
{
  std::string    name;
  std::string    uuid;
  std::vector< IDL_Interface_Method >   methods;

  const IDL_Interface_Method* GetMethod(const char *szName) const
  {
    int i;
    for (i = 0; i < methods.size(); i++)
      {
	if (strcmp(methods[i].name.c_str(), szName) == 0)
	  {
	    return &methods[i];
	  }
      }
    return NULL;
  }
} IDL_Interface;

class IDLParser
{
public:
  void Init();
  void AddEnumValue(const char *szEnumName, int iValue);
  int AddEnumRef(const char *szEnumName, const char *szRefEnumName);
  void AddEnumDef(const char *szName, const char *szTypeName);

  void AddForwardDecl(const char *szName);
  void AddStructTypeDef(const char *szStructName, const char *szType);
  void AddStructMember(const char *szName, const char *szType, const char *szSizeRef, int iPtrLevel, int iArraySize);
  void AddStructDef(const char *szName, const char *szTypeName, const char *szPtrTypeName);

  void AddUnionMember(const char *szName, const char *szType, const char *szSizeRef, int iPtrLevel, int iArraySize, const std::vector<int> &vecCaseValue);
  void AddUnionDef(const char *szName, const char *szTypeName);

  void AddInterfaceMethodParam(const char *szName, const char *szType, const char *szSizeRef, int iSizeRefPtrLevel, const char *szIIDRef, int iPtrLevel, int iArraySize, IDL_ParamDir enDir);
  void AddInterfaceMethodDef(const char *szName);
  void AddInterfaceDef(const char *szName, const char *szIID);

  int GetEnumValue(const char *szEnumname);
  IDL_IdentifierType GetIdentifierType(const char *szIdentifier);
  const char *GetTypeInStruct(const char *szStruct, const char *szMemberName);
  const IDL_Interface *GetInterface(const char *szName);
  const IDL_Struct *GetStruct(const char *szName);
  const IDL_Union *GetUnion(const char *szName);
  void AddCoClass(const char *szName, const char *szCLSID);

  void DumpToCHeader(const char *szFileName);

  //protected:
  std::map< std::string, IDL_IdentifierType > identifierTypeTable;
  std::map< std::string, int > enumTable;

  std::vector< IDL_Struct > structs;
  std::vector< IDL_Union > unions;
  std::vector< IDL_Enum > enums;
  std::vector< IDL_Interface > interfaces;
  std::vector< IDL_CoClass > coclasses;
  std::vector< std::string > forward_decls;
  std::map< std::string, std::string > struct_typedef;

private:
  // temporary workarea
  std::vector< IDL_Struct_Member > tmp_struct_members;
  std::vector< IDL_Union_Member > tmp_union_members;
  std::vector< IDL_Enum_Member > tmp_enum_members;
  std::vector< IDL_Interface_Method_Param > tmp_method_params;
  std::vector< IDL_Interface_Method > tmp_methods;

  void DumpUnion(const IDL_Union &idlUnion, FILE *fp);
};

extern IDLParser idl;

extern "C"
{

#endif

//=================================================================================

void IDL_Init();

void IDL_AddEnumValue(const char *szEnumName, int iValue);
int IDL_AddEnumRef(const char *szEnumName, const char *szRefEnumName);
void IDL_AddEnumDef(const char *szName, const char *szTypeName);

void IDL_AddStructTypeDef(const char *szStructName, const char *szType);
void IDL_AddForwardDecl(const char *szName);
void IDL_AddStructMember(const char *szName, const char *szType, const char *szSizeRef, int iPtrLevel, int iArraySize);
void IDL_AddStructDef(const char *szName, const char *szTypeName, const char *szPtrTypeName);

int IDL_GetEnumValue(const char *szEnumName);
IDL_IdentifierType IDL_GetIdentifierType(const char *szIdentifier);

void IDL_AddUnionMember(const char *szName, const char *szType, const char *szSizeRef, int iPtrLevel, int iArraySize, int *piCaseValue, int iCaseValueSize);
void IDL_AddUnionDef(const char *szName, const char *szTypeName);

void IDL_AddInterfaceMethodParam(const char *szName, const char *szType, const char *szSizeRef, int iSizeRefPtrLevel, const char *szIIDRef, int iPtrLevel, int iArraySize, IDL_ParamDir enDir);
void IDL_AddInterfaceMethodDef(const char *szName);
void IDL_AddInterfaceDef(const char *szName, const char *szIID);

void IDL_AddCoClass(const char *szName, const char *szCLSID);

void IDL_DumpToCHeader(const char *szFileName);

//=================================================================================

#ifdef __cplusplus
}
#endif

#endif



