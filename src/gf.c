#include "angband.h"
#include "gf.h"

#include <assert.h>

static gf_info_t _gf_tbl[] = {
    { GF_ACID, "Acid", TERM_GREEN, RES_ACID, "ACID" },
    { GF_ELEC, "Lightning", TERM_BLUE, RES_ELEC, "ELEC" },
    { GF_FIRE, "Fire", TERM_RED, RES_FIRE, "FIRE" },
    { GF_COLD, "Cold", TERM_L_WHITE, RES_COLD, "COLD" },
    { GF_POIS, "Poison", TERM_L_GREEN, RES_POIS, "POISON" },
    { GF_LITE, "Light", TERM_YELLOW, RES_LITE, "LITE" },
    { GF_DARK, "Dark", TERM_L_DARK, RES_DARK, "DARK" },
    { GF_CONFUSION, "Confusion", TERM_L_UMBER, RES_CONF, "CONFUSION" },
    { GF_NETHER, "Nether", TERM_L_DARK, RES_NETHER, "NETHER" },
    { GF_NEXUS, "Nexus", TERM_VIOLET, RES_NEXUS, "NEXUS" },
    { GF_SOUND, "Sound", TERM_ORANGE, RES_SOUND, "SOUND" },
    { GF_SHARDS, "Shards", TERM_L_UMBER, RES_SHARDS, "SHARDS" },
    { GF_CHAOS, "Chaos", TERM_VIOLET, RES_CHAOS, "CHAOS" },
    { GF_DISENCHANT, "Disenchantment", TERM_VIOLET, RES_DISEN, "DISENCHANT" },
    { GF_TIME, "Time", TERM_L_BLUE, RES_TIME, "TIME" },
    { GF_MANA, "Mana", TERM_L_BLUE, RES_INVALID, "MANA" },
    { GF_GRAVITY, "Gravity", TERM_L_UMBER, RES_INVALID, "GRAVITY" },
    { GF_INERT, "Inertia", TERM_L_UMBER, RES_INVALID, "INERTIA" },
    { GF_PLASMA, "Plasma", TERM_L_RED, RES_INVALID, "PLASMA" },
    { GF_FORCE, "Force", TERM_L_BLUE, RES_INVALID, "FORCE" },
    { GF_NUKE, "Toxic Waste", TERM_L_GREEN, RES_POIS, "NUKE" },
    { GF_DISINTEGRATE, "Disintegration", TERM_L_DARK, RES_INVALID, "DISINTEGRATE" },
    { GF_STORM, "Storm Winds", TERM_BLUE, RES_INVALID, "STORM" },
    { GF_HOLY_FIRE, "Holy Fire", TERM_YELLOW, RES_INVALID, "HOLY_FIRE" },
    { GF_HELL_FIRE, "Hell Fire", TERM_L_DARK, RES_INVALID, "HELL_FIRE" },
    { GF_ICE, "Ice", TERM_L_WHITE, RES_COLD, "ICE" },
    { GF_WATER, "Water", TERM_L_BLUE, RES_INVALID, "WATER" },
    { GF_ROCKET, "Rocket", TERM_RED, RES_SHARDS, "ROCKET" },
    { GF_DRAIN_MANA, "Drain Mana", TERM_L_BLUE, RES_INVALID, "DRAIN_MANA" },
    { GF_OLD_SLEEP, "Sleep", TERM_BLUE, RES_INVALID, "SLEEP" },
    { GF_OLD_SLOW, "Slow", TERM_L_UMBER, RES_INVALID, "SLOW" },
    { GF_TURN_ALL, "Terrify", TERM_RED, RES_INVALID, "FEAR" },
    { GF_STUN, "Stun", TERM_L_BLUE, RES_INVALID, "STUN" },
    { GF_AMNESIA, "Amnesia", TERM_L_DARK, RES_INVALID, "AMNESIA" },
    { GF_PARALYSIS, "Paralyze", TERM_VIOLET, RES_INVALID, "PARALYZE" },
    { GF_MIND_BLAST, "Mind Blast", TERM_L_BLUE, RES_INVALID, "MIND_BLAST" },
    { GF_BRAIN_SMASH, "Brain Smash", TERM_L_BLUE, RES_INVALID, "BRAIN_SMASH" },
    { GF_CAUSE_1, "Cause Light Wounds", TERM_RED, RES_INVALID, "CAUSE_1" },
    { GF_CAUSE_2, "Cause Serious Wounds", TERM_RED, RES_INVALID, "CAUSE_2" },
    { GF_CAUSE_3, "Cause Critical Wounds", TERM_RED, RES_INVALID, "CAUSE_3" },
    { GF_CAUSE_4, "Cause Mortal Wounds", TERM_RED, RES_INVALID, "CAUSE_4" },
    { GF_HAND_DOOM, "Hand of Doom", TERM_VIOLET, RES_INVALID, "HAND_DOOM" },
    { GF_ELDRITCH, "Eldritch Horror", TERM_VIOLET, RES_INVALID, "ELDRITCH" },
    { GF_MISSILE, "Damage", TERM_L_UMBER, RES_INVALID, "DAM" },
    { GF_OLD_POLY, "Polymorph", TERM_RED, RES_INVALID, "POLYMORPH" },
    {0}
};

gf_info_ptr gf_parse_name(cptr token)
{
    int i;
    for (i = 0; ; i++)
    {
        gf_info_ptr info = &_gf_tbl[i];
        if (!info->parse) return NULL;
        if (strcmp(info->parse, token) == 0) return info;
    }
}

gf_info_ptr gf_lookup(int id)
{
    int i;
    for (i = 0; ; i++)
    {
        gf_info_ptr info = &_gf_tbl[i];
        if (!info->parse) return NULL;
        if (info->id == id) return info;
    }
}

#define HURT_CHANCE 16

/* Stop using project() for direct damage effects! For one thing, monster auras
 * and attacks are not spells, so there is no learning/absorption/range damage
 * reduction, etc. For another, trying to get project() to actually work is extremely
 * obtuse ... You need to specify a half dozen flags to get it to actually work.
 * (The unexpected fact is that PROJECT_PLAYER still requires PROJECT_KILL to work).
 *
 * Cf monster melee, monster auras, monsters battling other monsters. There is 
 * absolutely no need for projection ... it is stupid! All we need is a way to 
 * directly damage player/monster with a specific attack type.
 * BTW, I have no idea what GF_ stands for ... 

   OLD: project(m_idx, 0, py, px, dam, aura->effect,
            PROJECT_KILL | PROJECT_PLAYER | PROJECT_HIDE |
            PROJECT_AIMED | PROJECT_JUMP | PROJECT_AURA, -1);  <== 13 parameters 
   NEW: gf_damage_p(m_idx, aura->effect, dam, GF_DAMAGE_AURA); <==  4 parameters
 */
int mon_spell_hack; /* Blue-mage: project_p will handle this ... everyone else can ignore */
static int _rlev(int m_idx)
{
    if (m_idx > 0)
    {
        mon_ptr mon = &m_list[m_idx];
        mon_race_ptr race = &r_info[mon->/*ap_*/r_idx]; /* XXX */
        return race->level;
    }
    return 0;
}
static int _plr_save_odds(int m_idx, int boost)
{
    int rlev = _rlev(m_idx);
    int roll = 100 + rlev/2 + boost;
    int sav = duelist_skill_sav(m_idx);
    int odds = sav * 100 / roll;
    return odds;
}
static bool _plr_save(int m_idx, int boost)
{
    int odds = _plr_save_odds(m_idx, boost);
    return randint0(100) < odds;
}
static int _align_dam_pct(int align)
{
    static point_t tbl[6] = { {-150, 200}, {-50, 150}, {-10, 125},
                              {10, 80}, {50, 66}, {150, 50} };

    return interpolate(align, tbl, 6);
}
static int _holy_dam(int dam)
{
    return dam * _align_dam_pct(p_ptr->align) / 100;
}
static int _hell_dam(int dam)
{
    return dam * _align_dam_pct(-p_ptr->align) / 100;
}
int gf_damage_p(int who, int type, int dam, int flags)
{
    int          result = 0;
    mon_ptr      m_ptr = NULL;
    mon_race_ptr r_ptr = NULL;
    int          rlev = 1;
    char         m_name[80];
    bool         touch = BOOL(flags & (GF_DAMAGE_AURA | GF_DAMAGE_ATTACK));
    bool         fuzzy = BOOL(p_ptr->blind);

    if (who > 0)
    {
        m_ptr = &m_list[who];
        r_ptr = &r_info[m_ptr->r_idx];
        rlev = MIN(1, r_ptr->level);

        monster_desc(m_name, m_ptr, 0);
    }
    else
    {
        switch (who)
        {
        case PROJECT_WHO_UNCTRL_POWER:
            strcpy(m_name, "uncontrollable power storm");
            break;

        case PROJECT_WHO_GLASS_SHARDS:
            strcpy(m_name, "shards of glass");
            break;

        default:
            strcpy(m_name, "a trap");
            break;
        }
    }

    /* Analyze the damage */
    switch (type)
    {
    case GF_ACID:
        if (touch) msg_print("You are <color:G>dissolved</color>!");
        else if (fuzzy) msg_print("You are hit by acid!");
        result = acid_dam(dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_ACID);
        break;
    case GF_FIRE:
        if (touch) msg_print("You are <color:r>burned</color>!");
        else if (fuzzy) msg_print("You are hit by fire!");
        result = fire_dam(dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_FIRE);
        break;
    case GF_COLD:
        if (touch) msg_print("You are <color:W>frozen</color>!");
        else if (fuzzy) msg_print("You are hit by cold!");
        result = cold_dam(dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_COLD);
        break;
    case GF_ELEC:
        if (touch) msg_print("You are <color:b>shocked</color>!");
        else if (fuzzy) msg_print("You are hit by lightning!");
        result = elec_dam(dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_ELEC);
        break;
    case GF_POIS:
        if (touch) msg_print("You are <color:G>poisoned</color>!");
        else if (fuzzy) msg_print("You are hit by poison!");
        if (CHECK_MULTISHADOW()) break;
        dam = res_calc_dam(RES_POIS, dam);
        /* Moving damage from immediate to delayed can't simply leave the
         * value unchanged, else this is a monster nerf! We can scale everything
         * in r_info and BR_POIS, but that is tedious and I'm unsure what a good
         * scale factor is without some playtesting. cf GF_NUKE below. */
        dam *= 2;
        set_poisoned(p_ptr->poisoned + dam, FALSE);
        if (!res_save_default(RES_POIS) && one_in_(HURT_CHANCE) && !CHECK_MULTISHADOW())
            do_dec_stat(A_CON);
        update_smart_learn(who, RES_POIS);
        break;
    case GF_NUKE:
        if (touch) msg_print("You are <color:G>irradiated</color>!");
        else if (fuzzy) msg_print("You are hit by radiation!");
        if (CHECK_MULTISHADOW()) break;
        dam = res_calc_dam(RES_POIS, dam);
        dam *= 2;
        set_poisoned(p_ptr->poisoned + dam, FALSE);
        if (!res_save_default(RES_POIS))
        {
            if (one_in_(5))
            {
                msg_print("You undergo a freakish metamorphosis!");
                if (one_in_(4))
                    do_poly_self();
                else
                    mutate_player();
            }
            if (!touch) inven_damage(set_acid_destroy, 2, RES_POIS);
        }
        update_smart_learn(who, RES_POIS);
        break;
    case GF_MISSILE:
    case GF_BLOOD:  /* Monsters can't do this ... */
        if (fuzzy) msg_print("You are hit by something!");
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_HOLY_FIRE:
        if (touch) msg_format("You are <color:y>%s</color>!", p_ptr->align < -10 ? "*burned*" : "burned");
        else if (fuzzy) msg_print("You are hit by something!");
        dam = _holy_dam(dam);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_HELL_FIRE:
        if (touch) msg_format("You are <color:D>%s</color>!", p_ptr->align > 10 ? "*burned*" : "burned");
        else if (fuzzy) msg_print("You are hit by something!");
        dam = _hell_dam(dam);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_ARROW:
        if (fuzzy) msg_print("You are hit by something sharp!");
        else if (equip_find_art(ART_ZANTETSU))
        {
            msg_print("You cut down the arrow!");
            break;
        }
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_PLASMA:
        if (touch) msg_print("You are <color:R>burned</color>!");
        else if (fuzzy) msg_print("You are hit by something *HOT*!");
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
        {
            int k = (randint1((dam > 40) ? 35 : (dam * 3 / 4 + 5)));
            set_stun(p_ptr->stun + k, FALSE);
        }

        if (!touch) inven_damage(set_acid_destroy, 3, RES_FIRE);
        break;
    case GF_NETHER:
        if (touch) msg_print("You are <color:D>drained</color>!");
        else if (fuzzy) msg_print("You are hit by nether forces!");
        dam = res_calc_dam(RES_NETHER, dam);
        if (!res_save_default(RES_NETHER) && !CHECK_MULTISHADOW())
            drain_exp(200 + (p_ptr->exp / 100), 200 + (p_ptr->exp / 1000), 75);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_NETHER);
        break;
    case GF_WATER:
    case GF_WATER2:
        if (fuzzy) msg_print("You are hit by something wet!");
        if (!CHECK_MULTISHADOW())
        {
            if (!res_save_default(RES_SOUND))
                set_stun(p_ptr->stun + randint1(40), FALSE);
            if (!res_save_default(RES_CONF))
                set_confused(p_ptr->confused + randint1(5) + 5, FALSE);
            inven_damage(set_cold_destroy, 3, RES_SOUND);
        }
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_CHAOS:
        if (touch) msg_print("You are <color:v>unmade</color>!");
        else if (fuzzy) msg_print("You are hit by a wave of anarchy!");
        dam = res_calc_dam(RES_CHAOS, dam);
        if (!CHECK_MULTISHADOW())
        {
            if (!res_save_default(RES_CONF))
                set_confused(p_ptr->confused + randint0(20) + 10, FALSE);
            if (!res_save_default(RES_CHAOS))
            {
                int count = mut_count(mut_unlocked_pred);
                if (prace_is_(RACE_BEASTMAN)) count = 0;
                if (one_in_(3 + count*count))
                {
                    msg_print("Your body is twisted by chaos!");
                    mut_gain_random(NULL);
                }
                if (p_ptr->pclass == CLASS_WILD_TALENT && one_in_(7))
                    wild_talent_scramble();

                set_image(p_ptr->image + randint1(10), FALSE);
            }
            if (!res_save_default(RES_NETHER) && !res_save_default(RES_CHAOS))
                drain_exp(5000 + (p_ptr->exp / 100), 500 + (p_ptr->exp / 1000), 75);

            if (!touch)
            {
                inven_damage(set_elec_destroy, 2, RES_CHAOS);
                inven_damage(set_fire_destroy, 2, RES_CHAOS);
            }
        }
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_CHAOS);
        break;
    case GF_ROCK:
        if (fuzzy) msg_print("You are hit by something solid!");
        if (one_in_(2))
        {
            if (!res_save_default(RES_SHARDS) && !CHECK_MULTISHADOW())
                set_cut(p_ptr->cut + dam/2, FALSE);
            inven_damage(set_cold_destroy, 2, RES_SHARDS);
        }
        else
        {
            if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
            {
                int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
                set_stun(p_ptr->stun + k, FALSE);
            }
            inven_damage(set_cold_destroy, 2, RES_SOUND);
        }
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_SHARDS:
        if (touch) msg_print("You are <color:U>shredded</color>!");
        else if (fuzzy) msg_print("You are hit by something sharp!");
        dam = res_calc_dam(RES_SHARDS, dam);
        if (!res_save_default(RES_SHARDS) && !CHECK_MULTISHADOW())
            set_cut(p_ptr->cut + dam, FALSE);
        if (!touch) inven_damage(set_cold_destroy, 2, RES_SHARDS);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_SHARDS);
        break;
    case GF_SOUND:
        if (!touch && fuzzy) msg_print("You are hit by a loud noise!");
        /*if (touch) ... */
        dam = res_calc_dam(RES_SOUND, dam);
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
        {
            int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
            set_stun(p_ptr->stun + k, FALSE);
        }
        if (!touch) inven_damage(set_cold_destroy, 2, RES_SOUND);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_SOUND);
        break;
    case GF_CONFUSION:
        if (!touch && fuzzy) msg_print("You are hit by something puzzling!");
        /*if (touch) ... */
        dam = res_calc_dam(RES_CONF, dam);
        if (!res_save_default(RES_CONF) && !CHECK_MULTISHADOW())
            set_confused(p_ptr->confused + randint1(20) + 10, FALSE);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_CONF);
        break;
    case GF_DISENCHANT:
        if (touch) msg_print("You are <color:v>disenchanted</color>!");
        else if (fuzzy) msg_print("You are hit by something static!");
        dam = res_calc_dam(RES_DISEN, dam);
        if (!(flags & GF_DAMAGE_SPELL) && !one_in_(5) && !CHECK_MULTISHADOW())
        {
            if (!res_save_default(RES_DISEN) || one_in_(5))
                disenchant_player();
        }
        else if (!res_save(RES_DISEN, 31) && !CHECK_MULTISHADOW())
            apply_disenchant(0);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_DISEN);
        break;
    case GF_NEXUS:
        if (touch) msg_print("You are <color:v>scrambled</color>!");
        else if (fuzzy) msg_print("You are hit by something strange!");
        dam = res_calc_dam(RES_NEXUS, dam);
        if (!res_save_default(RES_NEXUS) && !CHECK_MULTISHADOW())
            apply_nexus(m_ptr);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_NEXUS);
        break;
    case GF_FORCE:
        if (fuzzy) msg_print("You are hit by kinetic force!");
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
            set_stun(p_ptr->stun + randint1(20), FALSE);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_ROCKET:
        if (fuzzy) msg_print("There is an explosion!");
        dam = res_calc_dam(RES_SHARDS, dam);
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
            set_stun(p_ptr->stun + randint1(20), FALSE);
        if (!res_save_default(RES_SHARDS) && !CHECK_MULTISHADOW())
            set_cut(p_ptr->cut + (dam / 2), FALSE);
        inven_damage(set_cold_destroy, 3, RES_SHARDS);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_SHARDS);
        break;
    case GF_INERT:
        if (!touch && fuzzy) msg_print("You are hit by something slow!");
        /*if (touch) ... */
        if (!CHECK_MULTISHADOW())
            set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_LITE:
        if (touch) msg_print("You are <color:y>dazzled</color>!");
        else if (fuzzy) msg_print("You are hit by something!");
        dam = res_calc_dam(RES_LITE, dam);
        if (!p_ptr->blind && !res_save_default(RES_LITE) && !res_save_default(RES_BLIND) && !CHECK_MULTISHADOW())
            set_blind(p_ptr->blind + randint1(5) + 2, FALSE);

        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        if (prace_is_(RACE_MON_VAMPIRE))
            vampire_take_light_damage(dam);

        if (IS_WRAITH() && !CHECK_MULTISHADOW() && !touch)
        {
            p_ptr->wraith_form = 0;
            wild_reset_counter(WILD_WRAITH);
            msg_print("The light forces you out of your incorporeal shadow form.");
            p_ptr->redraw |= PR_MAP;
            p_ptr->update |= (PU_MONSTERS);
            p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
            p_ptr->redraw |= (PR_STATUS);
        }
        update_smart_learn(who, RES_LITE);
        break;
    case GF_DARK:
        if (touch) msg_print("You are <color:D>benighted</color>!");
        else if (fuzzy) msg_print("You are hit by something!");
        dam = res_calc_dam(RES_DARK, dam);
        if (!p_ptr->blind && !res_save_default(RES_DARK) && !res_save_default(RES_BLIND) && !CHECK_MULTISHADOW())
            set_blind(p_ptr->blind + randint1(5) + 2, FALSE);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        if (prace_is_(RACE_MON_VAMPIRE))
            vampire_take_dark_damage(dam);
        update_smart_learn(who, RES_DARK);
        break;
    case GF_ELDRITCH:
        if (touch && m_ptr)
            sanity_blast(m_ptr, FALSE);
        break;
    case GF_STUN:
        if (touch) set_stun(p_ptr->stun + dam, FALSE);
        break;
    case GF_AMNESIA:
        if (_plr_save(who, 0))
        {
            if (!touch) msg_print("You resist the effects!");
        }
        else if (lose_all_info())
        {
            msg_print("Your memories fade away.");
        }
        break;
    case GF_TIME:
        if (touch) msg_print("You are <color:B>chronosmashed</color>!");
        else if (fuzzy) msg_print("You are hit by a blast from the past!");
        if (!res_save_default(RES_TIME) && !CHECK_MULTISHADOW())
        {
            int k;
            cptr act;
            switch (randint1(10))
            {
            case 1: case 2: case 3: case 4: case 5:
                if (p_ptr->prace == RACE_ANDROID) break;
                msg_print("You feel life has clocked back.");
                lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
                break;

            case 6: case 7: case 8: case 9:
                switch (randint1(6))
                {
                    case 1: k = A_STR; act = "strong"; break;
                    case 2: k = A_INT; act = "bright"; break;
                    case 3: k = A_WIS; act = "wise"; break;
                    case 4: k = A_DEX; act = "agile"; break;
                    case 5: k = A_CON; act = "hale"; break;
                    case 6: k = A_CHR; act = "confident"; break;
                }
                msg_format("You're not as %s as you used to be...", act);
                p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
                if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
                p_ptr->update |= (PU_BONUS);
                break;
            case 10:
                msg_print("You're not as powerful as you used to be...");
                for (k = 0; k < 6; k++)
                {
                    p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 7) / 8;
                    if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
                }
                p_ptr->update |= (PU_BONUS);
                break;
            }
        }
        dam = res_calc_dam(RES_TIME, dam);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        update_smart_learn(who, RES_TIME);
        break;
    case GF_STORM:
        msg_print("You are hit by gale force winds!");
        if (!CHECK_MULTISHADOW())
        {
            teleport_player(5, TELEPORT_PASSIVE);
            if (!p_ptr->levitation)
                set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
            if (!(res_save_default(RES_SOUND) || p_ptr->levitation))
            {
                int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
                set_stun(p_ptr->stun + k, FALSE);
            }
        }
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_GRAVITY:
        if (!touch && fuzzy) msg_print("You are hit by something heavy!");
        /*if (touch) ... */
        msg_print("Gravity warps around you.");
        if (!CHECK_MULTISHADOW())
        {
            teleport_player(5, TELEPORT_PASSIVE);
            if (!p_ptr->levitation)
                set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
            if (!(res_save_default(RES_SOUND) || p_ptr->levitation))
            {
                int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
                set_stun(p_ptr->stun + k, FALSE);
            }
        }
        if (p_ptr->levitation)
        {
            dam = (dam * 2) / 3;
        }
        inven_damage(set_cold_destroy, 2, RES_SOUND);
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_DISINTEGRATE:
        if (fuzzy) msg_print("You are hit by pure energy!");
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_OLD_HEAL:
        if (fuzzy) msg_print("You are hit by something invigorating!");

        hp_player(dam);
        break;
    case GF_OLD_SPEED:
        if (fuzzy) msg_print("You are hit by something!");

        set_fast(p_ptr->fast + randint1(5), FALSE);
        break;
    case GF_OLD_SLOW:
        if (fuzzy) msg_print("You are hit by something slow!");
        set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
        break;
    case GF_OLD_SLEEP:
        if (p_ptr->free_act)
        {
            equip_learn_flag(OF_FREE_ACT);
            break;
        }
        if (fuzzy) msg_print("You fall asleep!");
        if (ironman_nightmare)
        {
            msg_print("A horrible vision enters your mind.");

            /* Pick a nightmare */
            get_mon_num_prep(get_nightmare, NULL);

            /* Have some nightmares */
            have_nightmare(get_mon_num(MAX_DEPTH));

            /* Remove the monster restriction */
            get_mon_num_prep(NULL, NULL);
        }
        set_paralyzed(dam, FALSE);
        break;
    case GF_TURN_ALL:
        if (fuzzy) msg_print("Your will is shaken!");
        fear_scare_p(m_ptr);
        break;
    case GF_PARALYSIS:
        if (p_ptr->free_act)
        {
            msg_print("You are unaffected!");
            equip_learn_flag(OF_FREE_ACT);
        }
        else if (_plr_save(who, dam))
            msg_print("You resist the effects!");
        else
            set_paralyzed(randint1(3), FALSE);
        update_smart_learn(who, SM_FREE_ACTION);
        break;
    case GF_MANA:
    case GF_SEEKER:
    case GF_SUPER_RAY:
        if (fuzzy) msg_print("You are hit by an touch of magic!");
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_PSY_SPEAR:
        if (fuzzy) msg_print("You are hit by an energy!");
        result = take_hit(DAMAGE_FORCE, dam, m_name, mon_spell_hack);
        break;
    case GF_METEOR:
        if (fuzzy) msg_print("Something falls from the sky on you!");
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        inven_damage(set_fire_destroy, 2, RES_FIRE);
        inven_damage(set_cold_destroy, 2, RES_SHARDS);
        break;
    case GF_ICE:
        if (touch) msg_print("You are <color:W>frozen</color>!");
        else if (fuzzy) msg_print("You are hit by something sharp and cold!");
        result = cold_dam(dam, m_name, mon_spell_hack);
        if (!CHECK_MULTISHADOW())
        {
            if (!res_save_default(RES_SHARDS))
                set_cut(p_ptr->cut + (touch ? damroll(3, 5) : damroll(5, 8)), FALSE);
            if (!res_save_default(RES_SOUND))
                set_stun(p_ptr->stun + (touch ? randint1(7) : randint1(15)), FALSE);
            inven_damage(set_cold_destroy, 3, RES_COLD);
        }
        update_smart_learn(who, RES_COLD);
        break;
    case GF_DEATH_RAY:
        if (fuzzy) msg_print("You are hit by something extremely cold!");
        if (!(get_race()->flags & RACE_IS_NONLIVING))
            result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_DRAIN_MANA:
        if (CHECK_MULTISHADOW())
        {
            if (!touch) msg_print("The attack hits Shadow, you are unharmed!");
        }
        else if (psion_mental_fortress())
        {
            if (!touch) msg_print("Your mental fortress is impenetrable!");
        }
        else if ( prace_is_(RACE_DEMIGOD)
                && p_ptr->psubrace == DEMIGOD_HERA
                && randint1(100) > r_ptr->level - 2*(p_ptr->stat_ind[A_WIS] + 3))
        {
            if (!touch) msg_print("You keep your wits about you!");
        }
        else if (p_ptr->csp)
        {
            if (who > 0) msg_format("%^s draws psychic energy from you!", m_name);
            else msg_print("Your psychic energy is drawn!");
            if (dam >= p_ptr->csp)
            {
                dam = p_ptr->csp;
                p_ptr->csp = 0;
                p_ptr->csp_frac = 0;
            }
            else
                p_ptr->csp -= dam;

            if (!touch) learn_spell(mon_spell_hack);

            p_ptr->redraw |= (PR_MANA);
            p_ptr->window |= (PW_SPELL);

            if (who > 0 && m_ptr->hp < m_ptr->maxhp)
            {
                m_ptr->hp += (6 * dam);
                if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
                check_mon_health_redraw(who);
                if (m_ptr->ml)
                    msg_format("%^s appears healthier.", m_name);
            }
        }
        break;
    case GF_MIND_BLAST:
        if (_plr_save(who, dam/5) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
                learn_spell(mon_spell_hack);
            }
        }
        else if (!CHECK_MULTISHADOW())
        {
            msg_print("Your mind is blasted by psionic energy.");

            if (!res_save_default(RES_CONF))
                set_confused(p_ptr->confused + randint0(4) + 4, FALSE);

            if (!res_save_default(RES_CHAOS) && one_in_(3))
                set_image(p_ptr->image + randint0(25) + 15, FALSE);

            p_ptr->csp -= touch ? 10 : 50;
            if (p_ptr->csp < 0)
            {
                p_ptr->csp = 0;
                p_ptr->csp_frac = 0;
            }
            p_ptr->redraw |= PR_MANA;
        }
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_BRAIN_SMASH:
        if (_plr_save(who, dam/3) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
                learn_spell(mon_spell_hack);
            }
        }
        else
        {
            if ( prace_is_(RACE_DEMIGOD)
              && p_ptr->psubrace == DEMIGOD_HERA
              && randint1(100) > r_ptr->level - 2*(p_ptr->stat_ind[A_WIS] + 3))
            {
                if (!touch) msg_print("You keep your wits about you!");
            }
            else if (!CHECK_MULTISHADOW())
            {
                msg_print("Your mind is blasted by psionic energy.");
                if (touch)
                    p_ptr->csp -= 10;
                else
                    p_ptr->csp -= 100;
                if (p_ptr->csp < 0)
                {
                    p_ptr->csp = 0;
                    p_ptr->csp_frac = 0;
                }
                p_ptr->redraw |= PR_MANA;
            }
            result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
            if (!CHECK_MULTISHADOW())
            {
                if (!res_save_default(RES_BLIND))
                    set_blind(p_ptr->blind + 8 + randint0(8), FALSE);
                if (!res_save_default(RES_CONF))
                    set_confused(p_ptr->confused + randint0(4) + 4, FALSE);
                if (!p_ptr->free_act)
                    set_paralyzed(randint1(4), FALSE);
                else
                    equip_learn_flag(OF_FREE_ACT);

                set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
                set_stun(p_ptr->stun + MIN(50, dam/6 + randint1(dam/6)), FALSE);

                while (!_plr_save(who, 0))
                    do_dec_stat(A_INT);
                while (!_plr_save(who, 0))
                    do_dec_stat(A_WIS);

                if (!res_save_default(RES_CHAOS))
                    set_image(p_ptr->image + randint0(25) + 15, FALSE);
            }
        }
        break;
    case GF_TELEKINESIS:
        if (!CHECK_MULTISHADOW())
        {
            if (one_in_(4))
                teleport_player(5, TELEPORT_PASSIVE);
            if (!_plr_save(who, dam/5))
                set_stun(p_ptr->stun + MIN(25, dam/6 + randint1(dam/6)), FALSE);
        }
        result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        break;
    case GF_CAUSE_1:
        if (_plr_save(who, dam/5) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
                learn_spell(mon_spell_hack);
            }
        }
        else
        {
            if (!CHECK_MULTISHADOW()) curse_equipment(15, 0);
            result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        }
        break;
    case GF_CAUSE_2:
        if (_plr_save(who, dam/5) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
                learn_spell(mon_spell_hack);
            }
        }
        else
        {
            if (!CHECK_MULTISHADOW()) curse_equipment(25, MIN(rlev / 2 - 15, 5));
            result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        }
        break;
    case GF_CAUSE_3:
        if (_plr_save(who, dam/5) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
                learn_spell(mon_spell_hack);
            }
        }
        else
        {
            if (!CHECK_MULTISHADOW()) curse_equipment(33, MIN(rlev / 2 - 15, 15));
            result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
        }
        break;
    case GF_CAUSE_4:
        if (_plr_save(who, dam/5) && m_ptr->r_idx != MON_KENSHIROU && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
                learn_spell(mon_spell_hack);
            }
        }
        else
        {
            result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);
            if (!CHECK_MULTISHADOW()) set_cut(p_ptr->cut + damroll(10, 10), FALSE);
        }
        break;
    case GF_HAND_DOOM:
        if (_plr_save(who, 0) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
                learn_spell(mon_spell_hack);
            }
        }
        else
        {
            if (!CHECK_MULTISHADOW())
            {
                msg_print("You feel your life fade away!");
                curse_equipment(40, 20);
            }

            result = take_hit(DAMAGE_ATTACK, dam, m_name, mon_spell_hack);

            if (p_ptr->chp < 1) p_ptr->chp = 1; /* Paranoia */
        }
        break;
    case GF_OLD_POLY:
        if ( prace_is_(RACE_ANDROID)
          || p_ptr->pclass == CLASS_MONSTER
          || p_ptr->prace == RACE_DOPPELGANGER
          || mut_present(MUT_DRACONIAN_METAMORPHOSIS) )
        {
            if (flags & GF_DAMAGE_SPELL)
                msg_print("You are unaffected!");
        }
        else if (_plr_save(who, 0))
        {
            if (flags & GF_DAMAGE_SPELL)
                msg_print("You resist the effects!");
        }
        else
        {
            int which;
            switch(randint1(5))
            {
            case 1:
                if (p_ptr->prace != RACE_SNOTLING)
                {
                    which = RACE_SNOTLING;
                    break;
                }
            case 2:
                if (p_ptr->prace != RACE_YEEK)
                {
                    which = RACE_YEEK;
                    break;
                }
            case 3:
                which = MIMIC_SMALL_KOBOLD;
                break;
            case 4:
                which = MIMIC_MANGY_LEPER;
                break;
            default:
                for (;;)
                {
                    which = randint0(MAX_RACES);
                    if ( which != RACE_HUMAN
                      && which != RACE_DEMIGOD
                      && which != RACE_DRACONIAN
                      && which != RACE_ANDROID
                      && p_ptr->prace != which
                      && !(get_race_aux(which, 0)->flags & RACE_IS_MONSTER) )
                    {
                        break;
                    }
                }
            }
            set_mimic(50 + randint1(50), which, FALSE);
        }
        break;
    case GF_ATTACK:
        make_attack_normal(who);
        break;
    }
    return result;
}
int gf_distance_hack = 1;
static int _gf_distance_mod(int n)
{
    if (!gf_distance_hack) return n;
    return (n + gf_distance_hack) / (gf_distance_hack + 1);
}
static bool _stun_save(int rlev, int dam)
{
    return randint1((1 + rlev/12)*rlev) > dam;
}
static int _stun_amount(int dam)
{
    static point_t tbl[4] = { {1, 1}, {10, 10}, {100, 25}, {500, 50} };
    return interpolate(dam, tbl, 4);
}

#define _BABBLE_HACK() \
            if (r_ptr->flagsr & RFR_RES_ALL) \
            { \
                note = " is immune."; \
                dam = 0; \
                mon_lore_r(m_ptr, RFR_RES_ALL); \
                break; \
            }

bool gf_damage_m(int who, point_t where, int type, int dam, int flags)
{
    int tmp;

    cave_type *c_ptr = &cave[where.y][where.x];

    monster_type *m_ptr = &m_list[c_ptr->m_idx];
    monster_type *caster_ptr = (who > 0) ? &m_list[who] : NULL;

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    char killer[80];

    /* Is the monster "seen"? */
    bool seen = m_ptr->ml;
    bool seen_msg = seen;

    bool slept = BOOL(MON_CSLEEP(m_ptr));

    /* Were the effects "obvious" (if seen)? */
    bool obvious = FALSE;

    /* Can the player know about this effect? */
    bool known = ((m_ptr->cdis <= MAX_SIGHT) || p_ptr->inside_battle);

    /* Were the effects "irrelevant"? */
    bool skipped = FALSE;

    /* Gets the monster angry at the source of the effect? */
    bool get_angry = FALSE;

    /* Polymorph setting (true or false) */
    bool do_poly = FALSE;

    /* Teleport setting (max distance) */
    int do_dist = 0;

    /* Confusion setting (amount to confuse) */
    int do_conf = 0;

    /* Stunning setting (amount to stun) */
    int do_stun = 0;

    /* Sleep amount (amount to sleep) */
    int do_sleep = 0;
    int do_paralyzed = 0;

    /* Fear amount (amount to fear) */
    int do_fear = 0;

    /* Time amount (amount to time) */
    int do_time = 0;

    bool heal_leper = FALSE;

    /* Hold the monster name */
    char m_name[MAX_NLEN];
    char m_name_object[MAX_NLEN];

    char m_poss[80];

    int photo = 0;

    /* Assume no note */
    cptr note = NULL;

    /* Assume a default death */
    cptr note_dies = extract_note_dies(real_r_ptr(m_ptr));

    int ty = m_ptr->fy;
    int tx = m_ptr->fx;

    int caster_lev = (who > 0) ? r_info[caster_ptr->r_idx].level : spell_power(p_ptr->lev * 2);

    bool who_is_pet = FALSE;
    if (who > 0 && is_pet(&m_list[who]))
        who_is_pet = TRUE;

    /* Nobody here */
    if (!c_ptr->m_idx) return (FALSE);

    /* Never affect projector */
    if (who && (c_ptr->m_idx == who)) return (FALSE);

    if (c_ptr->m_idx == p_ptr->riding && who == PROJECT_WHO_PLAYER)
    {
        switch (type)
        {
        case GF_OLD_HEAL:
        case GF_OLD_SPEED:
        case GF_STAR_HEAL:
        case GF_CRUSADE:
        case GF_UNHOLY_WORD:
            break;
        default:
            return FALSE;
        }
    }

    if (sukekaku && ((m_ptr->r_idx == MON_SUKE) || (m_ptr->r_idx == MON_KAKU))) return FALSE;

    /* Don't affect already death monsters */
    /* Prevents problems with chain reactions of exploding monsters */
    if (m_ptr->hp < 0) return (FALSE);

    /* Get the monster name (BEFORE polymorphing) */
    if (flags & GF_DAMAGE_SPELL)
    {
        monster_desc(m_name, m_ptr, 0);
        monster_desc(m_name_object, m_ptr, 0);
    }
    else
    {
        monster_desc(m_name, m_ptr, MD_PRON_VISIBLE);
        monster_desc(m_name_object, m_ptr, MD_PRON_VISIBLE | MD_OBJECTIVE);
    }
    /* Get the monster possessive ("his"/"her"/"its") */
    monster_desc(m_poss, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE);


    if (p_ptr->riding && (c_ptr->m_idx == p_ptr->riding)) disturb(1, 0);

    /* Analyze the damage type */
    switch (type)
    {
    case GF_MISSILE:
    case GF_BLOOD:
    case GF_ELDRITCH:
    case GF_ELDRITCH_DRAIN:  /* Lazy ... I'll give back hp later */
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_MANA_CLASH:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (!r_ptr->freq_spell)
        {
            note = " is immune.";
            dam = 0;
        }
        else /* 900 max dam coming in ... ~600 max dam going out */
            dam = dam * MIN(66, r_ptr->freq_spell) / 100;
        break;
    case GF_ACID:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_IM_ACID)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_IM_ACID);
        }
        else if (r_ptr->flagsr & RFR_RES_ACID)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(m_ptr, RFR_RES_ACID);
        }
        break;
    case GF_ELEC:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_IM_ELEC)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_IM_ELEC);
        }
        else if (r_ptr->flagsr & RFR_RES_ELEC)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(m_ptr, RFR_RES_ELEC);
        }
        break;
    case GF_FIRE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_IM_FIRE)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_IM_FIRE);
        }
        else if (r_ptr->flagsr & RFR_RES_FIRE)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(m_ptr, RFR_RES_FIRE);
        }
        else if (r_ptr->flags3 & RF3_HURT_FIRE)
        {
            note = " is hit hard.";
            dam *= 2;
            mon_lore_3(m_ptr, RF3_HURT_FIRE);
        }
        break;
    case GF_COLD:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_IM_COLD)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_IM_COLD);
        }
        else if (r_ptr->flagsr & RFR_RES_COLD)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(m_ptr, RFR_RES_COLD);
        }
        else if (r_ptr->flags3 & RF3_HURT_COLD)
        {
            note = " is hit hard.";
            dam *= 2;
            mon_lore_3(m_ptr, RF3_HURT_COLD);
        }
        break;
    case GF_POIS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_IM_POIS)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_IM_POIS);
        }
        else if (r_ptr->flagsr & RFR_RES_POIS)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(m_ptr, RFR_RES_POIS);
        }
        break;
    case GF_NUKE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_IM_POIS)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_IM_POIS);
        }
        else if (r_ptr->flagsr & RFR_RES_POIS)
        {
            note = " resists.";
            dam /= 2;
            mon_lore_r(m_ptr, RFR_RES_POIS);
        }
        else if (one_in_(3)) do_poly = TRUE;
        break;
    case GF_HELL_FIRE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags3 & RF3_GOOD)
        {
            dam *= 2;
            note = " is hit hard.";
            mon_lore_3(m_ptr, RF3_GOOD);
        }
        break;
    case GF_HOLY_FIRE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags3 & RF3_GOOD)
        {
            dam = 0;
            note = " is immune.";
            mon_lore_3(m_ptr, RF3_GOOD);
        }
        else if (r_ptr->flags3 & RF3_EVIL)
        {
            dam *= 2;
            note = " is hit hard.";
            mon_lore_3(m_ptr, RF3_EVIL);
        }
        else
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
        }
        break;
    case GF_ARROW:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_PLASMA:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_PLAS)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(m_ptr, RFR_RES_PLAS);
        }
        break;
    case GF_NETHER:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_NETH)
        {
            if (r_ptr->flags3 & RF3_UNDEAD)
            {
                note = " is immune.";
                dam = 0;
                mon_lore_3(m_ptr, RF3_UNDEAD);
            }
            else
            {
                note = " resists.";
                dam *= 3; dam /= randint1(6) + 6;
            }
            mon_lore_r(m_ptr, RFR_RES_NETH);
        }
        else if (r_ptr->flags3 & RF3_EVIL)
        {
            dam /= 2;
            note = " resists somewhat.";
            mon_lore_3(m_ptr, RF3_EVIL);
        }
        break;
    case GF_WATER:
    case GF_WATER2:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_WATE)
        {
            if ((m_ptr->r_idx == MON_WATER_ELEM) || (m_ptr->r_idx == MON_UNMAKER))
            {
                note = " is immune.";
                dam = 0;
            }
            else
            {
                note = " resists.";
                dam *= 3; dam /= randint1(6) + 6;
            }
            mon_lore_r(m_ptr, RFR_RES_WATE);
        }
        else if (who == GF_WHO_PLAYER && _stun_save(r_ptr->level, dam))
        {
            note = " resists stunning.";
        }
        else do_stun = _stun_amount(dam);
        break;
    case GF_CHAOS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_CHAO)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(m_ptr, RFR_RES_CHAO);
        }
        else if ((r_ptr->flags3 & RF3_DEMON) && one_in_(3))
        {
            note = " resists somewhat.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_3(m_ptr, RF3_DEMON);
        }
        else
        {
            do_poly = TRUE;
            /* Try to make the Chaos Vortex more playable. With too frequent
             * polymorphing, you always seem to get stuck on a chaos resistant
             * foe eventually. */
            if (p_ptr->current_r_idx == MON_CHAOS_VORTEX && !one_in_(5))
                do_poly = FALSE;
            do_conf = _gf_distance_mod(5 + randint1(11));
        }
        break;
    case GF_SHARDS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_SHAR)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(m_ptr, RFR_RES_SHAR);
        }
        break;
    case GF_ROCKET:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_SHAR)
        {
            note = " resists somewhat.";
            dam /= 2;
            mon_lore_r(m_ptr, RFR_RES_SHAR);
        }
        break;
    case GF_ELDRITCH_STUN:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_SOUN)
            mon_lore_r(m_ptr, RFR_RES_SOUN);
        else if (r_ptr->flags3 & RF3_NO_STUN)
            mon_lore_3(m_ptr, RF3_NO_STUN);
        else if (mon_save_p(m_ptr->r_idx, A_CHR)
              || ((r_ptr->flags1 & RF1_UNIQUE) && mon_save_p(m_ptr->r_idx, A_CHR)) )
        {
            note = " resists stunning.";
        }
        else
            do_stun = 3 + randint0(5);
        break;
    case GF_ROCK:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (!(r_ptr->flagsr & RFR_RES_SOUN))
        {
            if (who == GF_WHO_PLAYER  && _stun_save(r_ptr->level, dam))
                note = " resists stunning.";
            else
                do_stun = _stun_amount(dam);
        }
        break;
    case GF_SOUND:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_SOUN)
        {
            note = " resists.";
            dam *= 2; dam /= randint1(6) + 6;
            mon_lore_r(m_ptr, RFR_RES_SOUN);
        }
        else if (who == GF_WHO_PLAYER && _stun_save(r_ptr->level, dam))
            note = " resists stunning.";
        else
            do_stun = _stun_amount(dam);
        break;
    case GF_ELDRITCH_DISPEL:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        dispel_monster_status(c_ptr->m_idx);

        /* More Powerful vs. Evil */
        if ((r_ptr->flags3 & RF3_EVIL) && one_in_(5))
        {
            msg_print("You blast the forces of evil with great power!");
            dam = dam * 2;

            /* Attempt a Powerful Slow */
            if (r_ptr->level > randint1(p_ptr->lev * 2))
            {
                note = " resists being slowed!";
            }
            else
            {
                if (set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
                    note = " starts moving slower.";
            }
        }
        break;
    case GF_PHARAOHS_CURSE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        dam = spell_power(MIN(m_ptr->hp*8/100 + dam, 400));
        if (p_ptr->lev >= 50)
            dam += 50;
        note = " is cursed by ancient pharaohs of long ago!";
        break;
    case GF_ELDRITCH_CONFUSE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        /* 593 monsters have NO_CONF ... it is more widely resisted than fire ...
           Eldritch confusion will bypass this check, but that is hacked below */
        if (MON_CONFUSED(m_ptr))
            note = " is already confused.";
        else if ((r_ptr->flags3 & RF3_NO_CONF) && r_ptr->level + randint1(100) > p_ptr->lev*2 + (p_ptr->stat_ind[A_CHR] + 3) - 10)
            note = " resists confusion.";
        else
        {
            /* Recovery is randint1(r_info[m_ptr->r_idx].level / 20 + 1) */
            do_conf = 3 + randint0(5);
        }
        break;
    case GF_CONFUSION:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags3 & RF3_NO_CONF)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_3(m_ptr, RF3_NO_CONF);
        }
        else do_conf = _gf_distance_mod(10 + randint1(15));
        break;
    case GF_DISENCHANT:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_DISE)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(m_ptr, RFR_RES_DISE);
        }
        break;
    case GF_NEXUS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_NEXU)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(m_ptr, RFR_RES_NEXU);
        }
        break;
    case GF_FORCE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_WALL)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(m_ptr, RFR_RES_WALL);
        }
        else
            do_stun = _stun_amount(dam);
        break;
    case GF_INERT:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_INER)
        {
            note = " resists.";

            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(m_ptr, RFR_RES_INER);
        }
        else
        {
            /* Powerful monsters can resist */
            if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
                (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                obvious = FALSE;
            }
            /* Normal monsters slow down */
            else if (set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
                note = " starts moving slower.";
        }
        break;
    case GF_AMNESIA: {
        int ml = r_ptr->level;
        if (r_ptr->flags1 & RF1_UNIQUE)
            ml += 10;

        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (randint1(dam) <= randint1(ml))
            note = " resists.";
        else if (mon_amnesia(c_ptr->m_idx))
            note = " seems to have forgotten something.";
        else
            note = " doesn't seem to know much.";
        dam = 0;
        break; }
    case GF_TIME:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_TIME)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(m_ptr, RFR_RES_TIME);
        }
        else if (!who)
        {
            int ml = r_ptr->level;
            int pl = p_ptr->lev;
            bool unique = FALSE;

            if (r_ptr->flags1 & RF1_UNIQUE)
                unique = TRUE;

            if (unique)
                ml += 10;

            if (p_ptr->lev > 40)
                pl += (p_ptr->lev - 40) * 2;

            if (randint1(pl) <= randint1(ml))
            {
                #ifdef _DEBUG
                note = " resists the ravages of time.";
                #endif
            }
            else
            {
                int which = randint1(100);
                if (which <= 15)
                {
                    if (unique)
                    {
                        #ifdef _DEBUG
                        note = " cannot be slowed.";
                        #endif
                    }
                    else if(set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
                    {
                        #ifdef _DEBUG
                        note = " starts moving slower.";
                        #endif
                    }
                    else
                    {
                        #ifdef _DEBUG
                        note = " is already slow.";
                        #endif
                    }
                }
                else if (which <= 20)
                {
                    if (devolve_monster(c_ptr->m_idx, p_ptr->wizard))
                        return TRUE;
                }
                else if (which <= 25)
                {
                    if (evolve_monster(c_ptr->m_idx, p_ptr->wizard))
                        return TRUE;
                }
                else if (which <= 40)
                {
                    if (!unique)
                    {
                        note = " is suspended!";
                        do_paralyzed = 5;
                    }
                    else
                    {
                        #ifdef _DEBUG
                        note = " cannot be suspended.";
                        #endif
                    }
                }
                else if (which <= 50)
                {
                    m_ptr->ac_adj -= randint1(10);
                    note = " is more exposed!";
                }
                else if (which <= 60)
                {
                    do_time = (dam + 1) / 2;
                }
                else if (which <= 70)
                {
                    m_ptr->mspeed -= randint1(2);
                    note = " is permanently slowed!";
                }
                else if (which <= 80)
                {
                    if (mon_amnesia(c_ptr->m_idx))
                        note = " seems to have forgotten something.";
                    else
                    {
                        #ifdef _DEBUG
                        note = " is unaffected by memory loss.";
                        #endif
                    }
                }
                else if (which <= 90)
                {
                    m_ptr->melee_adj -= randint1(1 + r_ptr->level/10);
                    note = " shrinks!";
                }
                else
                {
                    note = " is suddenly sluggish.";
                    m_ptr->energy_need += ENERGY_NEED();
                }
            }
        }
        else
        {
            do_time = (dam + 1) / 2;
        }
        break;
    case GF_STORM: /* TODO */
    case GF_GRAVITY: {
        bool resist_tele = FALSE;

        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_TELE)
        {
            if (r_ptr->flags1 & (RF1_UNIQUE))
            {
                mon_lore_r(m_ptr, RFR_RES_TELE);
                note = " is unaffected!";
                resist_tele = TRUE;
            }
            else if (r_ptr->level > randint1(100))
            {
                mon_lore_r(m_ptr, RFR_RES_TELE);
                note = " resists!";
                resist_tele = TRUE;
            }
        }

        if (!resist_tele) do_dist = 10;
        else do_dist = 0;
        if (p_ptr->riding && (c_ptr->m_idx == p_ptr->riding)) do_dist = 0;

        if (type == GF_GRAVITY && (r_ptr->flagsr & RFR_RES_GRAV))
        {
            note = " resists.";

            dam *= 3; dam /= randint1(6) + 6;
            do_dist = 0;
            mon_lore_r(m_ptr, RFR_RES_GRAV);
        }
        else
        {
            /* 1. slowness */
            /* Powerful monsters can resist */
            if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
                (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                obvious = FALSE;
            }
            /* Normal monsters slow down */
            else
            {
                if (set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
                {
                    note = " starts moving slower.";
                }
            }

            /* 2. stun */
            do_stun = damroll((caster_lev / 20) + 3 , (dam)) + 1;

            /* Attempt a saving throw */
            if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
                (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                /* Resist */
                do_stun = 0;
                /* No obvious effect */
                note = " is unaffected!";

                obvious = FALSE;
            }
        }
        break; }
    case GF_MANA:
    case GF_SEEKER:
    case GF_SUPER_RAY:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_DISINTEGRATE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags3 & RF3_HURT_ROCK)
        {
            mon_lore_3(m_ptr, RF3_HURT_ROCK);
            note = " loses some skin!";
            note_dies = " evaporates!";
            dam *= 2;
        }
        break;
    case GF_PSI_STORM:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags2 & RF2_EMPTY_MIND)
        {
            dam = 0;
            note = " is immune!";
            mon_lore_2(m_ptr, RF2_EMPTY_MIND);
            break;

        }
        else if (r_ptr->flags2 & (RF2_STUPID | RF2_WEIRD_MIND))
        {
            dam /= 3;
            note = " resists.";
            break;
        }
        if (one_in_(4))
        {
            if (p_ptr->riding && (c_ptr->m_idx == p_ptr->riding)) do_dist = 0;
            else do_dist = 7;
        }
        if (one_in_(2))
        {
            int mult = 1;

            do_stun = damroll(caster_lev/20 + 3, dam) + 1;
            if (r_ptr->flags1 & RF1_UNIQUE)
                mult++;

            if (mult*r_ptr->level > 5 + randint1(dam))
            {
                do_stun = 0;
                obvious = FALSE;
            }
        }
        if (one_in_(4))
        {
            switch (randint1(3))
            {
                case 1:
                    do_conf = 3 + randint1(dam);
                    break;
                case 2:
                    do_fear = 3 + randint1(dam);
                    break;
                case 3:
                    note = " falls asleep!";
                    do_sleep = 3 + randint1(dam);
                    break;
            }
        }
        note_dies = " collapses, a mindless husk.";
        break;
    case GF_PSI:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        /* PSI only works if the monster can see you! -- RG */
        if (!(los(m_ptr->fy, m_ptr->fx, py, px)))
        {
            if (seen_msg) msg_format("%^s can't see you, and isn't affected!", m_name);
            skipped = TRUE;
            break;
        }
        if (r_ptr->flags2 & RF2_EMPTY_MIND)
        {
            dam = 0;
            note = " is immune!";
            mon_lore_2(m_ptr, RF2_EMPTY_MIND);

        }
        else if ((r_ptr->flags2 & (RF2_STUPID | RF2_WEIRD_MIND)) ||
                 (r_ptr->flags3 & RF3_ANIMAL) ||
                 (r_ptr->level > randint1(3 * dam)))
        {
            dam /= 3;
            note = " resists.";


            /*
             * Powerful demons & undead can turn a mindcrafter's
             * attacks back on them
             */
            if ((r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
                (r_ptr->level > p_ptr->lev / 2) &&
                one_in_(2))
            {
                note = NULL;
                msg_format("%^s%s corrupted mind backlashes your attack!",
                    m_name, (seen ? "'s" : "s"));

                /* Saving throw */
                if ((randint0(100 + r_ptr->level / 2) < p_ptr->skills.sav) && !CHECK_MULTISHADOW())
                {
                    msg_print("You resist the effects!");

                }
                else
                {
                    /* Injure +/- confusion */
                    monster_desc(killer, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
                    take_hit(DAMAGE_ATTACK, dam, killer, -1);  /* has already been /3 */
                    if (one_in_(4) && !CHECK_MULTISHADOW())
                    {
                        switch (randint1(4))
                        {
                            case 1:
                                set_confused(p_ptr->confused + 3 + randint1(dam), FALSE);
                                break;
                            case 2:
                                set_stun(p_ptr->stun + randint1(dam), FALSE);
                                break;
                            case 3:
                            {
                                if (r_ptr->flags3 & RF3_NO_FEAR)
                                    note = " is unaffected.";

                                else
                                    fear_add_p(FEAR_SCARED);
                                break;
                            }
                            default:
                                if (!p_ptr->free_act)
                                    (void)set_paralyzed(randint1(dam), FALSE);
                                else equip_learn_flag(OF_FREE_ACT);
                                break;
                        }
                    }
                }
                dam = 0;
            }
        }

        if ((dam > 0) && one_in_(4))
        {
            switch (randint1(4))
            {
                case 1:
                    do_conf = 3 + randint1(dam);
                    break;
                case 2:
                    do_stun = 3 + randint1(dam);
                    break;
                case 3:
                    do_fear = 3 + randint1(dam);
                    break;
                default:
                    note = " falls asleep!";

                    do_sleep = 3 + randint1(dam);
                    break;
            }
        }

        note_dies = " collapses, a mindless husk.";
        break;
    case GF_PSI_DRAIN:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags2 & RF2_EMPTY_MIND)
        {
            dam = 0;
            note = " is immune!";

        }
        else if ((r_ptr->flags2 & (RF2_STUPID | RF2_WEIRD_MIND)) ||
                 (r_ptr->flags3 & RF3_ANIMAL) ||
                 (r_ptr->level > randint1(3 * dam)))
        {
            dam /= 3;
            note = " resists.";


            /*
             * Powerful demons & undead can turn a mindcrafter's
             * attacks back on them
             */
            if ((r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
                 (r_ptr->level > p_ptr->lev / 2) &&
                 (one_in_(2)))
            {
                note = NULL;
                msg_format("%^s%s corrupted mind backlashes your attack!",
                    m_name, (seen ? "'s" : "s"));

                /* Saving throw */
                if ((randint0(100 + r_ptr->level / 2) < p_ptr->skills.sav) && !CHECK_MULTISHADOW())
                {
                    msg_print("You resist the effects!");
                }
                else
                {
                    /* Injure + mana drain */
                    monster_desc(killer, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
                    if (!CHECK_MULTISHADOW())
                    {
                        msg_print("Your psychic energy is drained!");

                        p_ptr->csp -= damroll(5, dam) / 2;
                        if (p_ptr->csp < 0) p_ptr->csp = 0;
                        p_ptr->redraw |= PR_MANA;
                        p_ptr->window |= (PW_SPELL);
                    }
                    take_hit(DAMAGE_ATTACK, dam, killer, -1);  /* has already been /3 */
                }
                dam = 0;
            }
        }
        else if (dam > 0)
        {
            int b = damroll(5, dam) / 4;
            cptr str = (p_ptr->pclass == CLASS_MINDCRAFTER) ? "psychic energy" : "mana";
            msg_format("You convert %s%s pain into %s!",
                m_name, (seen ? "'s" : "s"), str);

            b = MIN(p_ptr->msp, p_ptr->csp + b);
            p_ptr->csp = b;
            p_ptr->redraw |= PR_MANA;
            p_ptr->window |= (PW_SPELL);
        }

        note_dies = " collapses, a mindless husk.";
        break;
    case GF_TELEKINESIS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (one_in_(4))
        {
            if (p_ptr->riding && (c_ptr->m_idx == p_ptr->riding)) do_dist = 0;
            else do_dist = 7;
        }

        /* 1. stun */
        do_stun = damroll((caster_lev / 20) + 3 , dam) + 1;

        /* Attempt a saving throw */
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            (r_ptr->level > 5 + randint1(dam)))
        {
            /* Resist */
            do_stun = 0;
            /* No obvious effect */
            obvious = FALSE;
        }
        break;
    case GF_PSY_SPEAR:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_METEOR:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_DOMINATION: {
        int power = dam;

        /* No "real" damage */
        dam = 0;

        if (!is_hostile(m_ptr)) break;
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        /* Attempt a saving throw */
        if (randint1(r_ptr->level) > randint1(power))
        {
            /*
             * Powerful demons & undead can turn a mindcrafter's
             * attacks back on them
             */
            if ((r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
                (r_ptr->level > p_ptr->lev / 2) &&
                (one_in_(2)))
            {
                note = NULL;
                msg_format("%^s%s corrupted mind backlashes your attack!",
                    m_name, (seen ? "'s" : "s"));

                /* Saving throw */
                if (randint0(100 + r_ptr->level/2) < p_ptr->skills.sav)
                {
                    msg_print("You resist the effects!");

                }
                else
                {
                    /* Confuse, stun, terrify */
                    switch (randint1(4))
                    {
                        case 1:
                            set_stun(p_ptr->stun + power / 2, FALSE);
                            break;
                        case 2:
                            set_confused(p_ptr->confused + power / 2, FALSE);
                            break;
                        default:
                        {
                            if (r_ptr->flags3 & RF3_NO_FEAR)
                                note = " is unaffected.";

                            else
                                fear_add_p(power);
                        }
                    }
                }
            }
            else
            {
                note = " is unaffected!";
                obvious = FALSE;
            }
        }
        else
        {
            bool unique = BOOL(r_ptr->flags1 & RF1_UNIQUE);

            if (!unique && !(m_ptr->mflag2 & MFLAG2_QUESTOR) && power > 29 && randint1(100) < power)
            {
                note = " is in your thrall!";
                set_pet(m_ptr);
            }
            else
            {
                switch (randint1(4))
                {
                case 1:
                    do_stun = power / 2;
                    break;
                case 2:
                    if (r_ptr->flags3 & RF3_NO_CONF)
                    {
                        mon_lore_3(m_ptr, RF3_NO_CONF);
                        note = " is unaffected.";
                        break;
                    }
                    else if (!unique)
                    {
                        do_conf = power / 2;
                        break;
                    }
                default:
                    if (prace_is_(RACE_MON_VAMPIRE))
                    {
                        if (!unique && randint1(r_ptr->level) < randint1(power))
                        {
                            note = " is frozen in terror!";
                            do_paralyzed = randint1(3);
                        }
                        else
                        {
                            gf_damage_m(who, where, GF_AMNESIA, power, flags);
                        }
                        break;
                    }
                    do_fear = power;
                }
            }
        }
        break; }
    case GF_ICE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (who || !mon_save_p(m_ptr->r_idx, A_NONE))
            do_stun = _gf_distance_mod(randint1(15));
        if (r_ptr->flagsr & RFR_IM_COLD)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_IM_COLD);
        }
        else if (r_ptr->flagsr & RFR_RES_COLD)
        {
            note = " resists.";
            dam /= 2;
            mon_lore_r(m_ptr, RFR_RES_COLD);
        }
        else if (r_ptr->flags3 & (RF3_HURT_COLD))
        {
            note = " is hit hard.";
            dam *= 2;
            mon_lore_3(m_ptr, RF3_HURT_COLD);
        }
        break;
    case GF_OLD_DRAIN:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (!monster_living(r_ptr))
        {
            mon_lore_3(m_ptr, RF3_DEMON);
            mon_lore_3(m_ptr, RF3_UNDEAD);
            mon_lore_3(m_ptr, RF3_NONLIVING);
            note = " is unaffected!";
            obvious = FALSE;
            dam = 0;
        }
        else
        {
            if (dragon_vamp_hack)
                dragon_vamp_amt += dam;
            do_time = (dam+7)/8;
        }

            break;
    case GF_DEATH_TOUCH:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (!monster_living(r_ptr))
        {
            mon_lore_3(m_ptr, RF3_DEMON);
            mon_lore_3(m_ptr, RF3_UNDEAD);
            mon_lore_3(m_ptr, RF3_NONLIVING);
            note = " is immune.";
            obvious = FALSE;
            dam = 0;
        }
        else if (((r_ptr->flags1 & RF1_UNIQUE) && randint1(888) != 666)
                || mon_save_p(m_ptr->r_idx, A_INT)
                || mon_save_p(m_ptr->r_idx, A_INT) )
        {
            note = " resists!";
            obvious = FALSE;
            dam = 0;
        }
        break;
    case GF_DEATH_RAY:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (!monster_living(r_ptr))
        {
            mon_lore_3(m_ptr, RF3_DEMON);
            mon_lore_3(m_ptr, RF3_UNDEAD);
            mon_lore_3(m_ptr, RF3_NONLIVING);
            note = " is immune.";
            obvious = FALSE;
            dam = 0;
        }
        else if ( ((r_ptr->flags1 & RF1_UNIQUE) && randint1(888) != 666)
               || (r_ptr->level + randint1(20) > randint1(caster_lev)))
        {
            note = " resists!";
            obvious = FALSE;
            dam = 0;
        }

        break;
    case GF_OLD_POLY:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        do_poly = TRUE;
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            (m_ptr->mflag2 & MFLAG2_QUESTOR) ||
            (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
        {
            note = " is unaffected!";

            do_poly = FALSE;
            obvious = FALSE;
        }
        dam = 0;
        break;
    case GF_OLD_CLONE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        if ( p_ptr->inside_arena
          || is_pet(m_ptr)
          || (r_ptr->flags1 & RF1_UNIQUE)
          || (r_ptr->flags7 & (RF7_NAZGUL | RF7_UNIQUE2))
          || (m_ptr->mflag2 & MFLAG2_QUESTOR) )
        {
            note = " is unaffected!";
        }
        else
        {
            m_ptr->hp = m_ptr->maxhp;
            if (multiply_monster(c_ptr->m_idx, TRUE, 0L))
                note = " spawns!";
        }
        dam = 0;
        break;
    case GF_STAR_HEAL:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        (void)set_monster_csleep(c_ptr->m_idx, 0);

        if (m_ptr->maxhp < m_ptr->max_maxhp)
        {
            if (seen_msg) msg_format("%^s recovers %s vitality.", m_name, m_poss);
            m_ptr->maxhp = m_ptr->max_maxhp;
        }

        if (!dam)
        {
            /* Redraw (later) if needed */
            check_mon_health_redraw(c_ptr->m_idx);
            break;
        }

        /* Fall through */
    case GF_OLD_HEAL:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        (void)set_monster_csleep(c_ptr->m_idx, 0);
        if (MON_STUNNED(m_ptr))
        {
            if (seen_msg) msg_format("%^s is no longer stunned.", m_name);
            (void)set_monster_stunned(c_ptr->m_idx, 0);
        }
        if (MON_CONFUSED(m_ptr))
        {
            if (seen_msg) msg_format("%^s is no longer confused.", m_name);
            (void)set_monster_confused(c_ptr->m_idx, 0);
        }
        if (MON_MONFEAR(m_ptr))
        {
            if (seen_msg) msg_format("%^s recovers %s courage.", m_name, m_poss);
            (void)set_monster_monfear(c_ptr->m_idx, 0);
        }

        if (!who && !(r_ptr->flags3 & RF3_EVIL))
            dam = dam * (625 + virtue_current(VIRTUE_COMPASSION))/625;

        /* Heal */
        if (m_ptr->hp < 30000) m_ptr->hp += dam;

        /* No overflow */
        if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

        if (!who)
        {
            virtue_add(VIRTUE_VITALITY, 1);

            if (r_ptr->flags1 & RF1_UNIQUE)
                virtue_add(VIRTUE_INDIVIDUALISM, 1);

            if (is_friendly(m_ptr))
                virtue_add(VIRTUE_HONOUR, 1);
            else if (!(r_ptr->flags3 & RF3_EVIL))
            {
                if (r_ptr->flags3 & RF3_GOOD)
                    virtue_add(VIRTUE_COMPASSION, 2);
                else
                    virtue_add(VIRTUE_COMPASSION, 1);
            }

            if (r_ptr->flags3 & RF3_ANIMAL)
                virtue_add(VIRTUE_NATURE, 1);
        }

        if (m_ptr->r_idx == MON_LEPER)
        {
            heal_leper = TRUE;
            if (!who) virtue_add(VIRTUE_COMPASSION, 5);
        }

        /* Redraw (later) if needed */
        check_mon_health_redraw(c_ptr->m_idx);

        /* Message */
        note = " looks healthier.";

        /* No "real" damage */
        dam = 0;
        break;
    case GF_OLD_SPEED:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        /* Speed up */
        if (set_monster_fast(c_ptr->m_idx, MON_FAST(m_ptr) + 100))
        {
            note = " starts moving faster.";
        }

        if (!who)
        {
            if (r_ptr->flags1 & RF1_UNIQUE)
                virtue_add(VIRTUE_INDIVIDUALISM, 1);
            if (is_friendly(m_ptr))
                virtue_add(VIRTUE_HONOUR, 1);
        }

        /* No "real" damage */
        dam = 0;
        break;
    case GF_OLD_SLOW:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        if ( (r_ptr->flags1 & RF1_UNIQUE)
          || r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10 )
        {
            note = " is unaffected!";
            obvious = FALSE;
        }
        else
        {
            if (set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
                note = " starts moving slower.";
        }
        dam = 0;
        break;
    case GF_UNHOLY_WORD:
        if (is_pet(m_ptr) && (r_ptr->flags3 & RF3_EVIL))
        {
            if (seen) obvious = TRUE;

            set_monster_csleep(c_ptr->m_idx, 0);
            if (MON_STUNNED(m_ptr))
            {
                if (seen_msg) msg_format("%^s is no longer stunned.", m_name);
                set_monster_stunned(c_ptr->m_idx, 0);
            }
            if (MON_CONFUSED(m_ptr))
            {
                if (seen_msg) msg_format("%^s is no longer confused.", m_name);
                set_monster_confused(c_ptr->m_idx, 0);
            }
            if (MON_MONFEAR(m_ptr))
            {
                if (seen_msg) msg_format("%^s recovers %s courage.", m_name, m_poss);
                set_monster_monfear(c_ptr->m_idx, 0);
            }

            if (m_ptr->hp < 30000) m_ptr->hp += dam;
            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
            set_monster_fast(c_ptr->m_idx, MON_FAST(m_ptr) + 100);
            note = " fights with renewed vigor!";
        }
        dam = 0;
        break;
    case GF_STASIS_EVIL:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            !(r_ptr->flags3 & RF3_EVIL) ||
            (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
        {
            note = " is unaffected!";

            obvious = FALSE;
        }
        else
        {
            note = " is suspended!";
            do_paralyzed = 5;
        }
        dam = 0;
        break;
    case GF_PARALYSIS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((r_ptr->flags1 & RF1_UNIQUE) || r_ptr->level > randint1(dam))
        {
            note = " is unaffected!";
            obvious = FALSE;
        }
        else
        {
            note = " is paralyzed!";
            do_paralyzed = randint1(3);
        }
        dam = 0;
        break;
    case GF_STASIS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((r_ptr->flags1 & RF1_UNIQUE) || r_ptr->level > randint1(dam))
        {
            note = " is unaffected!";
            obvious = FALSE;
        }
        else
        {
            note = " is suspended!";
            do_paralyzed = 3;
        }
        dam = 0;
        break;
    case GF_SUBJUGATION: {
        bool unique = BOOL(r_ptr->flags1 & RF1_UNIQUE);
        int  attempts = randint1(1 + p_ptr->lev/50);
        int  ct = 0;

        if (seen) obvious = TRUE;
        set_monster_csleep(c_ptr->m_idx, 0);
        _BABBLE_HACK()
        if (is_pet(m_ptr))
            return FALSE;
        while (attempts--)
        {
            switch (randint1(5))
            {
            case 1:
                if (randint1(r_ptr->level) <= randint1(dam))
                {
                    do_stun = dam / 2;
                    ct++;
                }
                break;
            case 2:
                if (!(r_ptr->flags3 & RF3_NO_CONF) && randint1(r_ptr->level) <= randint1(dam))
                {
                    do_conf = dam / 2;
                    ct++;
                }
                break;
            case 3:
                if (!unique && !(m_ptr->mflag2 & MFLAG2_QUESTOR) && randint1(r_ptr->level) <= randint1(dam))
                {
                    note = " is frozen in terror!";
                    do_paralyzed = randint1(3);
                    attempts = 0;
                    ct++;
                }
                break;
            case 4:
                if (randint1(r_ptr->level) <= randint1(dam))
                {
                    do_fear = dam;
                    ct++;
                }
                break;
            default:
                if (unique || (m_ptr->mflag2 & MFLAG2_QUESTOR) || p_ptr->inside_arena)
                {
                }
                else if ((m_ptr->mflag2 & MFLAG2_NOPET) || randint1(r_ptr->level) > randint1(dam))
                {
                    if (one_in_(4))
                        m_ptr->mflag2 |= MFLAG2_NOPET;
                }
                else
                {
                    msg_format("%s bows to your will!", m_name);

                    set_pet(m_ptr);
                    attempts = 0;
                    ct++;

                    virtue_add(VIRTUE_INDIVIDUALISM, -1);
                    if (r_ptr->flags3 & RF3_ANIMAL)
                        virtue_add(VIRTUE_NATURE, 1);

                    /* Ignore any prior effects */
                    return TRUE;
                }
            }
        }
        if (!ct)
            note = " resists";
        dam = 0;
        break; }
    case GF_ELDRITCH_HOWL:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            break;
        }
        if (seen) obvious = TRUE;

        do_fear = damroll(3, (dam / 2)) + 1;

        if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
            (r_ptr->flags3 & (RF3_NO_FEAR)))
        {
            note = " is unaffected!";
            obvious = FALSE;
            do_fear = 0;
        }
        else if (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
        {
            note = " resists!";
            obvious = FALSE;
            do_fear = 0;
        }
        else if (r_ptr->level <= randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
        {
            note = " is frozen with terror!";
            do_paralyzed = randint1(3);
        }
        dam = 0;
        break;
    case GF_CHARM:
    case GF_CHARM_RING_BEARER:
        if (seen) obvious = TRUE;

        if (type == GF_CHARM_RING_BEARER)
        {
            if (!mon_is_type(m_ptr->r_idx, SUMMON_RING_BEARER))
            {
                note = " is not a suitable ring bearer.";
                dam = 0;
                break;
            }
        }
        else
        {
            dam += (adj_con_fix[p_ptr->stat_ind[A_CHR]] - 1);
            dam += virtue_current(VIRTUE_HARMONY)/10;
            dam -= virtue_current(VIRTUE_INDIVIDUALISM)/20;
            if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
                dam = dam * 2 / 3;
        }

        if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_RES_ALL);
            break;
        }
        else if (m_ptr->mflag2 & MFLAG2_QUESTOR)
        {
            note = " is unaffected!";
            obvious = FALSE;
        }
        else if ((m_ptr->mflag2 & MFLAG2_NOPET) || r_ptr->level > randint1(dam))
        {
            note = " resists!";
            obvious = FALSE;
            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else if (p_ptr->cursed & OFC_AGGRAVATE)
        {
            note = " hates you too much!";
            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else
        {
            note = " suddenly seems friendly!";

            set_pet(m_ptr);

            virtue_add(VIRTUE_INDIVIDUALISM, -1);
            if (r_ptr->flags3 & RF3_ANIMAL)
                virtue_add(VIRTUE_NATURE, 1);
        }
        dam = 0;
        break;
    case GF_CONTROL_UNDEAD:
        if (seen) obvious = TRUE;

        dam += virtue_current(VIRTUE_UNLIFE)/10;
        dam -= virtue_current(VIRTUE_INDIVIDUALISM)/20;

        if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_RES_ALL);
            break;
        }

        if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
            dam = dam * 2 / 3;

        /* Attempt a saving throw */
        if ((m_ptr->mflag2 & MFLAG2_QUESTOR) ||
            !(r_ptr->flags3 & RF3_UNDEAD) ||
            (m_ptr->mflag2 & MFLAG2_NOPET) ||
            (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
        {
            /* No obvious effect */
            note = " is unaffected!";

            obvious = FALSE;
            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else if (p_ptr->cursed & OFC_AGGRAVATE)
        {
            note = " hates you too much!";

            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else
        {
            note = " is in your thrall!";

            set_pet(m_ptr);
        }
        dam = 0;
        break;
    case GF_CONTROL_DEMON:
        if (seen) obvious = TRUE;

        dam += virtue_current(VIRTUE_UNLIFE)/10;
        dam -= virtue_current(VIRTUE_INDIVIDUALISM)/20;

        if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_RES_ALL);
            break;
        }

        if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
            dam = dam * 2 / 3;

        /* Attempt a saving throw */
        if ((m_ptr->mflag2 & MFLAG2_QUESTOR) ||
            !(r_ptr->flags3 & RF3_DEMON) ||
            (m_ptr->mflag2 & MFLAG2_NOPET) ||
            (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
        {
            /* No obvious effect */
            note = " is unaffected!";

            obvious = FALSE;
            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else if (p_ptr->cursed & OFC_AGGRAVATE)
        {
            note = " hates you too much!";

            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else
        {
            note = " is in your thrall!";

            set_pet(m_ptr);
        }

        /* No "real" damage */
        dam = 0;
        break;
    case GF_CONTROL_ANIMAL:
        if (seen) obvious = TRUE;

        dam += virtue_current(VIRTUE_NATURE)/10;
        dam -= virtue_current(VIRTUE_INDIVIDUALISM)/20;

        if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_RES_ALL);
            break;
        }

        if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
            dam = dam * 2 / 3;

        /* Attempt a saving throw */
        if ((m_ptr->mflag2 & MFLAG2_QUESTOR) ||
            !(r_ptr->flags3 & RF3_ANIMAL) ||
            (m_ptr->mflag2 & MFLAG2_NOPET) ||
            (r_ptr->flags3 & RF3_NO_CONF) ||
            (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
        {
            /* Memorize a flag */
            if (r_ptr->flags3 & (RF3_NO_CONF))
            {
                mon_lore_3(m_ptr, RF3_NO_CONF);
            }

            /* Resist */
            /* No obvious effect */
            note = " is unaffected!";

            obvious = FALSE;
            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else if (p_ptr->cursed & OFC_AGGRAVATE)
        {
            note = " hates you too much!";

            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else
        {
            note = " is tamed!";

            set_pet(m_ptr);

            if (r_ptr->flags3 & RF3_ANIMAL)
                virtue_add(VIRTUE_NATURE, 1);
        }

        /* No "real" damage */
        dam = 0;
        break;
    case GF_CONTROL_PACT_MONSTER:
        if (warlock_is_pact_monster(r_ptr) && !is_pet(m_ptr))
        {
            if (seen) obvious = TRUE;

            /* Attempt a saving throw */
            if ( (m_ptr->mflag2 & MFLAG2_QUESTOR)
              || (m_ptr->mflag2 & MFLAG2_NOPET)
              || mon_save_p(m_ptr->r_idx, A_CHR)
              || ((r_ptr->flags1 & RF1_UNIQUE) && mon_save_p(m_ptr->r_idx, A_CHR)) )
            {
                if (warlock_is_(WARLOCK_HOUNDS))
                    note = " growls at you in defiance!";
                else
                    note = " resists your control.";
                obvious = FALSE;
                if (one_in_(4))
                    m_ptr->mflag2 |= MFLAG2_NOPET;
            }
            else if (p_ptr->cursed & OFC_AGGRAVATE)
            {
                note = " finds you very aggravating!";
                if (one_in_(4))
                    m_ptr->mflag2 |= MFLAG2_NOPET;
            }
            else
            {
                if (warlock_is_(WARLOCK_HOUNDS))
                    note = " rolls on its back in submission.";
                else
                    note = " obeys your will.";
                set_pet(m_ptr);
            }
        }

        /* No "real" damage */
        dam = 0;
        break;
    case GF_CONTROL_LIVING:
        if (seen) obvious = TRUE;

        dam += (adj_chr_chm[p_ptr->stat_ind[A_CHR]]);
        dam -= virtue_current(VIRTUE_UNLIFE)/10;
        dam -= virtue_current(VIRTUE_INDIVIDUALISM)/20;

        if (r_ptr->flags3 & (RF3_NO_CONF)) dam -= 30;
        if (dam < 1) dam = 1;
        msg_format("You stare into %s.", m_name_object);
        if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(m_ptr, RFR_RES_ALL);
            break;
        }

        if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
            dam = dam * 2 / 3;

        /* Attempt a saving throw */
        if ((m_ptr->mflag2 & MFLAG2_QUESTOR) ||
            (m_ptr->mflag2 & MFLAG2_NOPET) ||
             !monster_living(r_ptr) ||
             ((r_ptr->level+10) > randint1(dam)))
        {
            /* Resist */
            /* No obvious effect */
            note = " is unaffected!";

            obvious = FALSE;
            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else if (p_ptr->cursed & OFC_AGGRAVATE)
        {
            note = " hates you too much!";

            if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
        }
        else
        {
            note = " is tamed!";

            set_pet(m_ptr);

            if (r_ptr->flags3 & RF3_ANIMAL)
                virtue_add(VIRTUE_NATURE, 1);
        }
        dam = 0;
        break;
    case GF_OLD_SLEEP: {
        int ml = r_ptr->level;
        int pl = dam;

        if (r_ptr->flags1 & RF1_UNIQUE) ml += 3;

        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags3 & RF3_NO_SLEEP)
        {
            note = " is immune!";
            mon_lore_3(m_ptr, RF3_NO_SLEEP);
            obvious = FALSE;
        }
        else if (r_ptr->flags1 & RF1_UNIQUE) /* Historical ... I'd like to remove this. */
        {
            note = " is immune!";
            obvious = FALSE;
        }
        else if (randint1(ml) >= randint1(pl))
        {
            note = " resists!";
            obvious = FALSE;
        }
        else
        {
            note = " falls asleep!";
            do_sleep = 500;
        }
        dam = 0;
        break; }
    case GF_OLD_CONF: {
        int ml = r_ptr->level;
        int pl = dam;

        if (r_ptr->flags1 & RF1_UNIQUE) ml += 3;

        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        do_conf = damroll(3, (dam / 2)) + 1;
        if (r_ptr->flags3 & RF3_NO_CONF)
        {
            do_conf = 0;
            note = " is immune!";
            mon_lore_3(m_ptr, RF3_NO_CONF);
            obvious = FALSE;
        }
        else if (r_ptr->flags1 & RF1_UNIQUE) /* Historical ... I'd like to remove this. */
        {
            do_conf = 0;
            note = " is immune!";
            obvious = FALSE;
        }
        else if (randint1(ml) >= randint1(pl))
        {
            do_conf = 0;
            note = " resists!";
            obvious = FALSE;
        }

        /* No "real" damage */
        dam = 0;
        break; }
    case GF_STUN: {
        int ml = r_ptr->level;
        int pl = dam;

        if (r_ptr->flags1 & RF1_UNIQUE) ml += 3;

        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        do_stun = damroll(caster_lev/20 + 3, dam) + 1;
        if (r_ptr->flags3 & RF3_NO_STUN)
        {
            do_stun = 0;
            note = " is unaffected!";
            mon_lore_3(m_ptr, RF3_NO_STUN);
            obvious = FALSE;
        }
        else if (randint1(ml) >= randint1(pl))
        {
            do_stun = 0;
            note = " resists!";
            obvious = FALSE;
        }
        dam = 0;
        break; }
    case GF_LITE_WEAK:
        if (!dam)
        {
            skipped = TRUE;
            break;
        }
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            dam = 0;
            break;
        }
        /* Hurt by light */
        if (r_ptr->flags3 & (RF3_HURT_LITE))
        {
            /* Obvious effect */
            if (seen) obvious = TRUE;

            /* Memorize the effects */
            mon_lore_3(m_ptr, RF3_HURT_LITE);

            /* Special effect */
            note = " cringes from the light!";
            note_dies = " shrivels away in the light!";

        }
        /* Normally no damage */
        else dam = 0;
        break;
    case GF_LITE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_LITE)
        {
            note = " resists.";

            dam *= 2; dam /= (randint1(6)+6);
            mon_lore_r(m_ptr, RFR_RES_LITE);
        }
        else if (r_ptr->flags3 & (RF3_HURT_LITE))
        {
            mon_lore_3(m_ptr, RF3_HURT_LITE);
            note = " cringes from the light!";
            note_dies = " shrivels away in the light!";

            dam *= 2;
        }
        break;
    case GF_DARK:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flagsr & RFR_RES_DARK)
        {
            note = " resists.";
            dam *= 2; dam /= (randint1(6)+6);
            mon_lore_r(m_ptr, RFR_RES_DARK);
        }
        break;
    case GF_KILL_WALL: /* e.g. Stone to Mud */
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            dam = 0;
            break;
        }
        if (r_ptr->flags3 & (RF3_HURT_ROCK))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(m_ptr, RF3_HURT_ROCK);
            note = " loses some skin!";
            note_dies = " dissolves!";
        }
        /* Usually, ignore the effects */
        else dam = 0;
        break;
    case GF_AWAY_UNDEAD:
        if (r_ptr->flags3 & (RF3_UNDEAD))
        {
            bool resists_tele = FALSE;
            if (r_ptr->flagsr & RFR_RES_TELE)
            {
                if ((r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flagsr & RFR_RES_ALL))
                {
                    mon_lore_r(m_ptr, RFR_RES_TELE);
                    note = " is unaffected!";

                    resists_tele = TRUE;
                }
                else if (r_ptr->level > randint1(100))
                {
                    mon_lore_r(m_ptr, RFR_RES_TELE);
                    note = " resists!";

                    resists_tele = TRUE;
                }
            }
            if (!resists_tele)
            {
                if (seen) obvious = TRUE;
                mon_lore_3(m_ptr, RF3_UNDEAD);
                do_dist = dam;
            }
        }
        else skipped = TRUE;
        dam = 0;
        break;
    case GF_AWAY_EVIL:
        if (r_ptr->flags3 & (RF3_EVIL))
        {
            bool resists_tele = FALSE;
            if (r_ptr->flagsr & RFR_RES_TELE)
            {
                if ((r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flagsr & RFR_RES_ALL))
                {
                    mon_lore_r(m_ptr, RFR_RES_TELE);
                    note = " is unaffected!";

                    resists_tele = TRUE;
                }
                else if (r_ptr->level > randint1(100))
                {
                    mon_lore_r(m_ptr, RFR_RES_TELE);
                    note = " resists!";

                    resists_tele = TRUE;
                }
            }
            if (!resists_tele)
            {
                if (seen) obvious = TRUE;
                mon_lore_3(m_ptr, RF3_EVIL);
                do_dist = dam;
            }
        }
        else skipped = TRUE;
        dam = 0;
        break;
    case GF_ISOLATION: {
        bool resists_tele = FALSE;
        if (c_ptr->m_idx == p_ptr->duelist_target_idx)
        {
            dam = 0;
            return TRUE;
        }
        if (r_ptr->flagsr & RFR_RES_TELE)
        {
            if (r_ptr->flagsr & RFR_RES_ALL)
            {
                mon_lore_r(m_ptr, RFR_RES_TELE);
                note = " is unaffected!";
                resists_tele = TRUE;
            }
            else if (mon_save_p(m_ptr->r_idx, A_DEX))
            {
                mon_lore_r(m_ptr, RFR_RES_TELE);
                note = " resists!";
                resists_tele = TRUE;
            }
        }
        if (!resists_tele)
        {
            if (seen) obvious = TRUE;
            do_dist = dam;
        }
        dam = 0;
        break; }
    case GF_AWAY_ALL: {
        bool resists_tele = FALSE;
        if (r_ptr->flagsr & RFR_RES_TELE)
        {
            if ( (r_ptr->flags1 & RF1_UNIQUE)
              || (r_ptr->flagsr & RFR_RES_ALL) 
              || (m_ptr->smart & (1U << SM_GUARDIAN)) )
            {
                mon_lore_r(m_ptr, RFR_RES_TELE);
                note = " is unaffected!";
                resists_tele = TRUE;
            }
            else if (r_ptr->level > randint1(100))
            {
                mon_lore_r(m_ptr, RFR_RES_TELE);
                note = " resists!";
                resists_tele = TRUE;
            }
        }
        else if (m_ptr->smart & (1U << SM_GUARDIAN))
        {
            note = " is unaffected!";
            resists_tele = TRUE;
        }
        if (!resists_tele)
        {
            if (seen) obvious = TRUE;
            do_dist = dam;
        }
        dam = 0;
        break; }
    case GF_TURN_UNDEAD:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            break;
        }
        if (r_ptr->flags3 & (RF3_UNDEAD))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(m_ptr, RF3_UNDEAD);

            do_fear = damroll(3, (dam / 2)) + 1;
            if (fear_save_m(m_ptr))
            {
                note = " is unaffected!";
                obvious = FALSE;
                do_fear = 0;
            }
        }
        else skipped = TRUE;
        dam = 0;
        break;
    case GF_TURN_EVIL:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            break;
        }
        if (r_ptr->flags3 & (RF3_EVIL))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(m_ptr, RF3_EVIL);

            do_fear = damroll(3, (dam / 2)) + 1;

            if (fear_save_m(m_ptr))
            {
                note = " is unaffected!";
                obvious = FALSE;
                do_fear = 0;
            }
        }
        else skipped = TRUE;
        dam = 0;
        break;
    case GF_TURN_ALL:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            break;
        }
        if (seen) obvious = TRUE;
        do_fear = damroll(3, (dam / 2)) + 1;
        if ((r_ptr->flags3 & RF3_NO_FEAR) || fear_save_m(m_ptr))
        {
            note = " is unaffected!";
            obvious = FALSE;
            do_fear = 0;
        }
        dam = 0;
        break;
    case GF_DISP_UNDEAD:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (r_ptr->flags3 & (RF3_UNDEAD))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(m_ptr, RF3_UNDEAD);
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_EVIL:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (r_ptr->flags3 & RF3_EVIL)
        {
            if (seen) obvious = TRUE;
            mon_lore_3(m_ptr, RF3_EVIL);
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_GOOD:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (r_ptr->flags3 & RF3_GOOD)
        {
            if (seen) obvious = TRUE;
            mon_lore_3(m_ptr, RF3_GOOD);
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_LIVING:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (monster_living(r_ptr))
        {
            if (seen) obvious = TRUE;
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_DEMON:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (r_ptr->flags3 & (RF3_DEMON))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(m_ptr, RF3_DEMON);
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_ALL:
        if (r_ptr->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (seen) obvious = TRUE;
        note = " shudders.";
        note_dies = " dissolves!";
        break;
    case GF_DRAINING_TOUCH:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((r_ptr->flags4 & ~(RF4_NOMAGIC_MASK)) || (r_ptr->flags5 & ~(RF5_NOMAGIC_MASK)) || (r_ptr->flags6 & ~(RF6_NOMAGIC_MASK)))
        {
            /*msg_format("You draw psychic energy from %s.", m_name);*/
            p_ptr->csp += dam;
            if (p_ptr->csp > p_ptr->msp)
            {
                p_ptr->csp = p_ptr->msp;
                p_ptr->csp_frac = 0;
            }
            p_ptr->redraw |= PR_MANA;
        }
        else
        {
            msg_format("%^s is unaffected.", m_name);
            dam = 0;
        }
        break;
    case GF_DRAIN_MANA:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((r_ptr->flags4 & ~(RF4_NOMAGIC_MASK)) || (r_ptr->flags5 & ~(RF5_NOMAGIC_MASK)) || (r_ptr->flags6 & ~(RF6_NOMAGIC_MASK)))
        {
            if (who > 0)
            {
                if (caster_ptr->hp < caster_ptr->maxhp)
                {
                    caster_ptr->hp += 6 * dam;
                    if (caster_ptr->hp > caster_ptr->maxhp) caster_ptr->hp = caster_ptr->maxhp;
                    check_mon_health_redraw(who);
                    monster_desc(killer, caster_ptr, 0);
                    msg_format("%^s appears healthier.", killer);
                }
            }
            else
            {
                msg_format("You draw psychic energy from %s.", m_name_object);
                hp_player(dam);
            }
        }
        else msg_format("%^s is unaffected.", m_name);
        dam = 0;
        break;
    case GF_ANTIMAGIC:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ( mon_save_p(m_ptr->r_idx, A_STR)
          && (!p_ptr->shero || mon_save_p(m_ptr->r_idx, A_STR)) )
        {
            msg_format("%^s resists!", m_name);
            dam = 0;
            m_ptr->anti_magic_ct = 0;
            return TRUE;
        }
        else
        {
            int dur = 2 + randint1(2);
            /* XXX Better odds of success is probably enough.
             * if (p_ptr->shero) dur *= 2; */
            m_ptr->anti_magic_ct = dur;
            msg_format("%^s can no longer cast spells!", m_name);
            dam = 0;
            return TRUE;
        }
        break;
    case GF_PSI_EGO_WHIP:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        /* Hack: No damage now ... simply latch the whip on and let process_monster()
           do the dirty work for us */
        (void)set_monster_csleep(c_ptr->m_idx, 0);
        m_ptr->ego_whip_ct = 5;
        m_ptr->ego_whip_pow = dam;
        p_ptr->redraw |= PR_HEALTH_BARS;
        return TRUE;
    case GF_PSI_BRAIN_SMASH: /* dam is the power of the effect (1-5) */
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags2 & RF2_EMPTY_MIND)
        {
            mon_lore_2(m_ptr, RF2_EMPTY_MIND);
            note = " is immune!";
            dam = 0;
        }
        else if (psion_mon_save_p(m_ptr->r_idx, dam))
        {
            note = " resists!";
            dam = 0;
        }
        else if (r_ptr->flags2 & RF2_WEIRD_MIND)
        {
            mon_lore_2(m_ptr, RF2_WEIRD_MIND);
            note = " resists somewhat.";
            dam /= 2;
            if (dam == 0) dam = 1;
        }
        if (dam)
        {
            note = " is blasted by psionic energy.";
            if (r_ptr->flags3 & RF3_NO_CONF)
                mon_lore_3(m_ptr, RF3_NO_CONF);
            else
                do_conf = 2*dam;
            if (r_ptr->flags3 & RF3_NO_STUN)
                mon_lore_3(m_ptr, RF3_NO_STUN);
            else
                do_stun = 2*dam;
            set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 2*dam);
            dam = 0;
        }
        break;
    case GF_MIND_BLAST:
        if (seen) obvious = TRUE;
        if (who == GF_WHO_PLAYER) msg_format("You gaze intently at %s.", m_name_object);
        _BABBLE_HACK()
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
             (r_ptr->flags3 & RF3_NO_CONF) ||
             (r_ptr->level > randint1((caster_lev - 10) < 1 ? 1 : (caster_lev - 10)) + 10))
        {
            if (r_ptr->flags3 & (RF3_NO_CONF))
                mon_lore_3(m_ptr, RF3_NO_CONF);
            note = " is unaffected!";
            dam = 0;
        }
        else if (r_ptr->flags2 & RF2_EMPTY_MIND)
        {
            mon_lore_2(m_ptr, RF2_EMPTY_MIND);
            note = " is immune!";
            dam = 0;
        }
        else if (r_ptr->flags2 & RF2_WEIRD_MIND)
        {
            mon_lore_2(m_ptr, RF2_WEIRD_MIND);
            note = " resists.";
            dam /= 3;
        }
        else
        {
            note = " is blasted by psionic energy.";
            note_dies = " collapses, a mindless husk.";

            if (who > 0) do_conf = randint0(4) + 4;
            else do_conf = randint0(8) + 8;
        }
        break;
    case GF_BRAIN_SMASH:
        if (seen) obvious = TRUE;
        if (who == GF_WHO_PLAYER) msg_format("You gaze intently at %s.", m_name_object);
        _BABBLE_HACK()
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            /* (r_ptr->flags3 & RF3_NO_CONF) || */
             (r_ptr->level > randint1((caster_lev - 10) < 1 ? 1 : (caster_lev - 10)) + 10))
        {
            if (r_ptr->flags3 & (RF3_NO_CONF))
                mon_lore_3(m_ptr, RF3_NO_CONF);
            note = " is unaffected!";
            dam = 0;
        }
        else if (r_ptr->flags2 & RF2_EMPTY_MIND)
        {
            mon_lore_2(m_ptr, RF2_EMPTY_MIND);
            note = " is immune!";
            dam = 0;
        }
        else if (r_ptr->flags2 & RF2_WEIRD_MIND)
        {
            mon_lore_2(m_ptr, RF2_WEIRD_MIND);
            note = " resists.";
            dam /= 3;
        }
        else
        {
            note = " is blasted by psionic energy.";
            note_dies = " collapses, a mindless husk.";

            if (who > 0)
            {
                do_conf = randint0(4) + 4;
                do_stun = randint0(4) + 4;
            }
            else
            {
                do_conf = randint0(8) + 8;
                do_stun = randint0(8) + 8;
            }
            set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 10);
            if (r_ptr->flags3 & RF3_NO_CONF) do_conf = 0;
        }
        break;
    case GF_CAUSE_1:
        if (seen) obvious = TRUE;
        if (!who) msg_format("You %s at %s and curse.", prace_is_(RACE_MON_BEHOLDER) ? "gaze" : "point", m_name);
        _BABBLE_HACK()
        if (randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
        {
            note = " is unaffected!";
            dam = 0;
        }
        break;
    case GF_CAUSE_2:
        if (seen) obvious = TRUE;
        if (!who) msg_format("You %s at %s and curse horribly.", prace_is_(RACE_MON_BEHOLDER) ? "gaze" : "point", m_name);
        _BABBLE_HACK()
        if (randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
        {
            note = " is unaffected!";
            dam = 0;
        }
        break;
    case GF_CAUSE_3:
        if (seen) obvious = TRUE;
        if (!who) msg_format("You point at %s, incanting terribly!", m_name);
        _BABBLE_HACK()
        if (randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
        {
            note = " is unaffected!";
            dam = 0;
        }
        break;
    case GF_CAUSE_4: {
        bool save = FALSE;
        if (seen) obvious = TRUE;
        if (!who) msg_format("You %s at %s and scream the word, 'DIE!'.", prace_is_(RACE_MON_BEHOLDER) ? "gaze" : "point", m_name);
        _BABBLE_HACK()
        if (who == GF_WHO_PLAYER)
        {
            save = p_ptr->current_r_idx != MON_KENSHIROU && mon_save_p(m_ptr->r_idx, A_WIS);
        }
        else
        {
            save = ((randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
                && ((who <= 0) || (caster_ptr->r_idx != MON_KENSHIROU)));
        }
        if (save)
        {
            note = " is unaffected!";
            dam = 0;
        }
        break; }
    case GF_HAND_DOOM:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            note = " is unaffected!";
            dam = 0;
        }
        else
        {
            if (randint1(dam) >= r_ptr->level + randint1(20))
            {
                dam = ((40 + randint1(20)) * m_ptr->hp) / 100;
                if (m_ptr->hp < dam) dam = m_ptr->hp - 1;
            }
            else
            {
                note = " resists!";
                dam = 0;
            }
        }
        break;
    case GF_CAPTURE: {
        int nokori_hp;
        if (!quests_allow_all_spells() && !is_pet(m_ptr))
        {
            msg_format("%^s is unaffected.", m_name);
            skipped = TRUE;
            break;
        }
        if ( (r_ptr->flags1 & RF1_UNIQUE)
          || (r_ptr->flags7 & RF7_NAZGUL)
          || (r_ptr->flags7 & RF7_UNIQUE2)
          || (m_ptr->mflag2 & MFLAG2_QUESTOR)
          || m_ptr->parent_m_idx )
        {
            msg_format("%^s is unaffected.", m_name);
            skipped = TRUE;
            break;
        }
        if (is_pet(m_ptr)) nokori_hp = m_ptr->maxhp * 4;
        else if ((p_ptr->pclass == CLASS_BEASTMASTER) && monster_living(r_ptr))
            nokori_hp = m_ptr->maxhp * 3 / 10;
        else if (p_ptr->easy_capture)
            nokori_hp = m_ptr->maxhp * 3 / 10;
        else if (warlock_is_(WARLOCK_DRAGONS) && (r_ptr->flags3 & RF3_DRAGON))
            nokori_hp = m_ptr->maxhp * 3 / 15;
        else
            nokori_hp = m_ptr->maxhp * 3 / 20;

        if (m_ptr->hp >= nokori_hp)
        {
            msg_format("You need to weaken %s more.", m_name);
            skipped = TRUE;
        }
        else if (m_ptr->hp < randint0(nokori_hp))
        {
            if (m_ptr->mflag2 & MFLAG2_CHAMELEON) choose_new_monster(c_ptr->m_idx, FALSE, MON_CHAMELEON);
            msg_format("You capture %^s!", m_name);
            quests_on_kill_mon(m_ptr);
            cap_mon = m_ptr->r_idx;
            cap_mspeed = m_ptr->mspeed;
            cap_hp = m_ptr->hp;
            cap_maxhp = m_ptr->max_maxhp;
            cap_nickname = m_ptr->nickname;
            if (c_ptr->m_idx == p_ptr->riding)
            {
                if (rakuba(-1, FALSE))
                {
                    msg_format("You have fallen from %s.", m_name);
                }
            }
            delete_monster_idx(c_ptr->m_idx);
            return TRUE;
        }
        else
        {
            msg_format("You failed to capture %s.", m_name);
            skipped = TRUE;
        }
        break; }
    case GF_ATTACK: /* dam = py_attack_mode */
        if (dam == BEHOLDER_GAZE && !los(m_ptr->fy, m_ptr->fx, py, px))
        {
            if (seen_msg) msg_format("%^s can't see you, and isn't affected!", m_name);
            skipped = TRUE;
            break;
        }

        /* Return this monster's death */
        return py_attack(where.y, where.x, dam);
    case GF_ENGETSU: {
        int effect = 0;
        bool done = TRUE;

        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (r_ptr->flags2 & RF2_EMPTY_MIND)
        {
            note = " is immune!";
            dam = 0;
            skipped = TRUE;
            mon_lore_2(m_ptr, RF2_EMPTY_MIND);
            break;
        }
        if (MON_CSLEEP(m_ptr))
        {
            note = " is immune!";
            dam = 0;
            skipped = TRUE;
            break;
        }

        if (one_in_(5)) effect = 1;
        else if (one_in_(4)) effect = 2;
        else if (one_in_(3)) effect = 3;
        else done = FALSE;

        if (effect == 1)
        {
            /* Powerful monsters can resist */
            if ((r_ptr->flags1 & RF1_UNIQUE) ||
                (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                note = " is unaffected!";

                obvious = FALSE;
            }

            /* Normal monsters slow down */
            else
            {
                if (set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
                {
                    note = " starts moving slower.";
                }
            }
        }

        else if (effect == 2)
        {
            do_stun = damroll((p_ptr->lev / 10) + 3 , (dam)) + 1;

            /* Attempt a saving throw */
            if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
                (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                /* Resist */
                do_stun = 0;

                /* No obvious effect */
                note = " is unaffected!";

                obvious = FALSE;
            }
        }

        else if (effect == 3)
        {
            /* Attempt a saving throw */
            if ((r_ptr->flags1 & RF1_UNIQUE) ||
                (r_ptr->flags3 & RF3_NO_SLEEP) ||
                (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                /* Memorize a flag */
                if (r_ptr->flags3 & RF3_NO_SLEEP)
                {
                    mon_lore_3(m_ptr, RF3_NO_SLEEP);
                }

                /* No obvious effect */
                note = " is unaffected!";

                obvious = FALSE;
            }
            else
            {
                /* Go to sleep (much) later */
                note = " falls asleep!";
                do_paralyzed = 5;
            }
        }

        if (!done)
        {
            note = " is immune!";
        }
        dam = 0;
        break; }
    case GF_ENTOMB: {
        int dir, x, y;

        if (is_pet(m_ptr) || is_friendly(m_ptr))
        {
            msg_print("Failed!");
            return FALSE;
        }

        for (dir = 0; dir < 8; dir++)
        {
            y = m_ptr->fy + ddy_ddd[dir];
            x = m_ptr->fx + ddx_ddd[dir];

            if (!cave_naked_bold(y, x)) continue;
            if (p_ptr->lev < 45)
                cave_set_feat(y, x, feat_rubble);
            else
                cave_set_feat(y, x, feat_granite);
        }
        return TRUE; }
    case GF_GENOCIDE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (genocide_aux(c_ptr->m_idx, dam, !who, (r_ptr->level + 1) / 2, "Genocide One"))
        {
            if (seen_msg)
            {
                if (dam == 666) /* Hack for Daemon flavored message */
                    msg_format("%^s is sent directly to hell!", m_name);
                else
                    msg_format("%^s disappeared!", m_name);
            }

            virtue_add(VIRTUE_VITALITY, -1);
            return TRUE;
        }
        skipped = TRUE;
        break;
    case GF_PHOTO:
        if (!who) msg_format("You take a photograph of %s.", m_name);
        /* Hurt by light */
        if (r_ptr->flags3 & (RF3_HURT_LITE))
        {
            /* Obvious effect */
            if (seen) obvious = TRUE;

            /* Memorize the effects */
            mon_lore_3(m_ptr, RF3_HURT_LITE);

            /* Special effect */
            note = " cringes from the light!";
            note_dies = " shrivels away in the light!";
        }
        /* Normally no damage */
        else dam = 0;
        photo = m_ptr->r_idx;
        break;
    case GF_BLOOD_CURSE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_CRUSADE: {
        bool success = FALSE;
        if (seen) obvious = TRUE;

        if ((r_ptr->flags3 & (RF3_GOOD)) && !p_ptr->inside_arena)
        {
            if (r_ptr->flags3 & (RF3_NO_CONF)) dam -= 50;
            if (dam < 1) dam = 1;

            /* No need to tame your pet */
            if (is_pet(m_ptr))
            {
                note = " starts moving faster.";

                (void)set_monster_fast(c_ptr->m_idx, MON_FAST(m_ptr) + 100);
                success = TRUE;
            }

            /* Attempt a saving throw */
            else if ((m_ptr->mflag2 & MFLAG2_QUESTOR) ||
                (r_ptr->flags1 & (RF1_UNIQUE)) ||
                (m_ptr->mflag2 & MFLAG2_NOPET) ||
                (p_ptr->cursed & OFC_AGGRAVATE) ||
                 ((r_ptr->level+10) > randint1(dam)))
            {
                /* Resist */
                if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
            }
            else
            {
                note = " is tamed!";

                set_pet(m_ptr);
                (void)set_monster_fast(c_ptr->m_idx, MON_FAST(m_ptr) + 100);

                /* Learn about type */
                mon_lore_3(m_ptr, RF3_GOOD);
                success = TRUE;
            }
        }

        if (!success)
        {
            if (!(r_ptr->flags3 & RF3_NO_FEAR))
            {
                do_fear = randint1(90)+10;
            }
            else
            {
                mon_lore_3(m_ptr, RF3_NO_FEAR);
            }
        }
        dam = 0;
        break; }
    case GF_WOUNDS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (randint0(50 + dam*3) < (20 + r_ptr->level))
        {
            note = " is unaffected!";
            dam = 0;
        }
        break;
    default:
        skipped = TRUE;
        dam = 0;
    }

    /* Absolutely no effect */
    if (skipped) return (FALSE);

    /* "Unique" monsters cannot be polymorphed */
    if (r_ptr->flags1 & (RF1_UNIQUE)) do_poly = FALSE;

    /* Quest monsters cannot be polymorphed */
    if (m_ptr->mflag2 & MFLAG2_QUESTOR) do_poly = FALSE;

    if (p_ptr->riding && (c_ptr->m_idx == p_ptr->riding)) do_poly = FALSE;

    /* "Unique" and "quest" monsters can only be "killed" by the player. */
    if (((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL) || (m_ptr->mflag2 & MFLAG2_QUESTOR))
      && !p_ptr->inside_battle
      && !prace_is_(RACE_MON_QUYLTHULG))
    {
        if (who && (dam > m_ptr->hp)) dam = m_ptr->hp;
    }

    if (!who && slept)
    {
        if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) virtue_add(VIRTUE_COMPASSION, -1);
        if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) virtue_add(VIRTUE_HONOUR, -1);
    }

    /* Modify the damage */
    tmp = dam;
    if (dam)
    {
        if (who > 0)
            dam = mon_damage_mod_mon(m_ptr, dam, type == GF_PSY_SPEAR);
        else
            dam = mon_damage_mod(m_ptr, dam, type == GF_PSY_SPEAR);
    }
    if (tmp > 0 && dam == 0)
        note = " is unharmed.";

    /* Check for death */
    if (dam > m_ptr->hp)
    {
        /* Extract method of death */
        note = note_dies;
    }
    else
    {
        /* Sound and Impact resisters never stun */
        if (do_stun &&
            !(r_ptr->flagsr & (RFR_RES_SOUN | RFR_RES_WALL)) &&
            !(r_ptr->flags3 & RF3_NO_STUN))
        {
            /* Obvious */
            if (seen) obvious = TRUE;

            /* Get stunned */
            if (MON_STUNNED(m_ptr))
            {
                note = " is <color:B>more dazed</color>.";
                tmp = MON_STUNNED(m_ptr) + (do_stun / 2);
            }
            else
            {
                note = " is <color:B>dazed</color>.";
                tmp = do_stun;
            }

            /* Apply stun */
            (void)set_monster_stunned(c_ptr->m_idx, tmp);

            /* Get angry */
            get_angry = TRUE;
        }

        /* Confusion and Chaos resisters (and sleepers) never confuse */
        if (do_conf && (type == GF_ELDRITCH_CONFUSE || (
             !(r_ptr->flags3 & RF3_NO_CONF) &&
             !(r_ptr->flagsr & RFR_EFF_RES_CHAO_MASK))))
        {
            /* Obvious */
            if (seen) obvious = TRUE;

            /* Already partially confused */
            if (MON_CONFUSED(m_ptr))
            {
                note = " looks more confused.";
                tmp = MON_CONFUSED(m_ptr) + (do_conf / 2);
            }

            /* Was not confused */
            else
            {
                note = " looks confused.";
                tmp = do_conf;
            }

            /* Apply confusion */
            (void)set_monster_confused(c_ptr->m_idx, tmp);

            /* Get angry */
            get_angry = TRUE;
        }

        if (do_time)
        {
            /* Obvious */
            if (seen) obvious = TRUE;

            if (do_time >= m_ptr->maxhp) do_time = m_ptr->maxhp - 1;

            if (do_time)
            {
                if (flags & GF_DAMAGE_SPELL)
                    note = " seems weakened.";
                m_ptr->maxhp -= do_time;
                if ((m_ptr->hp - dam) > m_ptr->maxhp) dam = m_ptr->hp - m_ptr->maxhp;
            }
            get_angry = TRUE;
        }

        /* Mega-Hack -- Handle "polymorph" -- monsters get a saving throw */
        if (do_poly && (randint1(90) > r_ptr->level))
        {
            if (polymorph_monster(where.y, where.x))
            {
                /* Obvious */
                if (seen) obvious = TRUE;
                /* Why, in the name of all sanity, would you do this??!
                 * dam = 0;*/
            }
            else
            {
                /* No polymorph */
                note = " is unaffected!";
            }

            /* Hack -- Get new monster */
            m_ptr = &m_list[c_ptr->m_idx];

            /* Hack -- Get new race */
            r_ptr = &r_info[m_ptr->r_idx];
        }

        /* Handle "teleport" ... double check the Guardian. It should be
         * handled above for nicer messaging, but I didn't get all the cases. */
        if (do_dist && !(m_ptr->smart & (1U << SM_GUARDIAN)))
        {
            /* Obvious */
            if (seen) obvious = TRUE;

            /* Message */
            note = " disappears!";
            if (!who) virtue_add(VIRTUE_VALOUR, -1);

            /* Teleport */
            teleport_away(c_ptr->m_idx, do_dist,
                        (!who ? TELEPORT_DEC_VALOUR : 0L) | TELEPORT_PASSIVE);

            /* Hack -- get new location */
            where.y = m_ptr->fy;
            where.x = m_ptr->fx;

            /* Hack -- get new grid */
            c_ptr = &cave[where.y][where.x];
        }

        /* Fear */
        if (do_fear)
        {
            /* Set fear */
            (void)set_monster_monfear(c_ptr->m_idx, MON_MONFEAR(m_ptr) + do_fear);

            /* Get angry */
            get_angry = TRUE;
        }
    }

    if (type == GF_DRAIN_MANA)
    {
        /* Drain mana does nothing */
    }
    /* If another monster did the damage, hurt the monster by hand */
    else if (who)
    {
        /* Redraw (later) if needed */
        check_mon_health_redraw(c_ptr->m_idx);

        /* Wake the monster up */
        (void)set_monster_csleep(c_ptr->m_idx, 0);

        /* Hurt the monster */
        m_ptr->hp -= dam;

        /* Dead monster */
        if (m_ptr->hp < 0)
        {
            bool sad = FALSE;

            if (is_pet(m_ptr) && !(m_ptr->ml))
                sad = TRUE;

            /* Give detailed messages if destroyed */
            if (known && note)
            {
                monster_desc(m_name, m_ptr, MD_TRUE_NAME);
                msg_format("%^s%s", m_name, note);
            }

            if (who > 0) monster_gain_exp(who, m_ptr->r_idx);

            mon_check_kill_unique(c_ptr->m_idx);

            /* Generate treasure, etc */
            monster_death(c_ptr->m_idx, who_is_pet);

            /* Delete the monster */
            delete_monster_idx(c_ptr->m_idx);

            if (sad)
            {
                msg_print("You feel sad for a moment.");
            }
        }

        /* Damaged monster */
        else
        {
            /* Give detailed messages if visible or destroyed */
            if (note && seen_msg) msg_format("%^s%s", m_name, note);

            /* Hack -- Pain message */
            else
            {
                mon_fight = TRUE;
            }

            /* Hack -- handle sleep */
            if (do_sleep) (void)set_monster_csleep(c_ptr->m_idx, do_sleep);
            if (do_paralyzed && !m_ptr->paralyzed)
                set_monster_paralyzed(c_ptr->m_idx, do_paralyzed);
        }
    }
    else if (heal_leper)
    {
        if (seen_msg) msg_print("The Mangy looking leper is healed!");
        delete_monster_idx(c_ptr->m_idx);
    }
    /* If the player did it, give him experience, check fear */
    else
    {
        bool fear = FALSE;

        /* Hacks  ... these effects probably belong in the gargantuan switch statement above ... sigh */
        /* Hack: The Draining Blast power gives hitpoints back. */
        if (type == GF_ELDRITCH_DRAIN && monster_living(r_ptr))
        {
            int heal = dam;
            if (heal > m_ptr->hp)
                heal = m_ptr->hp;
            heal /= 2;
            hp_player(heal);
        }

        /* Hurt the monster, check for fear and death
              v---- Revert 525c2ace: Warlocks: Playtesting Dragon Pact. Massive problems with project()
                    The problem here is that many attacks, like Slow Monster, do no physical damage, but
                    rely on mon_take_hit to do other things, like wake up sleeping monsters (or reveal
                    player ring mimics). This code needs refactoring ...*/
        if (/*dam &&*/ mon_take_hit(c_ptr->m_idx, dam, &fear, note_dies))
        {
            /* Dead monster */
        }

        /* Damaged monster */
        else
        {
            if (note == note_dies) /* Hack around crap code design ... Above we assumed monster would die but alas, we were wrong! */
                note = NULL;

            /* HACK - anger the monster before showing the sleep message */
            if (do_sleep) anger_monster(m_ptr);

            /* HACK - tick off smart monsters whenever damaged by the player
               from a distance. */
            if (who == 0 && dam > 0 && m_ptr->cdis > 1)
            {
                bool splashed = !projectable(py, px, where.y, where.x);
                if (allow_ticked_off(r_ptr))
                {
                    if (!mut_present(MUT_SUBTLE_CASTING))
                        m_ptr->anger = MIN(100, m_ptr->anger + 10 + m_ptr->anger / 2); 
                    if (splashed)
                        m_ptr->anger = MIN(100, m_ptr->anger + 10 + m_ptr->anger / 2); 
                    /* Attempt to deal with Dungeon Guardians splash exploit.
                       Dungeon guardians use AI_GUARD_POS, so cannot be lured
                       away from the dungeon entrance. Attempting this exploit
                       makes them really mad, and if they are mad enough, then
                       they will actually pursue the player (cf get_moves in melee2.c) */
                    if (splashed && m_ptr->cdis > MAX_RANGE)
                    {
                        pack_info_t *pack_ptr = pack_info_ptr(c_ptr->m_idx);
                        if (pack_ptr && pack_ptr->ai == AI_GUARD_POS)
                            m_ptr->anger = 100;
                        else
                            m_ptr->anger = MIN(100, m_ptr->anger + 10 + m_ptr->anger / 2); 
                    }
                }
                /* Splashing Uniques out of LOS makes them rethink their approach */
                if (splashed && (r_ptr->flags1 & RF1_UNIQUE))
                {
                    pack_info_t *pack_ptr = pack_info_ptr(c_ptr->m_idx);

                    if (pack_ptr)
                    {
                        int odds = MAX(1, 7 - m_ptr->anger/10);

                        if (!allow_ticked_off(r_ptr)) /* Paranoia ... These should already be seeking! */
                            odds = 1;

                        switch (pack_ptr->ai)
                        {
                        case AI_MAINTAIN_DISTANCE:
                            if (one_in_(odds))
                                pack_ptr->ai = AI_SEEK;
                            else
                                pack_ptr->distance += 2;
                            break;

                        case AI_SHOOT:
                        case AI_LURE:
                            if (one_in_(odds))
                                pack_ptr->ai = AI_SEEK;
                            break;
                        }
                    }
                }
            }

            /* Give detailed messages if visible or destroyed */
            if (note && seen_msg)
                msg_format("%^s%s", m_name, note);

            /* Hack -- Pain message */
            else if (known && dam && !(flags & GF_DAMAGE_SPELL))
            {
                message_pain(c_ptr->m_idx, dam);
            }

            /* Anger monsters */
            if (((dam > 0) || get_angry) && !do_sleep)
                anger_monster(m_ptr);

            /* Take note */
            if ((fear || do_fear) && seen)
            {
                /* Sound */
                sound(SOUND_FLEE);

                /* Message */
                msg_format("%^s flees in terror!", m_name);
            }

            /* Hack -- handle sleep */
            if (do_sleep) (void)set_monster_csleep(c_ptr->m_idx, do_sleep);
            if (do_paralyzed && !m_ptr->paralyzed)
                set_monster_paralyzed(c_ptr->m_idx, do_paralyzed);
        }
    }

    if ((type == GF_BLOOD_CURSE) && one_in_(4))
    {
        int curse_flg = (PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);
        int count = 0;
        do
        {
            switch (randint1(28))
            {
            case 1: case 2:
                if (!count)
                {
                    msg_print("The ground trembles...");

                    earthquake(ty, tx, 4 + randint0(4));
                    if (!one_in_(6)) break;
                }
            case 3: case 4: case 5: case 6: case 7: case 8:
                if (!count)
                {
                    int dam = damroll(10, 10);
                    msg_print("A portal opens to a plane of raw mana!");

                    project(0, 8, ty,tx, dam, GF_MANA, curse_flg, -1);
                    if (!one_in_(6)) break;
                }
            case 9: case 10: case 11:
                if (!count)
                {
                    msg_print("Space warps about it!");

                    if (m_ptr->r_idx) teleport_away(c_ptr->m_idx, damroll(10, 10), TELEPORT_PASSIVE);
                    if (one_in_(13)) count += activate_hi_summon(ty, tx, TRUE);
                    if (!one_in_(6)) break;
                }
            case 12: case 13: case 14: case 15: case 16:
                msg_print("It feels a surge of energy!");

                project(0, 7, ty, tx, 50, GF_DISINTEGRATE, curse_flg, -1);
                if (!one_in_(6)) break;
            case 17: case 18: case 19:
                aggravate_monsters(0);
                if (!one_in_(6)) break;
            case 20: case 21:
                count += activate_hi_summon(ty, tx, TRUE);
                if (!one_in_(6)) break;
            case 22: case 23: case 24: case 25: case 26:
            {
                bool pet = !one_in_(3);
                u32b mode = PM_ALLOW_GROUP;

                if (pet) mode |= PM_FORCE_PET;
                else mode |= (PM_NO_PET | PM_FORCE_FRIENDLY);

                count += summon_specific((pet ? -1 : 0), py, px, (pet ? p_ptr->lev*2/3+randint1(p_ptr->lev/2) : dun_level), 0, mode);
                if (!one_in_(6)) break;
            }
            case 27:
                if (p_ptr->hold_life && (randint0(100) < 75)) break;
                msg_print("You feel your life draining away...");

                if (p_ptr->hold_life) lose_exp(p_ptr->exp / 160);
                else lose_exp(p_ptr->exp / 16);
                if (!one_in_(6)) break;
            case 28:
            {
                int i = 0;
                if (one_in_(13))
                {
                    while (i < 6)
                    {
                        do
                        {
                            (void)do_dec_stat(i);
                        }
                        while (one_in_(2));

                        i++;
                    }
                }
                else
                {
                    (void)do_dec_stat(randint0(6));
                }
                break;
            }
            }
        }
        while (one_in_(5));
    }

    if (p_ptr->inside_battle)
    {
        p_ptr->health_who = c_ptr->m_idx;
        p_ptr->redraw |= PR_HEALTH_BARS;
        redraw_stuff();
    }

    /* XXX XXX XXX Verify this code */

    /* Update the monster */
    if (m_ptr->r_idx) update_mon(c_ptr->m_idx, FALSE);

    /* Redraw the monster grid */
    lite_spot(where.y, where.x);


    /* Update monster recall window */
    if ((p_ptr->monster_race_idx == m_ptr->r_idx) && (seen || !m_ptr->r_idx))
    {
        /* Window stuff */
        p_ptr->window |= (PW_MONSTER);
    }

    if ((dam > 0) && !is_pet(m_ptr) && !is_friendly(m_ptr))
    {
        if (!who)
        {
            if (!(flags & PROJECT_NO_HANGEKI))
            {
                set_target(m_ptr, monster_target_y, monster_target_x);
            }
        }
        else if ((who > 0) && is_pet(caster_ptr) && !player_bold(m_ptr->target_y, m_ptr->target_x))
        {
            set_target(m_ptr, caster_ptr->fy, caster_ptr->fx);
        }
    }

    if (p_ptr->riding && (p_ptr->riding == c_ptr->m_idx) && (dam > 0))
    {
        if (m_ptr->hp > m_ptr->maxhp/3) dam = (dam + 1) / 2;
        rakubadam_m = (dam > 200) ? 200 : dam;
    }

    if (photo)
    {
        object_type *q_ptr;
        object_type forge;

        /* Get local object */
        q_ptr = &forge;

        /* Prepare to make a Blade of Chaos */
        object_prep(q_ptr, lookup_kind(TV_STATUE, SV_PHOTO));

        q_ptr->pval = photo;

        /* Mark the item as fully known */
        q_ptr->ident |= (IDENT_KNOWN);

        /* Drop it in the dungeon */
        (void)drop_near(q_ptr, -1, py, px);
    }

    /* Return "Anything seen?" */
    return (obvious);
}
