/*
 * gameplay/panel.h
 *
 * All the control panels and the stats area.
 *
 */

#pragma once

#include "pan_info.h"

#include "../tribe.h"

#include "../../api/element.h"
#include "../../api/label.h"
#include "../../api/button/b_bitmap.h"
#include "../../api/button/b_text.h"
#include "../../api/button/b_skill.h"
#include "../../api/button/b_spawn.h"

struct GameplayPanel : public Api::Element {

public:

    enum Speed {
        SPEED_NORMAL,
        SPEED_FAST,
        SPEED_TURBO,
        SPEED_PAUSE
    };

    typedef Api::BitmapButton              BiB;
    typedef Api::SkillButton               SkB;
    typedef std::vector <Api::SkillButton> SkBVec;
    typedef SkBVec::iterator               SkBIt;

    SkBVec skill;

    Api::SpawnIntervalButton spawnint_slow;
    Api::SpawnIntervalButton spawnint_cur;

    Api::BitmapButton spawnint_fixed;
    Api::BitmapButton pause;
    Api::BitmapButton zoom;
    Api::TwoTasksButton speed_back;
    Api::TwoTasksButton speed_ahead;
    Api::TwoTasksButton speed_fast;
    Api::BitmapButton state_save;
    Api::BitmapButton state_load;
    Api::BitmapButton restart;
    Api::BitmapButton nuke_single;
    Api::BitmapButton nuke_multi;

    Api::TextButton spec_tribe;

    GameplayStats stats;

    Api::Label rate_fixed;

    void (*on_hint_change)(void*, const int); // int = new hint number
    void* on_hint_change_where; // first arg to give to above function

    ///////////////////////////////////////////////////////////////////////////

    GameplayPanel();
    virtual ~GameplayPanel() {}

    void set_gapamode_and_hints(GapaMode, const int); // hint size

    inline GapaMode get_gapamode() { return gapamode;}

    void  set_speed(Speed);
    Speed get_speed();

    SkBIt button_by_skill(const LixEn::Ac);

    void set_like_tribe(const Tribe*);
    void set_skill_on  (const LixEn::Ac);

           void set_hint_cur (const int);
    inline int  get_hint_cur () const          { return hint_cur;           }
    inline bool get_nuke_doubleclicked() const { return nuke_doubleclicked; }

    inline void suggest_tooltip_scrolling()   { tooltip_scrolling   = true; }
    inline void suggest_tooltip_force_dir()   { tooltip_force_dir   = true; }
    inline void suggest_tooltip_priority()    { tooltip_priority    = true; }
    inline void suggest_tooltip_builders()    { tooltip_builders    = true; }
    inline void suggest_tooltip_platformers() { tooltip_platformers = true; }

protected:

    virtual void calc_self();
    virtual void draw_self();

private:

    GapaMode gapamode;

    bool nuke_doubleclicked;
    int  timer_tick_nuke_single;

    bool tooltip_scrolling;
    bool tooltip_force_dir;
    bool tooltip_priority;
    bool tooltip_builders;
    bool tooltip_platformers;

    int hint_size; // including the tutorial hint
    int hint_cur;

    BiB hint_big,
        hint_plus,
        hint_minus;

    LixEn::Ac skill_last_set_on; // to reactivate after loading a state

};
