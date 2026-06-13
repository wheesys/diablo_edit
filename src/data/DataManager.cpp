#include "DataManager.h"

#include <QFile>
#include <QCoreApplication>
#include <QLocale>
#include <QDebug>

#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <cctype>
#include <numeric>
#include <algorithm>

using namespace std;

const int DataManager::CLASS_SKILL_INDEX[CLASS_NAME_SIZE][CLASS_SKILL_NAME_SIZE] = {
	{0,1,10,11,20,2,3,12,21,22,4,13,14,23,24,5,6,15,25,26,7,8,16,17,27,9,18,19,28,29},
	{0,1,10,20,21,2,11,12,22,23,3,4,13,14,24,5,6,15,16,25,7,17,18,26,27,8,9,19,28,29},
	{0,10,11,20,21,1,2,12,13,22,3,4,14,23,24,5,6,15,16,25,7,8,17,26,27,9,18,19,28,29},
	{0,1,10,20,21,2,11,12,22,23,3,4,13,24,25,5,6,14,15,26,7,8,16,17,27,9,18,19,28,29},
	{0,10,11,12,20,21,1,2,13,14,15,22,23,3,4,16,24,5,6,17,25,7,18,26,27,8,9,19,28,29},
	{0,1,10,11,20,2,3,12,21,22,4,13,14,23,24,5,6,15,16,25,7,17,18,26,27,8,9,19,28,29},
	{0,10,11,20,21,1,2,12,22,23,3,4,13,14,24,5,15,16,25,26,6,7,17,27,28,8,9,18,19,29},
	{0,1,2,10,11,3,4,12,13,20,5,6,14,21,22,7,8,15,16,23,24,9,17,18,25,26,27,19,28,29}
};

DataManager::DataManager() {
	QLocale locale;
	QString lang = locale.name();
	if (lang.startsWith(QStringLiteral("zh_CN"))) m_nLangIndex = 1;
	else if (lang.startsWith(QStringLiteral("zh_TW")) || lang.startsWith(QStringLiteral("zh_HK"))) m_nLangIndex = 2;
	else m_nLangIndex = 0;
}

DataManager::~DataManager() = default;

// D2Error 消息ID构造函数实现
D2Error::D2Error(int msgId)
    : std::runtime_error("D2Error")
    , m_msg(g_dataMgr ? g_dataMgr->msg(msgId) : QStringLiteral("Error %1").arg(msgId))
    , m_code(msgId)
{
}

bool DataManager::loadAll(const QString& dataPath) {
	m_sDataPath = dataPath;
	g_dataMgr = this; // 必须在 readNewChar() 之前设置，Item 解析依赖此全局指针
	qDebug() << "  loadAll: reading lang...";
	if (!readLangRes()) return false;
	qDebug() << "  loadAll: reading items...";
	if (!readItemRes()) return false;
	qDebug() << "  loadAll: reading props...";
	if (!readPropRes()) return false;
	qDebug() << "  loadAll: reading newchar...";
	if (!readNewChar()) return false;
	qDebug() << "  loadAll: done";
	return true;
}

// --- File loading utilities ---

string DataManager::loadCompressedFile(const QString& path, const char magic[4]) {
	QFile file(path);
	if (!file.open(QFile::ReadOnly)) return string();
	QByteArray data = file.readAll();
	file.close();
	string in_buf(data.constData(), data.size());
	string out_buf;
	if (!CCompressorQuickLZ().decompress(in_buf, out_buf)) return string();
	if (out_buf.size() < 5) return string();
	if (out_buf.substr(1, 4) != magic) return string();
	return out_buf;
}

string DataManager::loadRawFile(const QString& path) {
	QFile file(path);
	if (!file.open(QFile::ReadOnly)) return string();
	QByteArray data = file.readAll();
	file.close();
	return string(data.constData(), data.size());
}

// --- Internal helpers ---

static inline string trim(const string& str) {
	size_t i = 0;
	for (; i < str.length() && isspace(static_cast<unsigned char>(str[i])); ++i);
	return (i ? str.substr(i) : str);
}

static string escape(string str) {
	string ret;
	size_t pp = 0;
	for (size_t p = 0;;) {
		auto i = str.find_first_of('\\', p);
		if (i == string::npos) break;
		p = i + 1;
		if (p < str.size()) {
			const char c = str[p], n = (c == 'n' ? '\n' : (c == '\\' ? '\\' : 0));
			if (n) {
				ret.append(str.substr(pp, i - pp));
				ret.push_back(n);
				pp = ++p;
			}
		}
	}
	ret.append(str.substr(pp));
	return ret;
}

template<typename T>
static inline bool parse(istream& is, T& value) {
	if (!is.good()) return false;
	string s;
	getline(is, s, '\t');
	value = (s.empty() ? 0 : stoi(s, nullptr, 0));
	return !is.bad();
}

static bool parse(istream& is, string& str) {
	if (is.good()) getline(is, str, '\t');
	return !is.bad();
}

static bool parse(istream& is, vector<string>& strs) {
	if (is.good()) for (string s; getline(is, s, '\t'); strs.push_back(s));
	return !is.bad();
}

// --- Data loading ---

bool DataManager::readLangRes() {
	string out_buf = loadCompressedFile(m_sDataPath + QStringLiteral("/language.dat"), "LANG");
	if (out_buf.empty()) return false;
	istringstream iss(out_buf);
	decltype(m_saLanguage) langs;
	decltype(m_aLangBases) bases;
	UINT base = 0, idx = 0;
	for (string line; getline(iss, line);) {
		if (trim(line).empty()) continue;
		else if (line[0] == '*') {
			base = (langs.empty() ? 0 : (UINT)langs[0].size());
			idx = 0;
			if (base) bases.push_back(base);
			continue;
		}
		istringstream ls(line);
		UINT gidx;
		vector<string> msgs;
		parse(ls, gidx) && parse(ls, msgs);
		if (!gidx) gidx = idx++;
		else idx = gidx + 1;
		gidx += base;
		if (msgs.size() > langs.size()) langs.resize(msgs.size());
		for (size_t i = 0; i < msgs.size(); ++i) {
			auto& lang = langs[i];
			auto& msg = msgs[i];
			if (msg.size() > 2 && msg[0] == '"' && *msg.rbegin() == '"')
				msg = msg.substr(1, msg.size() - 2);
			msg = escape(msg);
			if (gidx < lang.size()) {
				lang[gidx] = QString::fromUtf8(msg.c_str());
			} else {
				if (gidx > lang.size()) lang.resize(gidx);
				lang.emplace_back(QString::fromUtf8(msg.c_str()));
			}
		}
	}
	if (!langs.empty()) bases.push_back((UINT)langs[0].size());
	for (size_t i = 1; i < langs.size(); ++i) {
		auto& lang = langs[i];
		const auto& prev = langs[i - 1];
		if (lang.size() < prev.size()) lang.resize(prev.size());
		for (size_t j = 0; j < lang.size(); ++j)
			if (lang[j].isEmpty()) lang[j] = prev[j];
	}
	reverse(bases.begin(), bases.end());
	bases.push_back(0);
	langs.swap(m_saLanguage);
	bases.swap(m_aLangBases);
	return true;
}

bool DataManager::readItemRes() {
	string out_buf = loadCompressedFile(m_sDataPath + QStringLiteral("/itemdata.dat"), "ITEM");
	if (out_buf.empty()) return false;
	istringstream iss(out_buf);
	decltype(m_vItemMetaData) sections;
	sections.reserve(30);
	decltype(m_mIdToMetaData) idMap;
	for (string line; getline(iss, line);) {
		line = trim(line);
		if (line.empty() || line.substr(0, 5) == "*ITEM") continue;
		else if (line[0] == '*') { sections.emplace_back(); continue; }
		istringstream ls(line);
		string id;
		int pic = 0, range = 0;
		CItemMetaData item;
		parse(ls, id) && parse(ls, pic) && parse(ls, item.NameIndex)
			&& parse(ls, range) && parse(ls, item.Equip) && parse(ls, item.Simple)
			&& parse(ls, item.Normal) && parse(ls, item.White) && parse(ls, item.IsNew)
			&& parse(ls, item.HasDef) && parse(ls, item.HasDur) && parse(ls, item.IsStacked)
			&& parse(ls, item.HasMonsterID);
		parse(ls, item.IsCharm) && parse(ls, item.SpellId) && parse(ls, item.IsUnique)
			&& parse(ls, item.IsCraft) && parse(ls, item.IsGem);
		// Damage fields
		int dmg1Min = 0, dmg1Max = 0, dmg2Min = 0, dmg2Max = 0;
		parse(ls, dmg1Min) && parse(ls, dmg1Max) && parse(ls, dmg2Min) && parse(ls, dmg2Max);
		item.Damage1Min = dmg1Min; item.Damage1Max = dmg1Max;
		item.Damage2Min = dmg2Min; item.Damage2Max = dmg2Max;
		parse(ls, item.iPadBits);
		// Type name
		if (id.size() > 4) id = id.substr(0, 4);
		else while (id.size() < 4) id.push_back(' ');
		CopyMemory(item.sTypeName, id.c_str(), 4);
		// Pic index
		item.PicIndex = static_cast<WORD>(pic);
		item.Range = static_cast<BYTE>(range);
		// Add to section
		if (sections.empty()) sections.emplace_back();
		UINT secIdx = (UINT)sections.size() - 1;
		UINT itemIdx = (UINT)sections.back().size();
		idMap[item.dwTypeID] = make_pair(secIdx, itemIdx);
		sections.back().push_back(item);
	}
	sections.swap(m_vItemMetaData);
	idMap.swap(m_mIdToMetaData);
	return true;
}

bool DataManager::readPropRes() {
	string out_buf = loadCompressedFile(m_sDataPath + QStringLiteral("/property.dat"), "PROP");
	if (out_buf.empty()) return false;
	istringstream iss(out_buf);
	decltype(m_vPropertyMetaData) props;
	for (string line; getline(iss, line);) {
		line = trim(line);
		if (line.empty() || line[0] == '*') continue;
		istringstream ls(line);
		UINT id, verMin;
		vector<CPropertyField> fields;
		DWORD def;
		parse(ls, id);
		if (id >= props.size()) props.resize(id + 1);
		parse(ls, verMin);
			// 读取所有剩余制表分隔数值，取最后一个为DefaultValue
			std::vector<int> vals;
			for (int v; parse(ls, v); ) vals.push_back(v);
			def = vals.empty() ? 0 : vals.back();
			if (!vals.empty()) vals.pop_back();
			// 按4个一组(bits,base,min,max)解析属性字段
			for (size_t i = 0; i + 3 < vals.size(); i += 4) {
				if (vals[i] > 0) {
					CPropertyField f;
					f.bits = vals[i];
					f.base = vals[i+1];
					f.min = vals[i+2];
					f.max = vals[i+3];
					fields.push_back(f.Normalize());
				}
			}
		props[id].addData(CPropertyMetaDataItem(verMin, fields, def));
	}
	props.swap(m_vPropertyMetaData);
	return true;
}

bool DataManager::readNewChar() {
	string data = loadRawFile(m_sDataPath + QStringLiteral("/new.dat"));
	if (data.empty()) {
		qWarning() << "readNewChar: new.dat not found or empty, new character template unavailable";
		return true; // 非关键数据，不影响程序启动
	}
	try {
		CInBitsStream bs(reinterpret_cast<const BYTE*>(data.data()), data.size());
		m_stNewCharacter.ReadData(bs);
		return true;
	} catch (const D2Error& e) {
		qWarning() << "readNewChar: new.dat parse failed:" << e.message()
					<< ", new character template unavailable";
		return true; // 非关键数据，不影响程序启动
	}
}

// --- Item/Property metadata lookup ---

const CItemMetaData* DataManager::itemMetaData(DWORD typeID) const {
	auto it = m_mIdToMetaData.find(typeID);
	if (it == m_mIdToMetaData.end()) return nullptr;
	const auto& idx = it->second;
	if (idx.first >= m_vItemMetaData.size()) return nullptr;
	const auto& sec = m_vItemMetaData[idx.first];
	if (idx.second >= sec.size()) return nullptr;
	return &sec[idx.second];
}

const CPropertyMetaDataItem& DataManager::propertyMetaData(DWORD version, UINT index) const {
	ASSERT(index < m_vPropertyMetaData.size());
	return m_vPropertyMetaData[index].findData(version);
}

// --- String lookup helpers ---

QString DataManager::getString(UINT index) const {
	ASSERT(m_nLangIndex < (INT)m_saLanguage.size());
	ASSERT(index < m_saLanguage[m_nLangIndex].size());
	return m_saLanguage[m_nLangIndex][index];
}

UINT DataManager::sectionSize(LangSection section) const {
	ASSERT(m_aLangBases[section - 1] >= m_aLangBases[section]);
	return m_aLangBases[section - 1] - m_aLangBases[section];
}

UINT DataManager::sectionToIndex(LangSection section, UINT index) const {
	ASSERT(index < sectionSize(section));
	return index + m_aLangBases[section];
}

// --- Language string forwarding ---

#define D2_LANG_GETTER(name, section) \
	QString DataManager::name(UINT index) const { return getString(sectionToIndex(section, index)); }

D2_LANG_GETTER(langTitle, DUMMY_START)
D2_LANG_GETTER(menuPrompt, MENU_PROMPT)
D2_LANG_GETTER(charItemPopupMenu, CHAR_ITEM_POPUP_MENU)
D2_LANG_GETTER(systemMenu, SYSTEM_MENU)
D2_LANG_GETTER(otherUI, OTHER_UI)
D2_LANG_GETTER(newItemUI, NEW_ITEM_UI)
D2_LANG_GETTER(foundryUI, FOUNDRY_UI)
D2_LANG_GETTER(itemSuspendUI, ITEM_SUSPEND_UI)
D2_LANG_GETTER(charItemsUI, CHAR_ITEMS_UI)
D2_LANG_GETTER(charBasicInfoUI, CHAR_BASIC_INFO_UI)
D2_LANG_GETTER(msgBoxInfo, MSG_BOX_INFO)
D2_LANG_GETTER(mercenaryBarbarianName, MERCENARY_NAME_BAR)
D2_LANG_GETTER(mercenarySorcerorName, MERCENARY_NAME_SOR)
D2_LANG_GETTER(mercenaryMercName, MERCENARY_NAME_MERC)
D2_LANG_GETTER(mercenaryScoutName, MERCENARY_NAME_SCOUT)
D2_LANG_GETTER(mercenaryTypeName, MERCENARY_TYPE_NAME)
D2_LANG_GETTER(itemCatagoryName, ITEM_CATAGORY_NAME)
D2_LANG_GETTER(itemQualityName, ITEM_QUALITY_NAME)
D2_LANG_GETTER(difficultyName, DIFFICULTY_NAME)
D2_LANG_GETTER(wayPointName, WAY_POINT_NAME)
D2_LANG_GETTER(questName, QUEST_NAME)
D2_LANG_GETTER(normalSkillName, NORMAL_SKILL_NAME)
D2_LANG_GETTER(timeName, TIME_NAME)
D2_LANG_GETTER(className, CLASS_NAME)
D2_LANG_GETTER(itemName, ITEM_NAME)
D2_LANG_GETTER(propertyName, PROPERTY_NAME)
D2_LANG_GETTER(monsterName, MONSTER_NAME)
D2_LANG_GETTER(magicPrefix, MAGIC_PREFIX)
D2_LANG_GETTER(magicSuffix, MAGIC_SUFFIX)
D2_LANG_GETTER(rareCraftedName, RARE_CRAFTED_NAME)
D2_LANG_GETTER(uniqueName, UNIQUE_ITEM_NAME)
D2_LANG_GETTER(setItemName, SET_ITEM_NAME)

#undef D2_LANG_GETTER

QString DataManager::runeWordName(UINT index) const {
	return index < sectionSize(RUNE_WORD_NAME) ? getString(sectionToIndex(RUNE_WORD_NAME, index)) : QString();
}

QString DataManager::classSkillName(UINT index, UINT class_id) const {
	ASSERT(index < CLASS_SKILL_NAME_SIZE && class_id < CLASS_NAME_SIZE);
	return getString(sectionToIndex(CLASS_SKILL_NAME, class_id * CLASS_SKILL_NAME_SIZE + index));
}

QString DataManager::classSkillTabName(UINT tab, UINT class_id) const {
	ASSERT(tab < CLASS_SKILL_TAB_SIZE && class_id < CLASS_NAME_SIZE);
	return getString(sectionToIndex(CLASS_SKILL_TAB_NAME, class_id * CLASS_SKILL_TAB_SIZE + tab));
}

pair<QString, int> DataManager::classSkillName(UINT skill_id) const {
	for (UINT c = 0; c < CLASS_NAME_SIZE; ++c)
		for (UINT s = 0; s < CLASS_SKILL_NAME_SIZE; ++s)
			if ((UINT)CLASS_SKILL_INDEX[c][s] == skill_id)
				return make_pair(classSkillName(s, c), (int)c);
	return make_pair(QString(), -1);
}

// --- Convenience methods ---

QString DataManager::msg(UINT index) const { return getString(sectionToIndex(OTHER_MSG, index)); }

QString DataManager::msgWarning() const { return msg(1); }
QString DataManager::msgError() const { return msg(2); }
QString DataManager::msgNoTitle() const { return msg(3); }
QString DataManager::msgUnknown() const { return msg(4); }

// --- Size queries ---

#define D2_SIZE_GETTER(name, section) \
	UINT DataManager::name() const { return sectionSize(section); }

D2_SIZE_GETTER(itemQualityNameSize, ITEM_QUALITY_NAME)
D2_SIZE_GETTER(propertyNameSize, PROPERTY_NAME)
D2_SIZE_GETTER(timeNameSize, TIME_NAME)
D2_SIZE_GETTER(difficultyNameSize, DIFFICULTY_NAME)
D2_SIZE_GETTER(magicPrefixSize, MAGIC_PREFIX)
D2_SIZE_GETTER(magicSuffixSize, MAGIC_SUFFIX)
D2_SIZE_GETTER(rareCraftedNameSize, RARE_CRAFTED_NAME)
D2_SIZE_GETTER(uniqueNameSize, UNIQUE_ITEM_NAME)
D2_SIZE_GETTER(setItemNameSize, SET_ITEM_NAME)
D2_SIZE_GETTER(monsterNameSize, MONSTER_NAME)
D2_SIZE_GETTER(mercenaryTypeNameSize, MERCENARY_TYPE_NAME)
D2_SIZE_GETTER(mercenaryNameScoutSize, MERCENARY_NAME_SCOUT)
D2_SIZE_GETTER(mercenaryNameMercSize, MERCENARY_NAME_MERC)
D2_SIZE_GETTER(mercenaryNameSorcerorSize, MERCENARY_NAME_SOR)
D2_SIZE_GETTER(mercenaryNameBarbarianSize, MERCENARY_NAME_BAR)

#undef D2_SIZE_GETTER

// --- Property description (ported from MFC original CDiabloEdit2App::PropertyDescription) ---

QString DataManager::propertyDescription(DWORD version, WORD id, DWORD value) const {
	const auto& meta = propertyMetaData(version, id);
	QString desc = propertyName(id);
	auto a = meta.Parse(value);

	if (a.size() < 4) {
		a.resize(4, 0);
	}

	switch (id) {
		case 54: a[2] /= 25; break;
		case 56: case 59: a[0] /= 25; break;
		case 57:
			a[0] = (a[0] * a[2] + 128) >> 8;
			a[1] = (a[1] * a[2] + 128) >> 8;
			a[2] /= 25;
			break;
		case 83:
			return desc.arg(a[1]).arg(className(a[0]));
		case 97: case 151:
			return desc.arg(a[1]).arg(classSkillName(a[0]).first);
		case 98:
			return desc.arg(1 <= a[0] && a[0] <= 5 ? msg(4 + a[0]) : msg(10));
		case 107: {
			auto p = classSkillName(a[0]);
			return desc.arg(a[1]).arg(p.first).arg(className(p.second));
		}
		case 112: a[0] = a[0] * 100 / 127; break;
		case 126: std::swap(a[0], a[1]); break;
		case 155: case 179: case 180:
			return desc.arg(a[1]).arg(monsterName(a[0]));
		case 188:
			return desc.arg(a[2]).arg(classSkillTabName(a[0], a[1])).arg(className(a[1]));
		case 195: case 196: case 197: case 198: case 199: case 201:
			return desc.arg(a[2]).arg(a[0]).arg(classSkillName(a[1]).first);
		case 204:
			return desc.arg(a[0]).arg(classSkillName(a[1]).first).arg(a[2]).arg(a[3]);
		// double / 8 cases
		case 214: case 215: case 216: case 217: case 218: case 219: case 220:
		case 221: case 222: case 223: case 226: case 227: case 228: case 229:
		case 230: case 231: case 232: case 233: case 234: case 235: case 236:
		case 237: case 238: case 239: case 240: case 241: case 242: case 243:
		case 244: case 245: case 246: case 247: case 248: case 249: case 250:
			return desc.arg(double(a[0]) / 8);
		case 224: case 225:
			return desc.arg(double(a[0]) / 2);
		case 252: case 253: a[0] = 100 / a[0]; break;
		// time format cases
		case 268: case 269: case 270: case 271: case 272: case 273: case 274:
		case 275: case 276: case 277: case 278: case 279: case 280: case 281:
		case 282: case 283: case 284: case 285: case 286: case 287: case 288:
		case 289: case 290: case 291: case 292: case 293: case 294: case 295:
		case 296: case 297: case 298: case 299: case 300: case 301: case 302:
		case 303:
			return desc.arg(a[1]).arg(a[2]).arg(timeName(a[0]));
		case 305: case 306: case 307: case 308: case 333: case 334: case 335:
		case 336:
			a[0] = -a[0]; break;
	}
	return desc.arg(a[0]).arg(a[1]).arg(a[2]).arg(a[3]);
}

QString DataManager::propertyDescription(DWORD version, WORD id) const {
	const auto& propItem = propertyMetaData(version, id);
	return propertyDescription(version, id, propItem.DefaultValue());
}

std::vector<CPropParam> DataManager::propertyParameters(DWORD version, WORD id, DWORD value) const {
	const auto& propItem = propertyMetaData(version, id);
	auto rawParams = propItem.GetParams(value);
	std::vector<CPropParam> result;
	for (auto& [val, min, max] : rawParams) {
		CPropParam p;
		p.iValue = val;
		p.iMin = min;
		p.iMax = max;
		result.push_back(p);
	}
	return result;
}
