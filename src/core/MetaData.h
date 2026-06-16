#pragma once

#include "D2Types.h"
#include <QString>
#include <string>
#include <vector>
#include <list>

// 物品元数据
struct CItemMetaData
{
	union {
		BYTE sTypeName[4];	// 物品名字（唯一ID）, 如"elx "
		DWORD dwTypeID;
	};
	WORD PicIndex = 0;
	WORD NameIndex = 0;
	BYTE Equip = 0;
	BYTE Range = 0;

	/** 返回物品类型代码（4字符，如 "elx", "hpo"） */
	const char* typeCode() const { return reinterpret_cast<const char*>(sTypeName); }
	BOOL Simple = FALSE;
	BOOL Normal = FALSE;
	BOOL White = FALSE;
	BOOL IsNew = FALSE;
	BOOL HasDef = FALSE;
	BOOL HasDur = FALSE;
	BOOL IsStacked = FALSE;
	BOOL HasMonsterID = FALSE;
	BOOL IsCharm = FALSE;
	UINT SpellId = 0;
	BOOL IsUnique = FALSE;
	BOOL IsCraft = FALSE;
	BOOL IsGem = FALSE;
	UINT Damage1Min = 0;
	UINT Damage1Max = 0;
	UINT Damage2Min = 0;
	UINT Damage2Max = 0;
	UINT iPadBits = 0;
	BYTE iPad = 0;
};

// 属性的参数
struct CPropertyField
{
	int bits, base, min, max;
	const CPropertyField & Normalize();
};

// 指定版本的属性元数据
class CPropertyMetaDataItem
{
	std::vector<CPropertyField> fields_;
	DWORD def_ = 0;
	DWORD verMin_ = 0;
	int bitsSum_ = 0;
	friend class CPropertyMetaData;
public:
	CPropertyMetaDataItem() {}
	CPropertyMetaDataItem(DWORD verMin, const std::vector<CPropertyField> & fields, DWORD def);
	int Bits() const { return bitsSum_; }
	std::vector<int> Parse(DWORD value) const;
	void Normalise(std::vector<int>& params) const;
	std::vector<std::tuple<int, int, int>> GetParams(DWORD value) const;
	std::pair<BOOL, DWORD> GetValue(const std::vector<int> & params) const;
	DWORD DefaultValue() const { return def_; }
	bool matchVersion(DWORD version) const { return verMin_ <= version; }
};

// 属性元数据
class CPropertyMetaData
{
	std::list<CPropertyMetaDataItem> data_;
public:
	CPropertyMetaData() {}
	void addData(const CPropertyMetaDataItem& item);
	const CPropertyMetaDataItem & findData(DWORD version) const;
};

// 跨平台格式化函数
QString CSFormat(const char* format, ...);
