#include "D2S_Struct.h"
#include "DataManager.h"
#include "DataManagerFwd.h"

#include <QFile>
#include <QDebug>
#include <algorithm>
#include <iterator>

using namespace std;

static DWORD ComputCRC(const BYTE* source, DWORD len, DWORD init) {
	ASSERT(source && len);
	for (UINT i = 0, add; i < len; ++i, init = (init << 1) + add)
		add = (init & 0x80000000 ? 1 : 0) + source[i];
	return init;
}

static BOOL ValidateCrc(std::vector<BYTE>& data, DWORD dwCrc, DWORD dwOff) {
	ASSERT(dwOff + 4 <= data.size());
	*reinterpret_cast<DWORD*>(&data[dwOff]) = 0;
	DWORD computed = ComputCRC(&data[0], data.size(), 0);
	BOOL ok = (dwCrc == computed);
	qDebug().nospace() << "CRC: stored=0x" << Qt::hex << dwCrc << " computed=0x" << computed << " ok=" << ok;
	return ok;
}

void CQuestInfoData::ReadData(CInBitsStream& bs) {
	bs >> wIntroduced1 >> wActI >> wTraval1
		>> wIntroduced2 >> wActII >> wTraval2
		>> wIntroduced3 >> wActIII >> wTraval3
		>> wIntroduced4 >> wActIV >> wTraval4
		>> unknown1 >> wIntroduced5 >> unknown2
		>> wActV >> bResetStats >> unknown3
		>> unknown4;
}

void CQuestInfoData::WriteData(COutBitsStream& bs) const {
	bs << wIntroduced1 << wActI << wTraval1
		<< wIntroduced2 << wActII << wTraval2
		<< wIntroduced3 << wActIII << wTraval3
		<< wIntroduced4 << wActIV << wTraval4
		<< unknown1 << wIntroduced5 << unknown2
		<< wActV << bResetStats << unknown3
		<< unknown4;
}

void CQuestInfo::ReadData(CInBitsStream& bs, DWORD version) {
	(void)version;
	bs >> dwMajic >> dwActs >> wSize;
	if (dwMajic != 0x216F6F57 || wSize != 0x12A)
		throw D2Error(13);
	for (auto& p : QIData) p.ReadData(bs);
}

void CQuestInfo::WriteData(COutBitsStream& bs, DWORD version) const {
	(void)version;
	bs << DWORD(0x216F6F57) << dwActs << WORD(0x12A);
	for (const auto& p : QIData) p.WriteData(bs);
}

void CWaypointData::ReadData(CInBitsStream& bs) {
	bs >> unkown >> Waypoints >> unkown2;
}

void CWaypointData::WriteData(COutBitsStream& bs) const {
	bs << unkown << Waypoints << unkown2;
}

void CWaypoints::ReadData(CInBitsStream& bs, DWORD version) {
	bs >> wMajic >> unkown >> wSize;
	if (wMajic != 0x5357)
		throw D2Error(14);
	if (!IsD2R(version) && wSize != 0x50)
		throw D2Error(14);
	if (IsD2R(version)) {
		// D2R: unkown和wSize均为DWORD(各4字节)，旧版为WORD(各2字节)，共多4字节
		WORD d2rExtra[2];
		bs >> d2rExtra;
	}
	for (auto& p : wp) p.ReadData(bs);
}

void CWaypoints::WriteData(COutBitsStream& bs, DWORD version) const {
	bs << WORD(0x5357) << unkown << wSize;
	if (IsD2R(version)) {
		bs << WORD(0) << WORD(0);
	}
	if (IsD2R(version))
		bs << WORD(0) << WORD(0);
	for (const auto& p : wp) p.WriteData(bs);
}

static const DWORD PLAYER_STATS_BITS_COUNT[CPlayerStats::ARRAY_SIZE] = {
	10,10,10,10,10,8,
	21,21,21,21,21,21,
	7,32,25,25
};

void CPlayerStats::ReadData(CInBitsStream& bs) {
	bs >> wMajic;
	if (wMajic != 0x6667) throw D2Error(15);
	ZeroMemory(m_adwValue, sizeof(m_adwValue));
	for (bs >> bits(iEnd, 9); bs.Good() && iEnd < size(m_adwValue); bs >> bits(iEnd, 9))
		bs >> bits(m_adwValue[iEnd], PLAYER_STATS_BITS_COUNT[iEnd]);
	bs.AlignByte();
}

void CPlayerStats::WriteData(COutBitsStream& bs) const {
	bs << WORD(0x6667);
	for (UINT i = 0; bs.Good() && i < size(m_adwValue); ++i)
		if (m_adwValue[i])
			bs << bits(WORD(i), 9) << bits(m_adwValue[i], PLAYER_STATS_BITS_COUNT[i]);
	bs << bits<WORD>(0x1FF, 9);
	bs.AlignByte();
}

void CCharSkills::ReadData(CInBitsStream& bs) {
	bs >> wMagic;
	if (wMagic != 0x6669) throw D2Error(16);
	bs >> bSkillLevel;
}

void CCharSkills::WriteData(COutBitsStream& bs) const {
	bs << WORD(0x6669) << bSkillLevel;
}

void CCorpseData::ReadData(CInBitsStream& bs, DWORD version) {
	bs >> unknown;
	stItems.ReadData(bs, version);
}

void CCorpseData::WriteData(COutBitsStream& bs, DWORD version) const {
	bs << unknown;
	stItems.WriteData(bs, version);
}

void CCorpse::ReadData(CInBitsStream& bs, DWORD version) {
	bs >> wMagic >> wCount;
	if (wMagic != 0x4D4A || wCount > 1) throw D2Error(19);
	if (wCount) pCorpseData.ensure().ReadData(bs, version);
}

void CCorpse::WriteData(COutBitsStream& bs, DWORD version) const {
	bs << WORD(0x4D4A);
	if (pCorpseData.exist()) {
		bs << WORD(1);
		pCorpseData->WriteData(bs, version);
	} else {
		bs << WORD(0);
	}
}

void CMercenary::ReadData(CInBitsStream& bs, BOOL hasMercenary, DWORD version) {
	bs >> wMagic;
	if (wMagic != 0x666A) throw D2Error(20);
	if (hasMercenary) stItems.ensure().ReadData(bs, version);
}

void CMercenary::WriteData(COutBitsStream& bs, BOOL hasMercenary, DWORD version) const {
	bs << WORD(0x666A);
	if (hasMercenary) stItems->WriteData(bs, version);
}

void CGolem::ReadData(CInBitsStream& bs, DWORD version) {
	bs >> wMagic >> bHasGolem;
	if (wMagic != 0x666B) throw D2Error(21);
	if (bHasGolem) pItem.ensure().ReadData(bs, version);
}

void CGolem::WriteData(COutBitsStream& bs, DWORD version) const {
	bs << WORD(0x666B) << bHasGolem;
	if (bHasGolem) pItem->WriteData(bs, version);
}

// CD2S_Struct

void CD2S_Struct::ReadFile(const QString& path) {
	QFile file(path);
	if (!file.open(QFile::ReadOnly)) throw D2Error(11);
	CInBitsStream bs;
	bs.ReadFile(file);
	file.close();
	ReadData(bs);
}

void CD2S_Struct::WriteFile(const QString& path) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly)) return;
	COutBitsStream bs;
	if (!WriteData(bs)) return;
	bs.WriteFile(file);
	file.close();
}

void CD2S_Struct::Reset() {
	QuestInfo.Reset();
	Waypoints.Reset();
	PlayerStats.Reset();
	Skills.Reset();
	ItemList.Reset();
	stCorpse.Reset();
	stMercenary.Reset();
	stGolem.Reset();
	wMagic666C = 0x666C;
	dwUnknown666C = 0;
}

void CD2S_Struct::ReadData(CInBitsStream& bs) {
	Reset();
	bs >> dwMajic;
	if (dwMajic != 0xAA55AA55) throw D2Error(11);
	bs >> dwVersion >> dwSize;
	if (bs.DataSize() != dwSize) throw D2Error(12);
	const DWORD offCrc = bs.BytePos();
	bs >> dwCRC;
	if (!ValidateCrc(bs.Data(), dwCRC, offCrc))
		throw D2Error(11);
	const BOOL isV31 = IsPtr31AndAbove(dwVersion);
	bs >> dwWeaponSet;
	if (!isV31) bs >> Name;
	bs >> charType >> charTitle >> unkown1 >> charClass >> unkown2
		>> charLevel >> unkown3 >> dwTime >> unkown4
		>> dwSkillKey >> dwLeftSkill1 >> dwRightSkill1
		>> dwLeftSkill2 >> dwRightSkill2
		>> outfit >> colors >> Town >> dwMapSeed
		>> unkown5 >> bMercDead >> unkown6
		>> dwMercControl >> wMercName >> wMercType >> dwMercExp >> unkown7;
	if (isV31) {
		BYTE NamePTR31[0x94];
		bs >> NamePTR31;
		CopyMemory(NamePTR, NamePTR31, sizeof(NamePTR31));
	} else if (IsPtr24AndAbove(dwVersion)) {
		// D2R PTR2.4/2.5 (0x62-0x68): NamePTR 为 0x40 字节
		BYTE NamePTR24[0x40];
		bs >> NamePTR24;
		CopyMemory(NamePTR, NamePTR24, sizeof(NamePTR24));
		bs >> unkown8;
	} else {
		bs >> NamePTR >> unkown8;
	}
	if (isV31) bs >> unkown8;
	QuestInfo.ReadData(bs, dwVersion);
	Waypoints.ReadData(bs, dwVersion);
	fprintf(stderr, "POS after Waypoints=%u\n", (unsigned)bs.BytePos()); fflush(stderr);
	bs >> NPC;
	fprintf(stderr, "POS after NPC=%u\n", (unsigned)bs.BytePos()); fflush(stderr);
	PlayerStats.ReadData(bs);
	fprintf(stderr, "POS before PS=%u\n", (unsigned)bs.BytePos()); fflush(stderr);
	Skills.ReadData(bs);
	ItemList.ReadData(bs, dwVersion);
	stCorpse.ReadData(bs, dwVersion);
	if (isExpansion() || isV31) {
		stMercenary.ReadData(bs, HasMercenary(), dwVersion);
		stGolem.ReadData(bs, dwVersion);
	}
	if (isV31) {
		bs.AlignByte();
		const DWORD remaining = bs.DataSize() - bs.BytePos();
		if (remaining >= 6) {
			WORD peekMagic = 0;
			bs >> peekMagic;
			if (peekMagic == 0x666C) {
				wMagic666C = peekMagic;
				bs >> dwUnknown666C;
			} else {
				bs.SeekBack(2);
			}
		}
	} else {
		bs.AlignByte();
	}
	if (!bs.Good() || bs.DataSize() < bs.BytePos())
		throw D2Error(11);
	if (!isV31 && bs.DataSize() != bs.BytePos())
		throw D2Error(11);
}

BOOL CD2S_Struct::WriteData(COutBitsStream& bs) const {
	bs << DWORD(0xAA55AA55) << dwVersion;
	const BOOL isV31 = IsPtr31AndAbove(dwVersion);
	const DWORD offSize = bs.BytePos();
	bs << DWORD(0);
	const DWORD offCrc = bs.BytePos();
	bs << DWORD(0) << dwWeaponSet;
	if (!isV31) bs << Name;
	bs << charType << charTitle << unkown1 << charClass << unkown2
		<< charLevel << unkown3 << dwTime << unkown4
		<< dwSkillKey << dwLeftSkill1 << dwRightSkill1
		<< dwLeftSkill2 << dwRightSkill2
		<< outfit << colors << Town << dwMapSeed
		<< unkown5 << bMercDead << unkown6
		<< dwMercControl << wMercName << wMercType << dwMercExp << unkown7;
	if (isV31) {
		bs << NamePTR;
	} else if (IsPtr24AndAbove(dwVersion)) {
		// D2R PTR2.4/2.5: NamePTR 只写 0x40 字节
		bs.WriteBytes(NamePTR, 0x40);
		bs << unkown8;
	} else {
		bs << NamePTR << unkown8;
	}
	if (isV31) bs << unkown8;
	QuestInfo.WriteData(bs, dwVersion);
	Waypoints.WriteData(bs, dwVersion);
	bs << NPC;
	if (isV31) {
		bs << WORD(0);
	}
	PlayerStats.WriteData(bs);
	Skills.WriteData(bs);
	ItemList.WriteData(bs, dwVersion);
	stCorpse.WriteData(bs, dwVersion);
	if (isExpansion() || isV31) {
		stMercenary.WriteData(bs, HasMercenary(), dwVersion);
		stGolem.WriteData(bs, dwVersion);
	}
	if (isV31 && wMagic666C != 0) {
		bs.AlignByte();
		bs << wMagic666C << dwUnknown666C;
	} else {
		bs.AlignByte();
	}
	bs << offset_value(offSize, bs.BytePos());
	auto& data = bs.Data();
	const DWORD dwCrc = ComputCRC(&data[0], data.size(), 0);
	bs << offset_value(offCrc, dwCrc);
	return bs.Good();
}

QString CD2S_Struct::name() const {
	if (IsPtr31AndAbove(dwVersion)) {
		const BYTE* p = NamePTR;
		const BYTE* end = NamePTR + sizeof(NamePTR);
		while (p < end && *p == 0) ++p;
		return DecodeCharName(p);
	}
	return DecodeCharName(IsPtr24AndAbove(dwVersion) ? NamePTR : Name);
}

void CD2S_Struct::name(const QString& name) {
	QByteArray utf8 = EncodeCharName(name);
	const BOOL isV31 = IsPtr31AndAbove(dwVersion);
	const BOOL isPtr24 = IsPtr24AndAbove(dwVersion);
	if (isV31) {
		ZeroMemory(NamePTR, sizeof(NamePTR));
		CopyMemory(NamePTR, utf8.constData(), std::min((int)utf8.length(), (int)sizeof(NamePTR)));
	} else {
		BYTE* dest = isPtr24 ? NamePTR : Name;
		int destLen = isPtr24 ? sizeof(NamePTR) : sizeof(Name);
		ZeroMemory(dest, destLen);
		CopyMemory(dest, utf8.constData(), std::min((int)utf8.length(), (int)destLen));
	}
}
