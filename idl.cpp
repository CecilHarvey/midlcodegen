#include "idl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning (disable : 4996)
#pragma warning (disable : 4018)
#pragma warning (disable : 4244)
#endif

void IDLParser::Init()
{
  identifierTypeTable.clear();
  enumTable.clear();

  structs.clear();
  unions.clear();
  enums.clear();
  interfaces.clear();
  coclasses.clear();
  forward_decls.clear();
  struct_typedef.clear();

  tmp_struct_members.clear();
  tmp_enum_members.clear();
  tmp_union_members.clear();
  tmp_method_params.clear();
  tmp_methods.clear();
}

void IDLParser::AddEnumValue(const char *szEnumName, int iValue)
{
  if (enumTable.find(szEnumName) != enumTable.end())
    {
      fprintf(stderr, "IDLParser::AddEnumValue: Duplicate enum value\n");
      return;
    }

  enumTable[szEnumName] = iValue;

  IDL_Enum_Member member;
  member.name = szEnumName;
  member.value = iValue;

  tmp_enum_members.push_back(member);
}

int IDLParser::AddEnumRef(const char *szEnumName, const char *szRefEnumName)
{
  int value = -1;

  std::map< std::string, int >::iterator it = enumTable.find(szRefEnumName);
  if (it != enumTable.end())
    {
      value = it->second;
    }

  enumTable[szEnumName] = value;

  return value;
}

void IDLParser::AddEnumDef(const char *szName, const char *szTypeName)
{
  if (identifierTypeTable.find(szTypeName) == identifierTypeTable.end())
    {
      identifierTypeTable[szTypeName] = IDENTIFIER_ENUM;
    }

  IDL_Enum e;
  e.name = szName;
  e.type_name = szTypeName;
  e.members.swap(tmp_enum_members);

  enums.push_back(e);
}

int IDLParser::GetEnumValue(const char *szEnumName)
{
  std::map< std::string, int >::iterator it = enumTable.find(szEnumName);
  if (it == enumTable.end())
    {
      return 0;
    }
  return it->second;
}

void IDLParser::AddStructTypeDef(const char *szStructName, const char *szType)
{
  struct_typedef[szStructName] = szType;
}

void IDLParser::AddForwardDecl(const char *szName)
{
  forward_decls.push_back(szName);
}

void IDLParser::AddStructMember(const char *szName, const char *szType, const char *szSizeRef, int iPtrLevel, int iArraySize)
{
  IDL_Struct_Member member;

  member.name = szName;
  member.type = szType;
  member.size_is = szSizeRef;
  member.ptr_level = iPtrLevel;
  member.array_size = iArraySize;

  tmp_struct_members.push_back(member);
}

void IDLParser::AddStructDef(const char *szName, const char *szTypeName, const char *szPtrTypeName)
{
  IDL_Struct st;
  st.name = szName;
  st.type_name = szTypeName;
  st.ptr_type_name = szPtrTypeName;
  st.members.swap(tmp_struct_members);

  structs.push_back(st);

  if (identifierTypeTable.find(szTypeName) == identifierTypeTable.end())
    {
      identifierTypeTable[szTypeName] = IDENTIFIER_STRUCT;
    }
}

void IDLParser::AddUnionMember(const char *szName, const char *szType, const char *szSizeRef, int iPtrLevel, int iArraySize, const std::vector<int> &vecCaseValue)
{
  IDL_Union_Member member;

  member.name = szName;
  member.type = szType;
  member.size_is = szSizeRef;
  member.ptr_level = iPtrLevel;
  member.array_size = iArraySize;
  member.case_value = vecCaseValue;

  tmp_union_members.push_back(member);
}

void IDLParser::AddUnionDef(const char *szName, const char *szTypeName)
{
  IDL_Union st;
  st.name = szName;
  st.type_name = szTypeName;
  st.members.swap(tmp_union_members);

  unions.push_back(st);

  if (identifierTypeTable.find(szName) == identifierTypeTable.end())
    {
      identifierTypeTable[szName] = IDENTIFIER_UNION;
    }
}

void IDLParser::AddInterfaceMethodParam(const char *szName, const char *szType, const char *szSizeRef, int iSizeRefPtrLevel, const char *szIIDRef, int iPtrLevel, int iArraySize, IDL_ParamDir enDir)
{
  IDL_Interface_Method_Param param;

  param.name = szName;
  param.type = szType;
  param.size_is = szSizeRef;
  param.size_is_ptr_level = iSizeRefPtrLevel;
  param.iid_is = szIIDRef;
  param.ptr_level = iPtrLevel;
  param.array_size = iArraySize;
  param.direction = enDir;

  tmp_method_params.push_back(param);
}

void IDLParser::AddInterfaceMethodDef(const char *szName)
{
  IDL_Interface_Method method;
  method.name = szName;
  method.params.swap(tmp_method_params);

  tmp_methods.push_back(method);
}

void IDLParser::AddInterfaceDef(const char *szName, const char *szIID)
{
  IDL_Interface interf;

  interf.name = szName;
  interf.uuid = szIID;
  interf.methods.swap(tmp_methods);

  interfaces.push_back(interf);

  if (identifierTypeTable.find(szName) == identifierTypeTable.end())
    {
      identifierTypeTable[szName] = IDENTIFIER_INTERFACE;
    }
}

IDL_IdentifierType IDLParser::GetIdentifierType(const char *szIdentifier)
{
  std::map< std::string, IDL_IdentifierType >::iterator it = identifierTypeTable.find(szIdentifier);
  if (it != identifierTypeTable.end())
    {
      return it->second;
    }
  return IDENTIFIER_UNKNOWN;
}

const char *IDLParser::GetTypeInStruct(const char *szStruct, const char *szMemberName)
{
  int i, j;

  for (i = 0; i < structs.size(); i++)
    {
      if (strcmp(structs[i].type_name.c_str(), szStruct) == 0)
	{
	  for (j = 0; j < structs[i].members.size(); j++)
	    {
	      if (strcmp(structs[i].members[j].name.c_str(), szMemberName) == 0)
		{
		  return structs[i].members[j].type.c_str();
		}
	    }
	  break;
	}
    }

  return NULL;
}

const IDL_Interface *IDLParser::GetInterface(const char *szName)
{
  int i;
  for (i = 0; i < idl.interfaces.size(); i++)
    {
      if (strcmp(idl.interfaces[i].name.c_str(), szName) == 0)
	{
	  return &idl.interfaces[i];
	}
    }
  return NULL;
}

const IDL_Struct *IDLParser::GetStruct(const char *szName)
{
  int i;
  for (i = 0; i < idl.structs.size(); i++)
    {
      if (strcmp(idl.structs[i].type_name.c_str(), szName) == 0)
	{
	  return &idl.structs[i];
	}
    }
  return NULL;
}

const IDL_Union *IDLParser::GetUnion(const char *szName)
{
  int i;
  for (i = 0; i < idl.unions.size(); i++)
    {
      if (strcmp(idl.unions[i].type_name.c_str(), szName) == 0)
	{
	  return &idl.unions[i];
	}
    }
  return NULL;
}

static unsigned int GetHashValue(const char *sz)
{
  unsigned int hash = 0;

  while (*sz)
    {
      hash += (unsigned int)*sz;
      hash += (hash << 10);
      hash ^= (hash >> 6);
      sz++;
    }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash;
}

void IDLParser::AddCoClass(const char *szName, const char *szCLSID)
{
  IDL_CoClass c;
  c.name = szName;
  c.uuid = szCLSID;
  coclasses.push_back(c);
}

void IDLParser::DumpUnion(const IDL_Union &idlUnion, FILE *fp)
{
  int j, k;
  fprintf(fp, "typedef union %s {\n", idlUnion.name.c_str());

  for (j = 0; j < idlUnion.members.size(); j++)
    {
      fprintf(fp, "\t/*case(");
      for (k = 0; k < idlUnion.members[j].case_value.size(); k++)
	{
	  if (k > 0) fprintf(fp, ", ");
	  fprintf(fp, "%d", idlUnion.members[j].case_value[k]);
	}
      fprintf(fp, ")*/");

      if (idlUnion.members[j].size_is.length() > 0)
	{
	  fprintf(fp, "/*size_is(%s)*/", idlUnion.members[j].size_is.c_str());
	}

      fprintf(fp, "%s", idlUnion.members[j].type.c_str());

      for (k = 0; k < idlUnion.members[j].ptr_level; k++)
	{
	  fprintf(fp, "*");
	}

      fprintf(fp, "\t%s", idlUnion.members[j].name.c_str());

      if (idlUnion.members[j].array_size > 0)
	{
	  fprintf(fp, "[%d]", idlUnion.members[j].array_size);
	}

      fprintf(fp, ";\n");
    }

  fprintf(fp, "} %s;\n\n", idlUnion.type_name.c_str());
}

void IDLParser::DumpToCHeader(const char *szFileName)
{
  FILE *fp = fopen(szFileName, "w");
  if (fp == NULL)
    {
      return;
    }

  fprintf(fp, "// This file is generated automatically\n\n");

  char identifier[256];
  sprintf(identifier, "IDL_CONVERT_%.8X_DEFINED", GetHashValue(szFileName));

  fprintf(fp, "#ifndef %s\n", identifier);
  fprintf(fp, "#define %s\n\n", identifier);

  int i, j, k, l;

  std::set<std::string> dumpedUnion;

  // Enums
  for (i = 0; i < enums.size(); i++)
    {
      fprintf(fp, "typedef enum %s {\n", enums[i].name.c_str());

      for (j = 0; j < enums[i].members.size(); j++)
	{
	  fprintf(fp, "\t%s = %d,\n", enums[i].members[j].name.c_str(), enums[i].members[j].value);
	}

      fprintf(fp, "} %s;\n\n", enums[i].type_name.c_str());
    }

  fprintf(fp, "\n//==================================================================\n\n");

  // typedefs
  std::map< std::string, std::string >::iterator it = struct_typedef.begin();
  while (it != struct_typedef.end())
    {
      fprintf(fp, "typedef struct %s %s;\n", it->first.c_str(), it->second.c_str());
      it++;
    }

  // forward declarations
  for (i = 0; i < forward_decls.size(); i++)
    {
      fprintf(fp, "struct %s;\n", forward_decls[i].c_str());
    }
  fprintf(fp, "\n");

  // Structs
  for (i = 0; i < structs.size(); i++)
    {
      for (j = 0; j < structs[i].members.size(); j++)
	{
	  const IDL_Union *p;
	  if ((p = GetUnion(structs[i].members[j].type.c_str())) != NULL)
	    {
	      DumpUnion(*p, fp);
	      dumpedUnion.insert(p->type_name);
	    }
	}

      fprintf(fp, "typedef struct %s {\n", structs[i].name.c_str());

      for (j = 0; j < structs[i].members.size(); j++)
	{
	  fprintf(fp, "\t");

	  if (structs[i].members[j].size_is.length() > 0)
	    {
	      fprintf(fp, "/*size_is(%s)*/", structs[i].members[j].size_is.c_str());
	    }

	  fprintf(fp, "%s", structs[i].members[j].type.c_str());

	  for (k = 0; k < structs[i].members[j].ptr_level; k++)
	    {
	      fprintf(fp, "*");
	    }

	  fprintf(fp, "\t%s", structs[i].members[j].name.c_str());

	  if (structs[i].members[j].array_size > 0)
	    {
	      fprintf(fp, "[%d]", structs[i].members[j].array_size);
	    }

	  fprintf(fp, ";\n");
	}

      if (structs[i].ptr_type_name.length() > 0)
	{
	  fprintf(fp, "} %s, *%s;\n\n", structs[i].type_name.c_str(), structs[i].ptr_type_name.c_str());
	}
      else
	{
	  fprintf(fp, "} %s;\n\n", structs[i].type_name.c_str());
	}
    }

  fprintf(fp, "\n//==================================================================\n\n");

  // Unions
  for (i = 0; i < unions.size(); i++)
    {
      if (dumpedUnion.find(unions[i].type_name) != dumpedUnion.end())
	{
	  continue;
	}
      DumpUnion(unions[i], fp);
    }

  fprintf(fp, "\n//==================================================================\n\n");

  // Interfaces
  for (i = 0; i < interfaces.size(); i++)
    {
      unsigned int iid1 = 0;
      unsigned int iid2 = 0, iid3 = 0;
      unsigned int iid4 = 0, iid5 = 0, iid6 = 0, iid7 = 0, iid8 = 0, iid9 = 0, iid10 = 0, iid11 = 0;

      sscanf(interfaces[i].uuid.c_str(), "%8x-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x",
	     &iid1, &iid2, &iid3, &iid4, &iid5, &iid6, &iid7, &iid8, &iid9, &iid10, &iid11);

      fprintf(fp, "DEFINE_IID( %s, 0x%.8x, 0x%.4x, 0x%.4x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x );\n",
	      interfaces[i].name.c_str(), iid1, iid2, iid3, iid4, iid5, iid6, iid7, iid8, iid9, iid10, iid11);

      fprintf(fp, "class %s : public IUnknown\n{\npublic:\n", interfaces[i].name.c_str());

      for (j = 0; j < interfaces[i].methods.size(); j++)
	{
	  fprintf(fp, "\tvirtual HRESULT %s(\n", interfaces[i].methods[j].name.c_str());

	  for (k = 0; k < interfaces[i].methods[j].params.size(); k++)
	    {
	      if (k > 0)
		{
		  fprintf(fp, ",\n");
		}
	      fprintf(fp, "\t\t");

	      if (interfaces[i].methods[j].params[k].direction == IN)
		{
		  fprintf(fp, "/*in*/");
		}
	      else if (interfaces[i].methods[j].params[k].direction == OUT)
		{
		  fprintf(fp, "/*out*/");
		}
	      else
		{
		  fprintf(fp, "/*INVALID_DIRECTION*/");
		}

	      if (interfaces[i].methods[j].params[k].size_is.length() > 0)
		{
		  fprintf(fp, "/*size_is(");
		  for (l = 0; l < interfaces[i].methods[j].params[k].size_is_ptr_level; l++)
		    {
		      fprintf(fp, "*");
		    }
		  fprintf(fp, "%s)*/", interfaces[i].methods[j].params[k].size_is.c_str());
		}

	      if (interfaces[i].methods[j].params[k].iid_is.length() > 0)
		{
		  fprintf(fp, "/*iid_is(%s)*/", interfaces[i].methods[j].params[k].iid_is.c_str());
		}

	      if (strcmp(interfaces[i].methods[j].params[k].type.c_str(), "REFIID") == 0)
		{
		  // translate REFIID to GUID*
		  fprintf(fp, "GUID *");
		}
	      else if (strcmp(interfaces[i].methods[j].params[k].type.c_str(), "boolean") == 0)
		{
		  // translate boolean to char
		  fprintf(fp, "char");
		}
	      else
		{
		  fprintf(fp, "%s", interfaces[i].methods[j].params[k].type.c_str());
		}

	      for (l = 0; l < interfaces[i].methods[j].params[k].ptr_level; l++)
		{
		  fprintf(fp, "*");
		}
	      fprintf(fp, "\t%s", interfaces[i].methods[j].params[k].name.c_str());

	      if (interfaces[i].methods[j].params[k].array_size > 0)
		{
		  fprintf(fp, "[%d]", interfaces[i].methods[j].params[k].array_size);
		}
	    }

	  if (interfaces[i].methods[j].params.size() > 0)
	    {
	      fprintf(fp, "\n");
	    }
	  fprintf(fp, "\t)=0;\n\n");
	}

      fprintf(fp, "};\n\n");
    }

  fprintf(fp, "\n//==================================================================\n\n");

  // coclasses
  for (i = 0; i < coclasses.size(); i++)
    {
      fprintf(fp, "DEFINE_CLSID(%s, ", coclasses[i].name.c_str());
      fprintf(fp, "0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s);\n",
	      coclasses[i].uuid.substr(0, 8).c_str(),
	      coclasses[i].uuid.substr(9, 4).c_str(),
	      coclasses[i].uuid.substr(14, 4).c_str(),
	      coclasses[i].uuid.substr(19, 2).c_str(),
	      coclasses[i].uuid.substr(21, 2).c_str(),
	      coclasses[i].uuid.substr(24, 2).c_str(),
	      coclasses[i].uuid.substr(26, 2).c_str(),
	      coclasses[i].uuid.substr(28, 2).c_str(),
	      coclasses[i].uuid.substr(30, 2).c_str(),
	      coclasses[i].uuid.substr(32, 2).c_str(),
	      coclasses[i].uuid.substr(34, 2).c_str());
    }

  fprintf(fp, "\n//==================================================================\n\n");

  fprintf(fp, "#endif\n");
  fclose(fp);
}

IDLParser idl;

//=================================================================================
// C-compatible APIs for lex/yacc parser
//=================================================================================

void IDL_Init()
{
  idl.Init();
}

void IDL_AddEnumValue(const char *szEnumName, int iValue)
{
  idl.AddEnumValue(szEnumName, iValue);
}

int IDL_AddEnumRef(const char *szEnumName, const char *szRefEnumName)
{
  return idl.AddEnumRef(szEnumName, szRefEnumName);
}

void IDL_AddEnumDef(const char *szName, const char *szTypeName)
{
  idl.AddEnumDef(szName, szTypeName);
}

int IDL_GetEnumValue(const char *szEnumName)
{
  return idl.GetEnumValue(szEnumName);
}

void IDL_AddStructTypeDef(const char *szStructName, const char *szType)
{
  return idl.AddStructTypeDef(szStructName, szType);
}

void IDL_AddForwardDecl(const char *szName)
{
  idl.AddForwardDecl(szName);
}

void IDL_AddStructMember(const char *szName, const char *szType, const char *szSizeRef, int iPtrLevel, int iArraySize)
{
  idl.AddStructMember(szName, szType, szSizeRef, iPtrLevel, iArraySize);
}

void IDL_AddStructDef(const char *szName, const char *szTypeName, const char *szPtrTypeName)
{
  idl.AddStructDef(szName, szTypeName, szPtrTypeName);
}

void IDL_AddUnionMember(const char *szName, const char *szType, const char *szSizeRef, int iPtrLevel, int iArraySize, int *piCaseValue, int iCaseValueSize)
{
  std::vector<int> v;
  for (int i = 0; i < iCaseValueSize; i++)
    {
      v.push_back(piCaseValue[i]);
    }

  idl.AddUnionMember(szName, szType, szSizeRef, iPtrLevel, iArraySize, v);
}

void IDL_AddUnionDef(const char *szName, const char *szTypeName)
{
  idl.AddUnionDef(szName, szTypeName);
}

void IDL_AddInterfaceMethodParam(const char *szName, const char *szType, const char *szSizeRef, int iSizeRefPtrLevel, const char *szIIDRef, int iPtrLevel, int iArraySize, IDL_ParamDir enDir)
{
  idl.AddInterfaceMethodParam(szName, szType, szSizeRef, iSizeRefPtrLevel, szIIDRef, iPtrLevel, iArraySize, enDir);
}

void IDL_AddInterfaceMethodDef(const char *szName)
{
  idl.AddInterfaceMethodDef(szName);
}

void IDL_AddInterfaceDef(const char *szName, const char *szIID)
{
  idl.AddInterfaceDef(szName, szIID);
}

IDL_IdentifierType IDL_GetIdentifierType(const char *szIdentifier)
{
  return idl.GetIdentifierType(szIdentifier);
}

void IDL_AddCoClass(const char *szName, const char *szCLSID)
{
  idl.AddCoClass(szName, szCLSID);
}

void IDL_DumpToCHeader(const char *szFileName)
{
  idl.DumpToCHeader(szFileName);
}
