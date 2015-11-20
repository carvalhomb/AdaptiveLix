/*
 * gameplay/gamepl_a.cpp
 *
 * Dinge, die bei einem aktiven Tick des Gameplays kontrolliert werden.
 * "Aktiv" heisst hierbei, dass kein Replay abgespielt wird, sondern spiel-
 * mechanisch relevante Dinge der Steuerung bearbeitet werden.
 *
 */

#include "gameplay.h"
#include "../other/user.h"

#include "../network/gameevents.h"
#include "../network/gamedata.h"

Replay::Data Gameplay::new_replay_data()
{
    Replay::Data data;
    data.player = malo->number;
    data.update = cs.update + 1;
    return data;
}



void Gameplay::calc_active()
{
    if (!malo) return;

    // Wo ist die Maus auf dem Land?
    int mx = map.get_mouse_x();
    int my = map.get_mouse_y();

    const bool mouse_on_panel = hardware.get_my() > map.get_screen_yl();

    // Remedies the bug: displays "N nothings" after level restart
    pan.stats.set_tarcnt(0);

    // Spawn interval buttons
    if (cs.tribes.size() == 1) {
        Tribe& tribe = cs.tribes[0];

        int cur = pan.spawnint_cur .get_spawnint();

        if (cur == tribe.spawnint_slow)
            ;
        else if (pan.spawnint_slow.get_execute_right())
            pan.spawnint_cur.set_spawnint(tribe.spawnint_slow);
        else if (pan.spawnint_slow.get_execute_left())
            pan.spawnint_cur.set_spawnint(cur + 1);

        if (cur == tribe.spawnint_fast)
            ;
        else if (pan.spawnint_cur .get_execute_right())
            pan.spawnint_cur.set_spawnint(tribe.spawnint_fast);
        else if (pan.spawnint_cur.get_execute_left())
            pan.spawnint_cur.set_spawnint(cur - 1);
    }

    // Selection of skills in the panel aren't checked here anymore.
    // They aren't replay data, so that check went into gamepl_c.cpp.

    // Atombombe
    if (pan.get_nuke_doubleclicked()) {
        // set_on() kommt zwar auch im Update, aber wenn wir das
        // hier immer machen, sieht es besser aus. Gleiches gilt fuer
        // den Sound, ist wie beim normalen Anklicken.
        pan.nuke_single.set_on();
        pan.nuke_multi .set_on();
        pan.pause      .set_off();
        Replay::Data data = new_replay_data();
        data.action       = Replay::NUKE;

        GameData event_data = GameData("", level);
        event_data.load_replay_data(data);
        GameEvents::send_event(event_data);

        replay.add(data);
        Network::send_replay_data(data);
        effect.add_sound(cs.update + 1, *trlo, 0, Sound::NUKE);
        Sound::play_loud(Sound::NUKE);
    }



    ///////////////////////////////////////
    // End of: check things in the panel //
    // Now: mouse on the playing field   //
    ///////////////////////////////////////



    // mouse on the playing field, lixes are selectable
    if (! mouse_on_panel && trlo) {
        // Bestimmte Richtung anwahlen?
        bool only_dir_l = false;
        bool only_dir_r = false;
        if (  key[useR->key_force_left]
         && ! key[useR->key_force_right]) {
            only_dir_l = true;
            mouse_cursor.set_x_frame(1);
        }
        else if (! key[useR->key_force_left]
         &&        key[useR->key_force_right]) {
            only_dir_r = true;
            mouse_cursor.set_x_frame(2);
        }
        // Decide which lix the cursor points at
        // Die Liste der Lixen wird durchlaufen und die Priorit�t jeder
        // Lix errechnet. Wird eine h�here Priorit�t als die derzeitig
        // h�chste gefunden, wechselt LixIt target. Bei gleicher Prioritaet
        // haben Lixen, die naeher am Mauscursor liegen, Vorrang! Mit rechter
        // Maustaste (selectable in the options) waehlt man dagegen die letzte
        // Lix mit der niedrigsten Priorit�t. Auch hier haben naeher liegende
        // Lixen Vorrang.
        LixIt  target = trlo->lixvec.end(); // Klickbar mit Prioritaet
        LixIt  tarinf = trlo->lixvec.end(); // Nicht unb. klickbar mit Prior.
        int    tarcnt = 0; // Anzahl Lixen unter Cursor
        int    target_priority = 0;
        int    target_prio_min = 100000; // if (< target_prio) => tooltip
        int    tarinf_priority = 0;
        double target_hypot = 1000;
        double tarinf_hypot = 1000;
        bool   tooltip_l_elig = false; // if both true => tooltip
        bool   tooltip_r_elig = false; // is there a l/r walker considered?

        // Bei Zoom diese Variablen veraendern
        int zoom   = map.get_zoom();
        int mmld_x = mouse_max_lix_distance_x - mouse_cursor_offset/2*zoom;
        int mmld_u = mouse_max_lix_distance_u - mouse_cursor_offset/2*zoom;
        int mmld_d = mouse_max_lix_distance_d - mouse_cursor_offset/2*zoom;

        // find current skill of the local player via the GUI
        GameplayPanel::SkBIt skill_visible = pan.skill.begin();
        while (skill_visible != pan.skill.end()
            && (! skill_visible->get_on()
                // panel has reordered skills. trlo and the replay don't
                || trlo->skills.find(skill_visible->get_skill()) ==
                   trlo->skills.end()
                || trlo->skills[skill_visible->get_skill()] == 0
                || skill_visible->get_number() == 0))
            ++skill_visible;

        if (skill_visible != pan.skill.end())
            for (LixIt i =  --trlo->lixvec.end();
                       i != --trlo->lixvec.begin(); --i)
        {
            // (skill_visible) is now a skill with (trlo has != 0 of it).
            // In particular, it appears in trlo->skills as a key.

            if (   map.distance_x(i->get_ex(), mx) <=  mmld_x
                && map.distance_x(i->get_ex(), mx) >= -mmld_x
                && map.distance_y(i->get_ey(), my) <=  mmld_d
                && map.distance_y(i->get_ey(), my) >= -mmld_u
            ) {
                // Hypot geht von (ex|ey+etwas) aus
                // true = Beachte persoenliche Einschraenkungen wie !MultBuild
                int priority
                    = i->get_priority(skill_visible->get_skill(), true);

                // Invert priority if a corresponding mouse button is held
                if ((hardware.get_mrh() && useR->prioinv_right)
                 || (hardware.get_mmh() && useR->prioinv_middle)
                 ||  hardware.key_hold(useR->key_priority)) {
                    priority = 100000 - priority;
                }
                double hypot = map.hypot(mx, my, i->get_ex(),
                                          i->get_ey() + ((mmld_d - mmld_u)/2)
                                          );
                if (priority > 0 && priority < 100000) {
// This is horrible code. 9 indentation levels imply a severe problem,
// we should use more functions. Will fix this in the D port.

// Die Anforderungen den offenen Mauscursur
// und das Schreiben des Strings auf die Info...
++tarcnt;
if (priority >  tarinf_priority
 ||(priority == tarinf_priority && hypot < tarinf_hypot)) {
    tarinf = i;
    tarinf_priority = priority;
    tarinf_hypot    = hypot;
}
// ...sind geringer als die f�r Anklick-Inbetrachtnahme!
if (priority > 1 && priority < 99999) {
    if (!(only_dir_l && i->get_dir() ==  1)
     && !(only_dir_r && i->get_dir() == -1)) {
        // consider this clickable and eligible for tooltip
        if  (priority >  target_priority
         || (priority == target_priority && hypot < target_hypot)) {
            if (target_priority != 0
             && target_priority < priority)
                pan.suggest_tooltip_priority();
            target          = i;
            target_priority = priority;
            target_hypot    = hypot;
            if (priority < target_prio_min)
                target_prio_min = priority;
        }
        else if (priority <  target_prio_min
             || (priority == target_prio_min && hypot >= target_hypot)) {
            if (target_prio_min != 100000
             && target_prio_min > priority)
                pan.suggest_tooltip_priority();
            target_prio_min = priority;
        }
    }
    if (i->get_dir() ==  1) tooltip_r_elig = true;
    if (i->get_dir() == -1) tooltip_l_elig = true;
    if (tooltip_r_elig && tooltip_l_elig)
        pan.suggest_tooltip_force_dir();
}
                }
            }
        }

        // Auswertung von tarinf
        if (tarinf != trlo->lixvec.end()) {
            mouse_cursor.set_y_frame(1);
        }
        pan.stats.set_tarinf(tarinf == trlo->lixvec.end() ? 0 : &*tarinf);
        pan.stats.set_tarcnt(tarcnt);

        // tooltips for queuing builders/platformers
        if (target != trlo->lixvec.end()
         && skill_visible->get_number() != 0) {
            if (target->get_ac() == LixEn::BUILDER
             && skill_visible->get_skill() == LixEn::BUILDER)
                pan.suggest_tooltip_builders();
            else if (target->get_ac() == LixEn::PLATFORMER
             && skill_visible->get_skill() == LixEn::PLATFORMER)
                pan.suggest_tooltip_platformers();
        }

        // Resolving target
        // target == trlo->lixvec.end() if there was nobody under the cursor,
        // or if skill_visible is somehow pan.skill.end(), shouldn't happen
        // we're also checking the displayed number, look at comment about the
        // visible number due to eye candy/making button seem more responsive
        if (target != trlo->lixvec.end() && hardware.get_ml()) {
            if (skill_visible->get_number() != 0) {

                const int lem_id = target - trlo->lixvec.begin();

                // put sound into effect manager, so that it's not played
                // again when the next update is computed
                Sound::Id snd = Lixxie::get_ac_func(skill_visible
                                ->get_skill()).sound_assign;
                effect.add_sound(cs.update + 1, *trlo, lem_id, snd);

                // Die sichtbare Zahl hinabsetzen geschieht nur fuer's Auge,
                // in Wirklichkeit geschieht dies erst beim Update. Das Augen-
                // spielzeug verabreichen wir allerdings nur, wenn nicht z.B.
                // zweimal auf dieselbe Lix mit derselben Faehigkeit
                // geklickt wurde.
                if (!replay.get_on_update_lix_clicked(
                    cs.update + 1, lem_id, skill_visible->get_skill())
                    && skill_visible->get_number() != LixEn::infinity
                ) {
                    skill_visible->set_number(skill_visible->get_number() - 1);
                    Sound::play_loud(snd);
                }

                // assign
                pan.pause.set_off();

                Replay::Data data = new_replay_data();
                data.action       = only_dir_l ? Replay::ASSIGN_LEFT
                                  : only_dir_r ? Replay::ASSIGN_RIGHT
                                  : Replay::ASSIGN;
                data.skill        = skill_visible->get_skill();
                data.what         = lem_id;

                GameData event_data = GameData("", level);
                event_data.load_replay_data(data);
                GameEvents::send_event(event_data);

                replay.add(data);
                Network::send_replay_data(data);
            }
            else {
                Sound::play_loud(Sound::PANEL_EMPTY);
            }
        }

    }
    // end of: mouse in the playing field, no aiming

}
