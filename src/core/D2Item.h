#pragma once

#include "MetaData.h"
#include "MayExist.h"
#include "BinDataStream.h"
#include "D2Version.h"
#include "DataManagerFwd.h"

#include <QString>
#include <QByteArray>
#include <vector>

const int MAX_SOCKETS = 7;	// 物品最大孔数，多于6个可能会导致游戏崩溃

// Ear
struct CEar
{
	BYTE	iEarClass = 0;		// 3 bits
	BYTE	iEarLevel = 1;		// 7 bits
	QString	sEarName;			// 变长字符串
	explicit CEar(const QString& name = QString()) : sEarName(name) {};
	void ReadData(CInBitsStream& bs, DWORD version);
	void WriteData(COutBitsStream& bs, DWORD version) const;
};

// Item Long Name
struct CLongName
{
	BYTE				iName1;		// 8 bits, First Name
	BYTE				iName2;		// 8 bits, Second Name
	BOOL				bPref1;		// 1 bit, Prefix 1 flag
	MayExist<WORD>		wPref1;		// 11 bits, Prefix 1, if bPref1 == TRUE
	BOOL				bSuff1;		// 1 bit, Suffix 1 flag
	MayExist<WORD>		wSuff1;		// 11 bits, Suffix 1, if bSuff1 == TRUE
	BOOL				bPref2;		// 1 bit, Prefix 2 flag
	MayExist<WORD>		wPref2;		// 11 bits, Prefix 2, if bPref2 == TRUE
	BOOL				bSuff2;		// 1 bit, Suffix 2 flag
	MayExist<WORD>		wSuff2;		// 11 bits, Suffix 2, if bSuff2 == TRUE
	BOOL				bPref3;		// 1 bit, Prefix 3 flag
	MayExist<WORD>		wPref3;		// 11 bits, Prefix 3, if bPref3 == TRUE
	BOOL				bSuff3;		// 1 bit, Suffix 3 flag
	MayExist<WORD>		wSuff3;		// 11 bits, Suffix 3, if bSuff3 == TRUE
	void ReadData(CInBitsStream& bs);
	void WriteData(COutBitsStream& bs) const;
};

// Gold Quantity
struct CGoldQuantity
{
	BOOL	bNotGold = FALSE;	// 1 bit
	WORD	wQuantity = 0;		// 12 bits, 黄金数量
	void ReadData(CInBitsStream& bs);
	void WriteData(COutBitsStream& bs) const;
};

struct CPropertyList
{
	std::vector<std::pair<WORD, DWORD>> mProperty;	// 属性列表，每项（9 bits ID + VALUE)
	WORD	iEndFlag;	// 9 bits, 0x1FF, 结束标志
	int ExtSockets() const;
	BOOL IsIndestructible() const;
	void ReadData(CInBitsStream& bs, DWORD version);
	void WriteData(COutBitsStream& bs, DWORD version) const;
private:
	DWORD	dwVersion;
};

// Extended Item Info
struct CExtItemInfo
{
	BYTE					nGems = 0;
	DWORD					dwGUID = 0;
	BYTE					iDropLevel = 99;
	BYTE					iQuality = 2;
	BOOL					bVarGfx = FALSE;
	MayExist<BYTE>			iVarGfx;
	BOOL					bClass = FALSE;
	MayExist<WORD>			wClass;
	MayExist<BYTE>			loQual;
	MayExist<BYTE>			hiQual;
	MayExist<WORD>			wPrefix;
	MayExist<WORD>			wSuffix;
	MayExist<WORD>			wSetID;
	MayExist<CLongName>	pRareName;
	MayExist<WORD>			wUniID;
	MayExist<CLongName>	pCraftName;
	MayExist<WORD>			wRune;
	MayExist<BYTE, 16>		sPersonName;
	MayExist<WORD>			wMonsterID;
	MayExist<WORD>			wCharm;
	MayExist<BYTE>			iSpellID;
	explicit CExtItemInfo(const CItemMetaData* meta = nullptr);
	BOOL IsSet() const { return iQuality == 5; }
	int RuneWordId() const { ASSERT(wRune.exist()); return (wRune & 0xFFF); }
	int Gems() const { return nGems; }
	void ReadData(CInBitsStream& bs, DWORD version, BOOL bIsCharm, BOOL bRuneWord, BOOL bPersonalized, BOOL bHasMonsterID, BOOL bHasSpellId);
	void WriteData(COutBitsStream& bs, DWORD version, BOOL bIsCharm, BOOL bRuneWord, BOOL bPersonalized, BOOL bHasMonsterID, BOOL bHasSpellId) const;
};

// Type Specific info
struct CTypeSpecificInfo
{
	MayExist<WORD>			iDefence;
	MayExist<WORD>			iMaxDurability;
	MayExist<WORD>			iCurDur;
	MayExist<BYTE>			iSocket;
	MayExist<WORD>			iQuantity;
	MayExist<BOOL, 5>		aHasSetPropList;
	CPropertyList			stPropertyList;
	MayExist<CPropertyList> apSetProperty[5];
	MayExist<CPropertyList>	stRuneWordPropertyList;
	explicit CTypeSpecificInfo(const CItemMetaData* meta = nullptr);
	std::pair<int, int> Sockets() const;
	int TotalSockets() const { auto s = Sockets(); return s.first + s.second; }
	int GetDefence() const { ASSERT(iDefence.exist()); return iDefence - 10; }
	void SetDefence(int def) { iDefence.ensure() = def + 10; }
	BOOL IsIndestructible() const;
	void ReadData(CInBitsStream& bs, DWORD version, BOOL bHasDef, BOOL bHasDur, BOOL bSocketed, BOOL bIsStacked, BOOL bIsSet, BOOL bRuneWord);
	void WriteData(COutBitsStream& bs, DWORD version, BOOL bHasDef, BOOL bHasDur, BOOL bSocketed, BOOL bIsStacked, BOOL bIsSet, BOOL bRuneWord) const;
};

// ItemInfo
struct CItemInfo
{
	union {
		BYTE						sTypeName[4];
		DWORD						dwTypeID;
	};
	MayExist<CExtItemInfo>			pExtItemInfo;
	MayExist<CGoldQuantity>			pGold;
	BOOL							bHasRand = FALSE;
	MayExist<DWORD, 4>				pTimeStampFlag;
	MayExist<CTypeSpecificInfo>		pTpSpInfo;
	BYTE							iPad;
	explicit CItemInfo(const CItemMetaData* meta = nullptr);
	const CItemMetaData* ReadData(CInBitsStream& bs, DWORD version, BOOL bSimple, BOOL bRuneWord, BOOL bPersonalized, BOOL bSocketed);
	void WriteData(COutBitsStream& bs, const CItemMetaData& itemData, DWORD version, BOOL bSimple, BOOL bRuneWord, BOOL bPersonalized, BOOL bSocketed) const;
	BOOL IsNameValid() const;
	BOOL IsSet() const { return pExtItemInfo.exist() && pExtItemInfo->IsSet(); }
	BOOL IsGold() const { return std::memcmp(sTypeName, "gld ", sizeof sTypeName) == 0; }
	BOOL IsBox() const { return std::memcmp(sTypeName, "box ", sizeof sTypeName) == 0; }
	int RuneWordId() const { ASSERT(pExtItemInfo.exist()); return pExtItemInfo->RuneWordId(); }
	int Gems() const { return (pExtItemInfo.exist() ? pExtItemInfo->Gems() : 0); }
	int Sockets() const { return (pTpSpInfo.exist() ? pTpSpInfo->TotalSockets() : 0); }
};

struct CD2Item
{
	DWORD	dwVersion = 0;
	WORD	wMajic = 0x4D4A;
	BOOL	bQuest = FALSE;
	BYTE	iUNKNOWN_01 = 0;
	BOOL	bIdentified = TRUE;
	BYTE	iUNKNOWN_02 = 0;
	BOOL	bDisabled = FALSE;
	BYTE	iUNKNOWN_10 = 0;
	BOOL	bSocketed = FALSE;
	BOOL	bUNKNOWN_03 = 0;
	BOOL	bNew = 0;
	BOOL	bBadEquipped = FALSE;
	BOOL	bUNKNOWN_04 = FALSE;
	BOOL	bEar = FALSE;
	BOOL	bNewbie = FALSE;
	BYTE	iUNKNOWN_05 = 0;
	BOOL	bSimple = TRUE;
	BOOL	bEthereal = FALSE;
	BOOL	bUNKNOWN_06 = TRUE;
	BOOL	bPersonalized = FALSE;
	BOOL	bUNKNOWN_07 = FALSE;
	BOOL	bRuneWord = FALSE;
	BYTE	iUNKNOWN_08 = 0;
	WORD	wVersion = 101;
	BYTE	iUNKNOWN_09 = 0xA0;
	BYTE	iLocation = 0;
	BYTE	iPosition = 0;
	BYTE	iColumn = 0;
	BYTE	iRow = 0;
	BYTE	iStoredIn = 1;
	MayExist<CEar>			pEar;
	MayExist<CItemInfo>		pItemInfo;
	std::vector<CD2Item>	aGemItems;

	explicit CD2Item(DWORD type = 0);
	const CItemMetaData& MetaData() const { return *pItemData; }
	BYTE Quality() const { return !bEar && !bSimple ? pItemInfo->pExtItemInfo->iQuality : (pItemData->IsUnique ? 7 : 2); }
	BOOL IsSet() const { return pItemInfo.exist() && pItemInfo->IsSet(); }
	BOOL IsRuneWord() const { return bRuneWord; }
	BOOL IsEditable() const { return TRUE; }
	int RuneWordId() const { ASSERT(IsRuneWord() && pItemInfo.exist()); return pItemInfo->RuneWordId(); }
	int Gems() const { return (pItemInfo.exist() ? pItemInfo->Gems() : 0); }
	int Sockets() const { return (bSocketed && pItemInfo.exist() ? pItemInfo->Sockets() : 0); }
	QString ItemName() const;
	int GemIndexMax() const;
	BOOL IsBox() const { return pItemInfo.exist() && pItemInfo->IsBox(); }
	BOOL HasPropertyList() const { return pItemInfo.exist() && pItemInfo->pTpSpInfo.exist(); }
	void ReadData(CInBitsStream& bs, DWORD version);
	void WriteData(COutBitsStream& bs, DWORD version) const;
	BOOL ReadFile(QFile& file);
	void WriteFile(QFile& file) const;
private:
	std::vector<BYTE>		vUnknownItem;
	const CItemMetaData*	pItemData = nullptr;
};

struct CItemList
{
	WORD		wMajic;
	std::vector<CD2Item> vItems;
	WORD		wEndMajic;
	void SwapItems(CItemList& list) { vItems.swap(list.vItems); }
	void Reset() { vItems.clear(); }
	void ReadData(CInBitsStream& bs, DWORD version);
	void WriteData(COutBitsStream& bs, DWORD version) const;
};
