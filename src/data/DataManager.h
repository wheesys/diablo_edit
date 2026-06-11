#pragma once

#include "core/D2Types.h"
#include "core/MetaData.h"
#include "core/D2S_Struct.h"
#include "core/compress_quicklz.h"

#include <QString>
#include <QByteArray>
#include <vector>
#include <unordered_map>

struct CPropParam {
	int iValue;
	int iMin;
	int iMax;
};

class DataManager
{
public:
	DataManager();
	~DataManager();

	// 初始化：加载所有数据文件
	bool loadAll(const QString& dataPath);

	// 角色常量
	static const UINT CLASS_NAME_SIZE = 8;
	static const UINT CLASS_SKILL_TAB_SIZE = 3;
	static const UINT CLASS_SKILL_NAME_SIZE = 30;
	static const int CLASS_SKILL_INDEX[CLASS_NAME_SIZE][CLASS_SKILL_NAME_SIZE];

	// 语言设置
	INT langIndex() const { return m_nLangIndex; }
	void setLangIndex(INT idx) { m_nLangIndex = idx; }
	INT langCount() const { return m_saLanguage.size(); }

	// 物品元数据
	const std::vector<std::vector<CItemMetaData>>& itemCategories() const { return m_vItemMetaData; }
	const CItemMetaData* itemMetaData(DWORD typeID) const;
	const CPropertyMetaDataItem& propertyMetaData(DWORD version, UINT index) const;
	const CD2S_Struct& newCharacter() const { return m_stNewCharacter; }

	// 属性描述
	QString propertyDescription(DWORD version, WORD id, DWORD value) const;
	QString propertyDescription(DWORD version, WORD id) const;
	std::vector<CPropParam> propertyParameters(DWORD version, WORD id, DWORD value) const;

	// 语言字符串查找
	QString langTitle(UINT index) const;
	QString msgWarning() const;
	QString msgError() const;
	QString msgNoTitle() const;
	QString msgUnknown() const;
	QString msg(UINT index) const;

	// UI 字符串
	QString menuPrompt(UINT index) const;
	QString charItemPopupMenu(UINT index) const;
	QString systemMenu(UINT index) const;
	QString otherUI(UINT index) const;
	QString newItemUI(UINT index) const;
	QString foundryUI(UINT index) const;
	QString itemSuspendUI(UINT index) const;
	QString charItemsUI(UINT index) const;
	QString charBasicInfoUI(UINT index) const;
	QString msgBoxInfo(UINT index) const;

	// 雇佣兵名称
	QString mercenaryBarbarianName(UINT index) const;
	QString mercenarySorcerorName(UINT index) const;
	QString mercenaryMercName(UINT index) const;
	QString mercenaryScoutName(UINT index) const;
	QString mercenaryTypeName(UINT index) const;

	// 物品名称
	QString itemCatagoryName(UINT index) const;
	QString itemQualityName(UINT index) const;
	QString difficultyName(UINT index) const;
	QString wayPointName(UINT index) const;
	QString questName(UINT index) const;
	QString normalSkillName(UINT index) const;
	std::pair<QString, int> classSkillName(UINT skill_id) const;
	QString classSkillName(UINT index, UINT class_id) const;
	QString classSkillTabName(UINT tab, UINT class_id) const;
	QString timeName(UINT index) const;
	QString className(UINT index) const;
	QString itemName(UINT index) const;
	QString propertyName(UINT index) const;
	QString monsterName(UINT index) const;
	QString magicPrefix(UINT index) const;
	QString magicSuffix(UINT index) const;
	QString rareCraftedName(UINT index) const;
	QString uniqueName(UINT index) const;
	QString setItemName(UINT index) const;
	QString runeWordName(UINT index) const;

	// 大小查询
	UINT itemQualityNameSize() const;
	UINT propertyNameSize() const;
	UINT timeNameSize() const;
	UINT difficultyNameSize() const;
	UINT magicPrefixSize() const;
	UINT magicSuffixSize() const;
	UINT rareCraftedNameSize() const;
	UINT uniqueNameSize() const;
	UINT setItemNameSize() const;
	UINT monsterNameSize() const;
	UINT mercenaryTypeNameSize() const;
	UINT mercenaryNameScoutSize() const;
	UINT mercenaryNameMercSize() const;
	UINT mercenaryNameSorcerorSize() const;
	UINT mercenaryNameBarbarianSize() const;

private:
	QString m_sDataPath;
	INT m_nLangIndex = 0;
	std::vector<std::vector<QString>> m_saLanguage;
	std::vector<UINT> m_aLangBases;

	std::vector<std::vector<CItemMetaData>> m_vItemMetaData;
	std::unordered_map<DWORD, std::pair<UINT, UINT>> m_mIdToMetaData;
	std::vector<CPropertyMetaData> m_vPropertyMetaData;
	CD2S_Struct m_stNewCharacter;

	enum LangSection {
		DUMMY_START,
		RUNE_WORD_NAME,
		SET_ITEM_NAME,
		UNIQUE_ITEM_NAME,
		RARE_CRAFTED_NAME,
		MAGIC_SUFFIX,
		MAGIC_PREFIX,
		MONSTER_NAME,
		PROPERTY_NAME,
		ITEM_NAME,
		CLASS_NAME,
		TIME_NAME,
		CLASS_SKILL_TAB_NAME,
		CLASS_SKILL_NAME,
		NORMAL_SKILL_NAME,
		QUEST_NAME,
		WAY_POINT_NAME,
		DIFFICULTY_NAME,
		ITEM_QUALITY_NAME,
		ITEM_CATAGORY_NAME,
		MERCENARY_TYPE_NAME,
		MERCENARY_NAME_SCOUT,
		MERCENARY_NAME_MERC,
		MERCENARY_NAME_SOR,
		MERCENARY_NAME_BAR,
		MSG_BOX_INFO,
		CHAR_BASIC_INFO_UI,
		CHAR_ITEMS_UI,
		ITEM_SUSPEND_UI,
		FOUNDRY_UI,
		NEW_ITEM_UI,
		OTHER_UI,
		SYSTEM_MENU,
		CHAR_ITEM_POPUP_MENU,
		MENU_PROMPT,
		OTHER_MSG,
	};

	// 数据加载
	bool readLangRes();
	bool readItemRes();
	bool readPropRes();
	bool readNewChar();

	// 内部辅助
	QString getString(UINT index) const;
	UINT sectionSize(LangSection section) const;
	UINT sectionToIndex(LangSection section, UINT index) const;

	// 文件加载工具
	static std::string loadCompressedFile(const QString& path, const char magic[4]);
	static std::string loadRawFile(const QString& path);
};
