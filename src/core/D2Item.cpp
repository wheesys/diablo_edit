#include "D2Item.h"
#include "DataManager.h"

#include <fstream>
#include <iterator>
#include <cstring>
#include <deque>
#include <random>
#include <ctime>
#include <map>
#include <algorithm>

using namespace std;

class CRandom {
	mt19937 r_;
	uniform_int_distribution<DWORD>	dw_;
public:
	CRandom() {
		r_.seed(random_device()());
	}
	DWORD dwValue() { return dw_(r_); }
};

static CRandom g_rand;

BOOL CheckCharName(const QString& name) {
	int len = name.length();
	return len > 1 && len < 16;
}

QString DecodeCharName(const BYTE* name) {
	if (!name) return QString();
	return QString::fromUtf8(reinterpret_cast<const char*>(name));
}

QByteArray EncodeCharName(const QString& name) {
	return name.toUtf8();
}

// Huffman encoding table
const BYTE HUFFMAN[] = {
   1,	1,	1,	1,	1,	'w',0,	0,	'u',0,
   0,	1,	1,	'8',0,	0,	1,	'y',0,	0,
   1,	'5',0,	0,	1,	'j',0,	0,	1,	0,
   0,	'h',0,	0,	1,	's',0,	0,	1,	1,
   '2',0,	0,	'n',0,	0,	'x',0,	0,	1,
   1,	1,	'c',0,	0,	1,	'k',0,	0,	'f',
   0,	0,	'b',0,	0,	1,	1,	't',0,	0,
   'm',0,	0,	1,	'9',0,	0,	'7',0,	0,
   1,	' ',0,	0,	1,	1,	1,	1,	'e',0,
   0,	'd',0,	0,	'p',0,	0,	1,	'g',0,
   0,	1,	1,	1,	'z',0,	0,	'q',0,	0,
   '3',0,	0,	1,	'v',0,	0,	'6',0,	0,
   1,	1,	'r',0,	0,	'l',0,	0,	1,	'a',
   0,	0,	1,	1,	'1',0,	0,	1,	'4',0,
   0,	'0',0,	0,	1,	'i',0,	0,	'o',0,
};

class HuffmanTree {
	struct HuffmanNode {
		const HuffmanNode* left = nullptr;
		const HuffmanNode* right = nullptr;
		BYTE data = 0;
		~HuffmanNode() {
			delete left;
			delete right;
		}
	};
public:
	HuffmanTree() {
		int i = 0;
		root_ = constructHuffmanTree(i);
	}
	~HuffmanTree() {
		delete root_;
	}
	BYTE readData(CInBitsStream& bs) const {
		auto node = root_;
		for (BOOL b; bs.Good(); node = b ? node->right : node->left) {
			ASSERT(node);
			if (node->data) return node->data;
			bs >> b;
		}
		ASSERT(false);
		return 0;
	}
	void writeData(COutBitsStream& bs, BYTE ch) const {
		auto wh = codes_.find(ch);
		ASSERT(wh != codes_.end());
		bs << bits(wh->second.first, wh->second.second);
	}
private:
	HuffmanTree(const HuffmanTree&);
	HuffmanTree& operator =(const HuffmanTree&);
	const HuffmanNode* constructHuffmanTree(int& index, int bits = 0, DWORD code = 0) {
		if (index >= (int)sizeof(HUFFMAN)) return nullptr;
		const BYTE c = HUFFMAN[index++];
		if (!c) return nullptr;
		HuffmanNode* node = new HuffmanNode;
		if (c > 1) {
			node->data = c;
			codes_[c] = make_pair(code, bits);
		}
		node->left = constructHuffmanTree(index, bits + 1, code);
		node->right = constructHuffmanTree(index, bits + 1, code + (1 << bits));
		return node;
	}
	const HuffmanNode* root_;
	map<BYTE, pair<DWORD, int>> codes_;
};

static const HuffmanTree g_huffmanTree;

// struct CEar

void CEar::ReadData(CInBitsStream& bs, DWORD version) {
	bs >> bits(iEarClass, 3) >> bits(iEarLevel, 7);
	int b = IsPtr24AndAbove(version) ? 8 : 7;
	BYTE buf[0x40] = { 0 };
	for (auto& c : buf) {
		bs >> bits(c, b);
		if (!bs.Good() || c == 0)
			break;
	}
	sEarName = DecodeCharName(buf);
}

void CEar::WriteData(COutBitsStream& bs, DWORD version) const {
	bs << bits(iEarClass, 3) << bits(iEarLevel, 7);
	const QByteArray buf = EncodeCharName(sEarName);
	const BOOL isPtr24 = IsPtr24AndAbove(version);
	int b = isPtr24 ? 8 : 7;
	int len = isPtr24 ? 60 : 15;
	for (int i = 0; i < buf.length() && i < len; ++i) {
		BYTE c = static_cast<BYTE>(buf[i]);
		bs << bits(c, b);
		if (!bs.Good() || c == 0)
			break;
	}
	bs << bits(BYTE(0), b);
}

// struct CLongName

void CLongName::ReadData(CInBitsStream& bs) {
	bs >> bits(iName1, 8) >> bits(iName2, 8) >> bPref1;
	if (bPref1) bs >> bits(wPref1, 11);
	bs >> bSuff1;
	if (bSuff1) bs >> bits(wSuff1, 11);
	bs >> bPref2;
	if (bPref2) bs >> bits(wPref2, 11);
	bs >> bSuff2;
	if (bSuff2) bs >> bits(wSuff2, 11);
	bs >> bPref3;
	if (bPref3) bs >> bits(wPref3, 11);
	bs >> bSuff3;
	if (bSuff3) bs >> bits(wSuff3, 11);
}

void CLongName::WriteData(COutBitsStream& bs) const {
	bs << bits(iName1, 8) << bits(iName2, 8) << bPref1;
	if (bPref1) bs << bits(wPref1, 11);
	bs << bSuff1;
	if (bSuff1) bs << bits(wSuff1, 11);
	bs << bPref2;
	if (bPref2) bs << bits(wPref2, 11);
	bs << bSuff2;
	if (bSuff2) bs << bits(wSuff2, 11);
	bs << bPref3;
	if (bPref3) bs << bits(wPref3, 11);
	bs << bSuff3;
	if (bSuff3) bs << bits(wSuff3, 11);
}

// struct CGoldQuantity

void CGoldQuantity::ReadData(CInBitsStream& bs) {
	bs >> bNotGold >> bits(wQuantity, 12);
}

void CGoldQuantity::WriteData(COutBitsStream& bs) const {
	bs << bNotGold << bits(wQuantity, 12);
}

// struct CPropertyList

void CPropertyList::ReadData(CInBitsStream& bs, DWORD version) {
	dwVersion = version;
	for (bs >> bits(iEndFlag, 9); bs.Good() && iEndFlag < 0x1FF; bs >> bits(iEndFlag, 9)) {
		const int b = g_dataMgr->propertyMetaData(version, iEndFlag).Bits();
		if (b > 0)
			bs >> bits(mProperty.emplace_back(iEndFlag, 0).second, b);
	}
}

void CPropertyList::WriteData(COutBitsStream& bs, DWORD version) const {
	for (auto& p : mProperty) {
		const auto item = g_dataMgr->propertyMetaData(version, p.first);
		const int b = item.Bits();
		if (b > 0) {
			DWORD value = p.second;
			if (version != dwVersion) {
				const auto oldItem = g_dataMgr->propertyMetaData(dwVersion, p.first);
				if (&item != &oldItem) {
					auto v = oldItem.Parse(value);
					item.Normalise(v);
					value = item.GetValue(v).second;
				}
			}
			bs << bits(p.first, 9) << bits(value, b);
		}
	}
	bs << bits<WORD>(0x1FF, 9);
}

int CPropertyList::ExtSockets() const {
	int r = 0;
	for (auto& p : mProperty)
		if (p.first == 194)
			r += p.second;
	return r;
}

BOOL CPropertyList::IsIndestructible() const {
	for (auto& p : mProperty)
		if (p.first == 152 && p.second != 0)
			return TRUE;
	return FALSE;
}

template<class V, class T>
static pair<V&, const T&> pack(V& v, const T& t) {
	return pair<V&, const T&>(v, t);
}

// struct CExtItemInfo

CExtItemInfo::CExtItemInfo(const CItemMetaData* meta) {
	if (meta) {
		dwGUID = g_rand.dwValue();
		if (meta->HasMonsterID) wMonsterID.ensure();
		if (meta->IsCharm) wCharm.ensure();
		if (meta->SpellId > 0) iSpellID.ensure(meta->SpellId - 1);
	}
}

void CExtItemInfo::ReadData(CInBitsStream& bs, DWORD version, BOOL bIsCharm, BOOL bRuneWord, BOOL bPersonalized, BOOL bHasMonsterID, BOOL bHasSpellId) {
	bs >> bits(nGems, 3) >> bits(dwGUID, 32) >> bits(iDropLevel, 7) >> bits(iQuality, 4) >> bVarGfx;
	if (bVarGfx) bs >> bits(iVarGfx, 3);
	bs >> bClass;
	if (bClass) bs >> bits(wClass, 11);
	switch (iQuality) {
	case 1: bs >> bits(loQual, 3); break;
	case 2: if (bIsCharm) bs >> bits(wCharm, 12); break;
	case 3: bs >> bits(hiQual, 3); break;
	case 4: bs >> bits(wPrefix, 11) >> bits(wSuffix, 11); break;
	case 5: bs >> bits(wSetID, 12); break;
	case 6: pRareName.ensure().ReadData(bs); break;
	case 7: bs >> bits(wUniID, 12); break;
	case 8: pCraftName.ensure().ReadData(bs); break;
	default: throw D2Error(7);
	}
	if (bRuneWord) bs >> bits(wRune, 16);
	if (bPersonalized) {
		int b = IsPtr24AndAbove(version) ? 8 : 7;
		for (auto& c : sPersonName.ensure()) {
			bs >> bits(c, b);
			if (!bs.Good() || !c) break;
		}
	}
	if (bHasMonsterID) bs >> bits(wMonsterID, 10);
	else if (bHasSpellId) bs >> bits(iSpellID, 5);
}

void CExtItemInfo::WriteData(COutBitsStream& bs, DWORD version, BOOL bIsCharm, BOOL bRuneWord, BOOL bPersonalized, BOOL bHasMonsterID, BOOL bHasSpellId) const {
	bs << bits(nGems, 3) << bits(dwGUID, 32) << bits(iDropLevel, 7) << bits(iQuality, 4) << bVarGfx;
	if (bVarGfx) bs << bits(iVarGfx, 3);
	bs << bClass;
	if (bClass) bs << bits(wClass, 11);
	switch (iQuality) {
	case 1: bs << bits(loQual, 3); break;
	case 2: if (bIsCharm) bs << bits(wCharm, 12); break;
	case 3: bs << bits(hiQual, 3); break;
	case 4: bs << bits(wPrefix, 11) << bits(wSuffix, 11); break;
	case 5: bs << bits(wSetID, 12); break;
	case 6: pRareName->WriteData(bs); break;
	case 7: bs << bits(wUniID, 12); break;
	case 8: pCraftName->WriteData(bs); break;
	default: ASSERT(FALSE);
	}
	if (bRuneWord) bs << bits(wRune, 16);
	if (bPersonalized) {
		int b = IsPtr24AndAbove(version) ? 8 : 7;
		for (auto c : sPersonName) {
			bs << bits(c, b);
			if (!bs.Good() || !c) break;
		}
	}
	if (bHasMonsterID) bs << bits(wMonsterID, 10);
	else if (bHasSpellId) bs << bits(iSpellID, 5);
}

// struct CTypeSpecificInfo

CTypeSpecificInfo::CTypeSpecificInfo(const CItemMetaData* meta) {
	if (meta) {
		if (meta->HasDef) iDefence.ensure();
		if (meta->HasDur) { iMaxDurability.ensure(10); iCurDur.ensure(10); }
		if (meta->IsStacked) iQuantity.ensure();
	}
}

void CTypeSpecificInfo::ReadData(CInBitsStream& bs, DWORD version, BOOL bHasDef, BOOL bHasDur, BOOL bSocketed, BOOL bIsStacked, BOOL bIsSet, BOOL bRuneWord) {
	if (bHasDef) bs >> bits(iDefence, 11);
	if (bHasDur) {
		bs >> bits(iMaxDurability, 8);
		if (iMaxDurability) bs >> bits(iCurDur, 9);
	}
	if (bSocketed) { bs >> bits(iSocket, 4); if (iSocket < 1) iSocket = 1; }
		if (IsRotWAndAbove(version)) {
			// v105: 物品总是有 1-bit 数量标志位 (即使非堆叠物品也写 0)
			BOOL bHasQuantity;
			bs >> bHasQuantity;
			if (bHasQuantity) bs >> bits(iQuantity.ensure(), 9);
		} else if (bIsStacked) {
			bs >> bits(iQuantity, 9);
		}
	if (bIsSet) for (auto& b : aHasSetPropList.ensure()) if (bs.Good()) bs >> b;
	stPropertyList.ReadData(bs, version);
	if (bIsSet) for (size_t i = 0; bs.Good() && i < aHasSetPropList.size(); ++i) if (aHasSetPropList[i]) apSetProperty[i].ensure().ReadData(bs, version);
	if (bRuneWord) stRuneWordPropertyList.ensure().ReadData(bs, version);
}

void CTypeSpecificInfo::WriteData(COutBitsStream& bs, DWORD version, BOOL bHasDef, BOOL bHasDur, BOOL bSocketed, BOOL bIsStacked, BOOL bIsSet, BOOL bRuneWord) const {
		if (bHasDef) bs << bits(iDefence, 11);
		if (bHasDur) {
			bs << bits(iMaxDurability, 8);
			if (iMaxDurability) bs << bits(iCurDur, 9);
		}
		if (bSocketed) bs << bits(iSocket, 4);
		if (IsRotWAndAbove(version)) {
			// v105: 总是写 1-bit 数量标志位
			BOOL hasQty = bIsStacked && iQuantity.exist();
			bs << hasQty;
			if (hasQty) bs << bits(iQuantity, 9);
		} else if (bIsStacked) {
			bs << bits(iQuantity, 9);
		}
		if (bIsSet) for (auto b : aHasSetPropList) if (bs.Good()) bs << b;
		stPropertyList.WriteData(bs, version);
		if (bIsSet) for (size_t i = 0; bs.Good() && i < aHasSetPropList.size(); ++i) if (aHasSetPropList[i]) apSetProperty[i]->WriteData(bs, version);
		if (bRuneWord) stRuneWordPropertyList->WriteData(bs, version);
	}
pair<int, int> CTypeSpecificInfo::Sockets() const {
	const int b = min(MAX_SOCKETS, (iSocket.exist() ? (int)iSocket : 0));
	int e = stPropertyList.ExtSockets();
	for (auto& p : apSetProperty) if (p.exist()) e += p->ExtSockets();
	if (stRuneWordPropertyList.exist()) e += stRuneWordPropertyList->ExtSockets();
	e = min(MAX_SOCKETS - b, e);
	return make_pair(b, e);
}

BOOL CTypeSpecificInfo::IsIndestructible() const {
	if ((iMaxDurability.exist() && 0 == iMaxDurability) || stPropertyList.IsIndestructible()) return TRUE;
	for (auto& p : apSetProperty) if (p.exist() && p->IsIndestructible()) return TRUE;
	if (stRuneWordPropertyList.exist() && stRuneWordPropertyList->IsIndestructible()) return TRUE;
	return FALSE;
}

// struct CItemInfo

CItemInfo::CItemInfo(const CItemMetaData* meta) {
	if (meta) {
		dwTypeID = meta->dwTypeID;
		if (IsGold()) pGold.ensure();
		if (!meta->Simple) { pExtItemInfo.ensure(meta); pTpSpInfo.ensure(meta); }
		iPad = meta->iPad;
	}
}

const CItemMetaData* CItemInfo::ReadData(CInBitsStream& bs, DWORD version, BOOL bSimple, BOOL bRuneWord, BOOL bPersonalized, BOOL bSocketed) {
	const BOOL isD2R = IsD2R(version);
	for (auto& b : sTypeName)
		if (isD2R) b = g_huffmanTree.readData(bs);
		else bs >> bits(b, 8);
	auto pItemData = g_dataMgr->itemMetaData(dwTypeID);
	if (!pItemData) {
		if (IsNameValid()) throw D2Error(6);
		else throw D2Error(18);
	}
	if (!bSimple) {
		pExtItemInfo.ensure().ReadData(bs, version, pItemData->IsCharm, bRuneWord, bPersonalized, pItemData->HasMonsterID, pItemData->SpellId);
		if (IsGold()) pGold.ensure().ReadData(bs);
		bs >> bHasRand;
			if (bHasRand) {
				// v105: 3个时间戳 (96 bits), 旧版: 4个 (128 bits)
				const int nTimestamps = IsRotWAndAbove(version) ? 3 : 4;
				int tscount = 0;
				for (auto& i : pTimeStampFlag.ensure()) {
					if (tscount++ >= nTimestamps || !bs.Good()) break;
					bs >> bits(i, 32);
				}
			}
		pTpSpInfo.ensure().ReadData(bs, version, pItemData->HasDef, pItemData->HasDur, bSocketed, pItemData->IsStacked, pExtItemInfo->IsSet(), bRuneWord);
	} else {
		if (IsGold()) pGold.ensure().ReadData(bs);
		if (isD2R && pItemData->iPadBits > 0)
			bs >> bits(iPad, pItemData->iPadBits);
	}
	return pItemData;
}

void CItemInfo::WriteData(COutBitsStream& bs, const CItemMetaData& itemData, DWORD version, BOOL bSimple, BOOL bRuneWord, BOOL bPersonalized, BOOL bSocketed) const {
	const BOOL isD2R = IsD2R(version);
	for (auto b : sTypeName)
		if (isD2R) g_huffmanTree.writeData(bs, b);
		else bs << bits(b, 8);
	if (!bSimple) {
		pExtItemInfo->WriteData(bs, version, itemData.IsCharm, bRuneWord, bPersonalized, itemData.HasMonsterID, itemData.SpellId);
		if (IsGold()) pGold->WriteData(bs);
		bs << bHasRand;
		if (bHasRand) {
			// v105: 3个时间戳 (96 bits)
			const int nTimestamps = IsRotWAndAbove(version) ? 3 : 4;
			int tscount = 0;
			for (auto i : pTimeStampFlag) {
				if (tscount++ >= nTimestamps || !bs.Good()) break;
				bs << bits(i, 32);
			}
		}
		pTpSpInfo->WriteData(bs, version, itemData.HasDef, itemData.HasDur, bSocketed, itemData.IsStacked, pExtItemInfo->IsSet(), bRuneWord);
	} else {
	if (IsGold()) pGold->WriteData(bs);
		if (isD2R && itemData.iPadBits > 0)
			bs << bits(iPad, itemData.iPadBits);
	}
}

BOOL CItemInfo::IsNameValid() const {
	for (char c : sTypeName)
		if (c != ' ' && !isalnum(c)) return FALSE;
	return TRUE;
}

// CD2Item

typedef deque<QString> __Tokens;

static QString text(const __Tokens& tokens) {
	QString ret;
	int i = 0;
	for (auto& t : tokens) {
		if (t.isEmpty()) continue;
		if (i++) ret += QStringLiteral(" ");
		ret += t;
	}
	return ret;
}

CD2Item::CD2Item(DWORD type) {
	if (type > 0) {
		pItemData = g_dataMgr->itemMetaData(type);
		ASSERT(pItemData);
		bNew = pItemData->IsNew;
		bSimple = pItemData->Simple;
		if (0x20726165 == type) {	// "ear "
			bEar = TRUE;
			pEar.ensure(QStringLiteral("unknown"));
		} else {
			pItemInfo.ensure(pItemData);
		}
	}
}

QString CD2Item::ItemName() const {
	if (bEar) return CSFormat(g_dataMgr->itemSuspendUI(10).toUtf8().constData(), QString(pEar->sEarName).toUtf8().constData());
	__Tokens name{ g_dataMgr->itemName(MetaData().NameIndex) };
	if (!bSimple) {
		ASSERT(pItemInfo->pExtItemInfo.exist());
		auto& extInfo = *pItemInfo->pExtItemInfo;
		switch (extInfo.iQuality) {
		case 1: name.push_front(g_dataMgr->itemSuspendUI(0)); break;
		case 3: name.push_front(g_dataMgr->itemSuspendUI(1)); break;
		case 4:
			name.push_front(g_dataMgr->magicPrefix(extInfo.wPrefix));
			name.push_back(g_dataMgr->magicSuffix(extInfo.wSuffix));
			break;
		case 5: name.push_front(g_dataMgr->setItemName(extInfo.wSetID)); break;
		case 6: {
			const auto& rare = *extInfo.pRareName;
			name.push_front(g_dataMgr->rareCraftedName(rare.iName2)); name.push_front(g_dataMgr->rareCraftedName(rare.iName1));
			break;
		}
		case 7: name.push_front(g_dataMgr->uniqueName(extInfo.wUniID)); break;
		case 8: {
			const auto& craft = *extInfo.pCraftName;
			name.push_front(g_dataMgr->rareCraftedName(craft.iName2)); name.push_front(g_dataMgr->rareCraftedName(craft.iName1));
			break;
		}
		default:
			if (extInfo.wMonsterID.exist()) name.push_front(g_dataMgr->monsterName(extInfo.wMonsterID));
		}
		if (extInfo.iQuality <= 3 && IsRuneWord()) {
			const QString runewordName = g_dataMgr->runeWordName(RuneWordId());
			name.push_front(runewordName);
		}
		if (bEthereal) name.push_back(QStringLiteral("(ETH)"));
	}
	return text(name);
}

int CD2Item::GemIndexMax() const {
	int r = -1;
	for (auto& i : aGemItems) r = max(r, (int)i.iColumn);
	return r;
}

void CD2Item::ReadData(CInBitsStream& bs, DWORD version) {
	bs >> wMajic;
	if (wMajic != 0x4D4A) { bs.SeekBack(sizeof wMajic); wMajic = 0x4D4A; }
	bs >> dwVersion;
	if (!IsValidVersion(dwVersion)) { bs.SeekBack(sizeof dwVersion); dwVersion = version; }
	bs >> bQuest >> bits(iUNKNOWN_01, 3) >> bIdentified >> bits(iUNKNOWN_02, 3)
		>> bDisabled >> bits(iUNKNOWN_10, 2) >> bSocketed >> bUNKNOWN_03
		>> bNew >> bBadEquipped >> bUNKNOWN_04 >> bEar >> bNewbie
		>> bits(iUNKNOWN_05, 3) >> bSimple >> bEthereal >> bUNKNOWN_06
		>> bPersonalized >> bUNKNOWN_07 >> bRuneWord
		>> bits(iUNKNOWN_11, 3)		// v105: 32-bit flags 的最后5位
		>> bits(iUNKNOWN_13, 2);
	if (IsD2R(dwVersion)) {
		if (IsRotWAndAbove(dwVersion))
			bs >> bits(iUNKNOWN_09, 3);	// v105: 3 bits extension (=5)
		else
			bs >> bits(iUNKNOWN_09, 8);	// v97-v99: 8 bits extension
	} else
		bs >> bits(iUNKNOWN_08, 5) >> bits(wVersion, 10);
	if (IsD2R(dwVersion))
		bs >> bits(iLocation, 3) >> bits(iPosition, 4) >> bits(iColumn, 4) >> bits(iRow, 3) >> bUNKNOWN_12 >> bits(iStoredIn, 3);
	else
		bs >> bits(iLocation, 3) >> bits(iPosition, 4) >> bits(iColumn, 4) >> bits(iRow, 4) >> bits(iStoredIn, 3);
	if (bEar) {
		pEar.ensure().ReadData(bs, dwVersion);
		pItemData = g_dataMgr->itemMetaData(0x20726165);	// "ear "
	} else {
		pItemData = pItemInfo.ensure().ReadData(bs, dwVersion, bSimple, bRuneWord, bPersonalized, bSocketed);
	}
	ASSERT(pItemData);
	bs.AlignByte();
	aGemItems.resize(Gems());
	for (auto& item : aGemItems) item.ReadData(bs, dwVersion);
}

void CD2Item::WriteData(COutBitsStream& bs, DWORD version) const {
	const BOOL bExport = !version;
	if (!version) version = dwVersion;
	if (!vUnknownItem.empty()) {
		bs << vUnknownItem;
	} else {
		const BOOL isD2R = IsD2R(version);
		if (!isD2R || bExport) bs << WORD(0x4D4A);
		if (bExport && IsD2R(version)) bs << version;
		bs << bQuest << bits(iUNKNOWN_01, 3) << bIdentified << bits(iUNKNOWN_02, 3)
			<< BOOL(FALSE) << bits(iUNKNOWN_10, 2) << bSocketed << bUNKNOWN_03
			<< bNew << bBadEquipped << bUNKNOWN_04 << bEar << bNewbie
			<< bits(iUNKNOWN_05, 3) << bSimple << bEthereal << bUNKNOWN_06
			<< bPersonalized << bUNKNOWN_07 << bRuneWord
			<< bits(iUNKNOWN_11, 3)		// v105: 32-bit flags 的最后5位
			<< bits(iUNKNOWN_13, 2);
		if (isD2R) {
			if (IsRotWAndAbove(version))
				bs << bits(BYTE(5), 3);	// v105: 3 bits extension = 5
			else
				bs << bits(iUNKNOWN_09, 8);	// v97-v99: 8 bits extension
		} else
			bs << bits(iUNKNOWN_08, 5) << bits(wVersion, 10);
		if (isD2R)
			bs << bits(iLocation, 3) << bits(iPosition, 4) << bits(iColumn, 4) << bits(iRow, 3) << bUNKNOWN_12 << bits(iStoredIn, 3);
		else
			bs << bits(iLocation, 3) << bits(iPosition, 4) << bits(iColumn, 4) << bits(iRow, 4) << bits(iStoredIn, 3);
		if (bEar) pEar->WriteData(bs, version);
		else pItemInfo->WriteData(bs, *pItemData, version, bSimple, bRuneWord, bPersonalized, bSocketed);
	}
	bs.AlignByte();
	for (auto item : aGemItems) if (bs.Good()) item.WriteData(bs, version);
}

BOOL CD2Item::ReadFile(QFile& file) {
	CInBitsStream bs;
	bs.ReadFile(file);
	try { ReadData(bs, 0); }
	catch (...) { return FALSE; }
	return TRUE;
}

void CD2Item::WriteFile(QFile& file) const {
	COutBitsStream bs;
	WriteData(bs, 0);
	bs.WriteFile(file);
}

// struct CItemList

void CItemList::ReadData(CInBitsStream& bs, DWORD version) {
	WORD nItems;
	bs >> wMajic >> nItems;
	if (wMajic != 0x4D4A) throw D2Error(17);
	vItems.resize(nItems);
	for (auto& item : vItems) { if (!bs.Good()) break; item.ReadData(bs, version); }
}

void CItemList::WriteData(COutBitsStream& bs, DWORD version) const {
	bs << WORD(0x4D4A) << WORD(vItems.size());
	for (auto& item : vItems) { if (!bs.Good()) break; item.WriteData(bs, version); }
}
