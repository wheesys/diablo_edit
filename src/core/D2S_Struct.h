#pragma once

#include "D2Item.h"
#include "D2Version.h"

// 任务完成信息
struct CQuestInfoData
{
	WORD	wIntroduced1;
	WORD	wActI[6];
	WORD	wTraval1;
	WORD	wIntroduced2;
	WORD	wActII[6];
	WORD	wTraval2;
	WORD	wIntroduced3;
	WORD	wActIII[6];
	WORD	wTraval3;
	WORD	wIntroduced4;
	WORD	wActIV[3];
	WORD	wTraval4;
	WORD	unknown1[3];
	WORD	wIntroduced5;
	WORD	unknown2[2];
	WORD	wActV[6];
	BYTE	bResetStats;
	BYTE	unknown3;
	WORD	unknown4[6];
	void ReadData(CInBitsStream& bs);
	void WriteData(COutBitsStream& bs) const;
};

struct CQuestInfo
{
	DWORD	dwMajic;
	DWORD	dwActs;
	WORD	wSize;
	CQuestInfoData	QIData[3];
	void Reset() {}
	void ReadData(CInBitsStream& bs);
	void WriteData(COutBitsStream& bs) const;
};

struct CWaypointData
{
	WORD	unkown;
	BYTE	Waypoints[5];
	BYTE	unkown2[17];
	void ReadData(CInBitsStream& bs);
	void WriteData(COutBitsStream& bs) const;
};

struct CWaypoints
{
	WORD	wMajic;
	DWORD	unkown;
	WORD	wSize;
	CWaypointData	wp[3];
	void Reset() {}
	void ReadData(CInBitsStream& bs);
	void WriteData(COutBitsStream& bs) const;
};

struct CPlayerStats
{
	static const int ARRAY_SIZE = 0x10;
	WORD	wMajic;
	DWORD	m_adwValue[ARRAY_SIZE];
	WORD iEnd;
	void Reset() {}
	void ReadData(CInBitsStream& bs);
	void WriteData(COutBitsStream& bs) const;
};

struct CCharSkills
{
	WORD	wMagic;
	BYTE	bSkillLevel[30];
	void Reset() {}
	void ReadData(CInBitsStream& bs);
	void WriteData(COutBitsStream& bs) const;
};

struct CCorpseData
{
	BYTE		unknown[12];
	CItemList	stItems;
	void ReadData(CInBitsStream& bs, DWORD version);
	void WriteData(COutBitsStream& bs, DWORD version) const;
};

struct CCorpse
{
	WORD		wMagic;
	WORD		wCount;
	MayExist<CCorpseData> pCorpseData;
	BOOL HasCorpse() const { return wCount > 0 && pCorpseData.exist(); }
	void Reset() { pCorpseData.reset(); }
	void ReadData(CInBitsStream& bs, DWORD version);
	void WriteData(COutBitsStream& bs, DWORD version) const;
};

struct CMercenary
{
	WORD				wMagic;
	MayExist<CItemList>	stItems;
	void Reset() { stItems.reset(); }
	void ReadData(CInBitsStream& bs, BOOL hasMercenary, DWORD version);
	void WriteData(COutBitsStream& bs, BOOL hasMercenary, DWORD version) const;
};

struct CGolem
{
	WORD	wMagic;
	BYTE	bHasGolem;
	MayExist<CD2Item>	pItem;
	void Reset() { bHasGolem = FALSE; pItem.reset(); }
	void ReadData(CInBitsStream& bs, DWORD version);
	void WriteData(COutBitsStream& bs, DWORD version) const;
};

struct CD2S_Struct
{
	void ReadData(CInBitsStream& bs);
	void ReadFile(const QString& path);
	void WriteFile(const QString& path) const;
	QString name() const;
	void name(const QString& name);
	BOOL HasCorpse() const { return stCorpse.HasCorpse(); }
	BOOL HasMercenary() const { return IsD2R(dwVersion) ? (dwMercControl != 0) : (wMercName > 0); }
	BOOL isLadder() const { return (charType & 0x40) != 0; }
	BOOL isExpansion() const { return IsPtr31AndAbove(dwVersion) ? TRUE : (charType & 0x20) != 0; }
	BOOL isDiedBefore() const { return (charType & 0x8) != 0; }
	BOOL isHardcore() const { return (charType & 0x4) != 0; }
	void Reset();
private:
	BOOL WriteData(COutBitsStream& bs) const;
public:
	DWORD	dwMajic;
	DWORD	dwVersion;
	DWORD	dwSize;
	DWORD	dwCRC;
	DWORD	dwWeaponSet;
	BYTE	Name[16];
	BYTE	charType;
	BYTE	charTitle;
	WORD	unkown1;
	BYTE	charClass;
	WORD	unkown2;
	BYTE	charLevel;
	DWORD	unkown3;
	DWORD	dwTime;
	DWORD	unkown4;
	DWORD	dwSkillKey[16];
	DWORD	dwLeftSkill1;
	DWORD	dwRightSkill1;
	DWORD	dwLeftSkill2;
	DWORD	dwRightSkill2;
	BYTE	outfit[16];
	BYTE	colors[16];
	BYTE	Town[3];
	DWORD	dwMapSeed;
	WORD	unkown5;
	BYTE	bMercDead;
	BYTE	unkown6;
	DWORD	dwMercControl;
	WORD	wMercName;
	WORD	wMercType;
	DWORD	dwMercExp;
	BYTE	unkown7[0x4C];
	BYTE	NamePTR[0x94];
	DWORD	unkown8;

	CQuestInfo		QuestInfo;
	CWaypoints		Waypoints;
	BYTE			NPC[0x34];
	CPlayerStats	PlayerStats;
	CCharSkills		Skills;
	CItemList		ItemList;
	CCorpse			stCorpse;
	CMercenary		stMercenary;
	CGolem			stGolem;
};
