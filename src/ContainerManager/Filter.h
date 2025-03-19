#pragma once

namespace ContainerManager
{
    enum FormType : uint32_t
    {
        kNone = 0,

        // Weapons & Ammo
        kDagger     = 1 << 0,
        kSword      = 1 << 1,
        kAxe        = 1 << 2,
        kMace       = 1 << 3,
        kBow        = 1 << 4,
        kGreatsword = 1 << 5,
        kWarAxe     = 1 << 6,
        kWarhammer  = 1 << 7,
        kStaff      = 1 << 8,
        kBolt       = 1 << 9,
        kArrow      = 1 << 10,

        // Apparel
        kHeavy      = 1 << 11,
        kLight      = 1 << 12,
        kShield     = 1 << 13,
        kClothing   = 1 << 14,

        // Books, Tomes, Notes
        kNovice     = 1 << 15,
        kApprentice = 1 << 16,
        kAdept      = 1 << 17,
        kExpert     = 1 << 18,
        kMaster     = 1 << 19,
        kBook       = 1 << 20,
        kNote       = 1 << 21,
        kSkillBook  = 1 << 22,
        kScroll     = 1 << 23,

        // Alchemy & Food
        kPotion     = 1 << 24,
        kPoison     = 1 << 25,
        kFood       = 1 << 26,
        kIngredient = 1 << 27,

        // Misc stuff
        kSoulGem    = 1 << 28,
        kMisc       = 1 << 29,

        // Hack
        kAll        = (1 << 30) - 1,

        // Combined flags

        kMeleeWeapon  = kDagger | kSword | kAxe | kMace | kGreatsword | kWarAxe | kWarhammer,
        kRangedWeapon = kBow | kStaff,
        kAmmo         = kBolt | kArrow,
        kWeapon       = kMeleeWeapon | kRangedWeapon | kAmmo,

        kArmor     = kHeavy | kLight,
        kDefensive = kArmor | kShield,
        kBody      = kArmor | kClothing,
        kApparel   = kDefensive | kClothing,

        kSpellTome = kNovice | kApprentice | kAdept | kExpert | kMaster,
        kMagic     = kSpellTome | kScroll,
        kNonMagic  = kBook | kNote | kSkillBook,
        kWritten   = kMagic | kNonMagic,

        kAlchemy    = kPotion | kPoison,
        kIngestible = kAlchemy | kIngredient | kFood
    };

    inline FormType operator|(FormType lhs, FormType rhs) {
        return static_cast<FormType>(
            static_cast<std::underlying_type_t<FormType>>(lhs) |
            static_cast<std::underlying_type_t<FormType>>(rhs)
            );
    }

    inline FormType& operator|=(FormType& lhs, FormType rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    inline FormType operator&(FormType lhs, FormType rhs) {
        return static_cast<FormType>(
            static_cast<std::underlying_type_t<FormType>>(lhs) &
            static_cast<std::underlying_type_t<FormType>>(rhs)
            );
    }

    inline bool IsFormType(FormType value, FormType flag) {
        return (value & flag) != FormType::kNone;
    }

    enum SimpleFilter : uint32_t
    {
        kNoFilters = 0,
        kEnchanted = 1 << 0,
        kUnenchanted = 1 << 1,
        kFilled = 1 << 2,
		kEmpty = 1 << 3,
        kRefillable = 1 << 4,
		kSingleUse = 1 << 5,
    };

    inline SimpleFilter operator|(SimpleFilter lhs, SimpleFilter rhs) {
        return static_cast<SimpleFilter>(
            static_cast<std::underlying_type_t<SimpleFilter>>(lhs) |
            static_cast<std::underlying_type_t<SimpleFilter>>(rhs)
            );
    }

    inline SimpleFilter& operator|=(SimpleFilter& lhs, SimpleFilter rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    inline SimpleFilter operator&(SimpleFilter lhs, SimpleFilter rhs) {
        return static_cast<SimpleFilter>(
            static_cast<std::underlying_type_t<SimpleFilter>>(lhs) &
            static_cast<std::underlying_type_t<SimpleFilter>>(rhs)
            );
    }

    inline bool IsSimpleFilter(SimpleFilter value, SimpleFilter flag) {
        return (value & flag) != SimpleFilter::kNoFilters;
    }

    struct StoredForm
    {
        int32_t                            formValue;
        FormType                           formType;
        SimpleFilter                       formFilter;
        RE::TESBoundObject*                form;
        std::vector<const RE::BGSKeyword*> formKeywords;
        std::vector<const RE::BGSKeyword*> formEffectKeywords;

        // Ideally const, but GetSpell() for Books violates that.
        StoredForm(RE::TESBoundObject* a_form);

        void PrettyPrint();
    };

	struct Rule
	{
        int          minGoldValue{ -1 };
        int          maxGoldValue{ -1 };
        int          minWarmthValue{ -1 };
        int          maxWarmthValue{ -1 };
        FormType     acceptedFormTypes{ FormType::kAll };
        std::string  ruleName{ "UNDEFINED" };
        SimpleFilter acceptedFilters{ SimpleFilter::kNoFilters };

        std::vector<RE::BGSKeyword*> formKeywords{};
        std::vector<RE::BGSKeyword*> effectKeywords{};
	};

    class RuleBuilder
    {
    public:
        RuleBuilder();
        int Build(); 

		RuleBuilder& WithMinMaxValue(int a_min, int a_max);
		RuleBuilder& WithMinMaxWarmthValue(int a_min, int a_max);
		RuleBuilder& WithName(const std::string& a_name);
		RuleBuilder& WithFormKeywords(std::vector<RE::BGSKeyword*> a_keywords);
		RuleBuilder& WithEffectKeywords(std::vector<RE::BGSKeyword*> a_keywords);
		RuleBuilder& WithFormType(FormType a_type);
		RuleBuilder& WithFilters(SimpleFilter a_filters);

    private:
        Rule m_rule{};
    };
}