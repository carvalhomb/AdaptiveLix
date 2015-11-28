/*
 * gameplay/gamepl_u.cpp
 *
 * Ein Update, also eine spielmechanische Zeiteinheit, in der sich
 * beispielsweise die Lixen bewegen
 *
 */

#include "gameplay.h"

#include "../graphic/sound.h"
#include "../other/user.h"

#include "../exposer/gamedata.h"
#include "../exposer/exposer.h"
//#include "../other/file/log.h"


void Gameplay::update()
{
    // Noch schnell die Replaydaten mit der eingestellten Rate fertig machen:
    // Siehe Ratenbutton-Calculate fuer Kommentar, warum dies hier passiert.
	//
	// Yet quickly make the Replay data ready at the set rate:
	// See installments Button Calculate for comment as to why this happened here.
	//
    if (trlo && !replaying &&
        pan.spawnint_cur.get_spawnint() != trlo->spawnint
    ) {
        trlo->spawnint    = pan.spawnint_cur.get_spawnint();
        Replay::Data data = new_replay_data();
        data.action       = Replay::SPAWNINT;
        data.what         = trlo->spawnint;

        GameData event_data = GameData("", level);
        event_data.load_replay_data(data);
        Exposer exposer = Exposer(event_data, gloB->notification_center);
        exposer.run();


        replay.add(data);
        Network::send_replay_data(data);
    }

    // Im Netzwerkspiel den Netzwerk-Replaydaten-Puffer nach neuen Ereignissen
    // der anderen Spieler kontrollieren - die eigenen sind schon im Replay.
    // In the network game network Replay data buffer for new events of the
    // other players control - their own are already in the replay.
    if (Network::get_started()) {
        Replay::Vec netdata = Network::get_replay_data();
        replay.add(netdata);

        replay_recalc_from = cs.update;
        for (Replay::ConstIt i = netdata.begin(); i != netdata.end(); ++i) {
            // Das folgende <= statt <  behebt den lange quaelenden Netzwerkbug
            // und ist sogar theoretisch korrekt: Wenn wir in Update N sind und
            // es kommt Data zu Upd. N, so muss N-1 -> N neu berechnet werden.
        	// The following <= instead of <resolves the long torturous network
        	// Bug and is even theoretically correct: If we are in Update N and
        	// it comes to Data Upd. N, N-1 so must -> N be recalculated.
            if (i->update <= replay_recalc_from) {
                // recalc_from ist inklusive: Es wird ein Stand geladen,
                // der dieses Update noch nicht mitberechnet hat.
            	// recalc_from is including: loading a state, which has
            	// not yet included in the calculation of this update.
                replay_recalc_need = true;
                replay_recalc_from = i->update;
            }
        }
        Network::clear_replay_data();
    }

    // Updaten, entweder mehrmals oder nur einmal
    // Updating, either repeatedly or only once
    if (replay_recalc_need) {
        GameState state = state_manager.get_auto(replay_recalc_from);
        if (state) {
            // Wir nutzen nicht load_state(), weil dies kein manuelles Laden
            // ist. Dies sollte das einizge nicht-manuelle Laden sein.
        	// We do not use load_state (), because this is not a manual load.
        	// This should be the einizge non-manual loading.
            unsigned long updates_to_calc_to = cs.update;
            cs = state;
            while (cs.update < updates_to_calc_to) update_cs_once();
        }
        replay_recalc_need = false;
        replay_recalc_from = 0;
    }
    else update_cs_once();

    finalize_update_and_animate_gadgets();
}
// Ende des cs.update inkl. Neuladerei und Nachberechnung
// End of cs.update incl. Neuladerei and recalculation



void Gameplay::finalize_update_and_animate_gadgets()
{
    // Diese Dinge muessen nicht mehrfach gemacht werden, selbst wenn neu
    // geladen wird, weil Netzwerkpakete eintreffen.
	// These things must not be done more than once, even if reloads because network packets arrive.
    for (IacIt i =  special[Object::DECO  ].begin();
               i != special[Object::DECO  ].end(); ++i) i->animate();
    for (IacIt i =  special[Object::WATER ].begin();
               i != special[Object::WATER ].end(); ++i) i->animate();
    for (IacIt i =  special[Object::ONEWAY].begin();
               i != special[Object::ONEWAY].end(); ++i) i->animate();
    for (Goal::It i = goal.begin();    i != goal.end();    ++i) i->animate();
    for (HatchIt  i = hatches.begin(); i != hatches.end(); ++i)
     i->animate(effect, cs.update);

    if (trlo)
        // To counter leftover misinformation after Player::return_skills
        pan.set_like_tribe(trlo);

    timer_tick_last_update = Help::timer_ticks;
}



// ############################################################################
// ############################################################ go_back_updates
// ############################################################################



void Gameplay::go_back_updates(const int go_back_by)
{
    if (go_back_by <= 0)
        return;

    unsigned long target_upd = 0;
    if (static_cast <unsigned long> (go_back_by) < cs.update)
        target_upd = cs.update - go_back_by;

    load_state(state_manager.get_auto(target_upd + 1));
    while (cs.update < target_upd)
        update_cs_once();

    finalize_update_and_animate_gadgets();
}



// ############################################################################
// ############################################################# update_cs_once
// ############################################################################



// upd ist, welches Update berechnet wird. Das Spiel beginnt etwa mit Update 0
// und im ersten zu errechnenden Update setzt diese Funktion das auf 1.
//
// is upd which update is calculated. The game begins with about update 0
// and the first to be calculated update this function sets to 1.
//
void Gameplay::update_cs_once()
{
    // Neues Update einleiten. Initiate New Update
    ++cs.update;
    const Ulng& upd = cs.update;

    Replay::Vec data = replay.get_data_for_update(upd);

    // Erst den ersten Spieler, dann den zweiten usw. pro Update,
    // damit aequivalente Replays mit verschiedener Spielerreihenfolge pro
    // Update immer gleich verlaufen.
    // Only the first player, then the second and so on per update,
    // so equivalents replays with different order of play per update always run the same.
    for (Tribe::It tritr = cs.tribes.begin(); tritr!=cs.tribes.end();++tritr) {
        for (Replay::It i = data.begin(); i != data.end(); ++i) {
            std::list <Tribe::Master>::iterator mitr = tritr->masters.begin();
            while (mitr != tritr->masters.end()
             && mitr->number != i->player) ++mitr;
            if (mitr != tritr->masters.end())
             update_cs_one_data(*tritr, &*mitr, i);
        }
    }



    // Kurzer Einschub: Uhrendinge. Short slot: clocks things.
    if (level.seconds > 0) {
        if (cs.clock_running && cs.clock > 0) --cs.clock;
        // Im Multiplayer:
        // Nuke durch die letzten Sekunden der Uhr. Dies loest
        // kein Netzwerk-Paket aus! Alle Spieler werden jeweils lokal genukt.
        // Dies fuehrt dennoch bei allen zum gleichen Spielverlauf, da jeder
        // Spieler das Zeitsetzungs-Paket zum gleichen Update erhalten.
        // Wir muessen dies nach dem Replayauswerten machen, um festzustellen,
        // dass noch kein Nuke-Ereignis im Replay ist.
        // In Multiplayer:
        // Nuke through the closing seconds of the clock.
        // This triggers no network packet! All players are genukt each locally.
        // This leads yet all the same gameplay, as each player will receive the time step-down
        // package for the same update. We need to do this after Replayauswerten
        // to determine that there is still no nuke event in the replay.
        if (multiplayer && cs.clock_running &&
         cs.clock <= (unsigned long) Lixxie::updates_for_bomb)
         for (Tribe::It tr = cs.tribes.begin(); tr != cs.tribes.end(); ++tr) {
            if (!tr->nuke) {
                // Paket anfertigen.
            	// Customize package
                Replay::Data  data;
                data.update = upd;
                data.player = tr->masters.begin()->number;
                data.action = Replay::NUKE;
                replay.add(data);

                // Und sofort ausfuehren: Replay wurde ja schon ausgewertet
                // And running immediately: Replay was already evaluated
                tr->lix_hatch = 0;
                tr->nuke           = true;
                if (&*tr == trlo) {
                    pan.nuke_single.set_on();
                    pan.nuke_multi .set_on();
                }
                effect.add_sound(upd, *tr, 0, Sound::NUKE);
            }
        }
        // Singleplayer:
        // Upon running out of time entirely, shut all exits
        if (! multiplayer && cs.clock_running && cs.clock == 0
         && ! cs.goals_locked) {
            cs.goals_locked = true;
            effect.add_sound(upd, *trlo, 0, Sound::OVERTIME);
        }
        // Ebenfalls etwas Uhriges: Gibt es Spieler mit geretteten Lixen,
        // die aber keine Lixen mehr im Spiel haben oder haben werden? Dann
        // wird die Nachspielzeit angesetzt. Falls aber alle Spieler schon
        // genukt sind, dann setzen wir die Zeit nicht an, weil sie vermutlich
        // gerade schon ausgelaufen ist.
        // Also something quiet: there are players with rescued Lixen,
        // but have no more Lixen in the game or have to be? Then,
        // the added time will be applied. However, if all
        // the players are already genukt, then we set the time
        // not to, because it's probably just been leaked.
        if (!cs.clock_running)
         for (Tribe::CIt i = cs.tribes.begin(); i != cs.tribes.end(); ++i)
         if (i->lix_saved > 0 && ! i->get_still_playing()) {
            // Suche nach Ungenuktem
        	// Search Unused
            for (Tribe::CIt j = cs.tribes.begin(); j != cs.tribes.end(); ++j)
             if (! j->nuke && j->get_still_playing()) {
                cs.clock_running = true;
                // Damit die Meldung nicht mehrmals kommt bei hoher Netzlast
                // Thus, the message is not repeatedly comes at a high network load
                effect.add_overtime(upd, *i, cs.clock);
                break;
            }
            break;
        }
        // Warnsounds
        if (cs.clock_running
         && cs.clock >  0
         && cs.clock != (unsigned long) level.seconds
                                      * gloB->updates_per_second
         && cs.clock <= (unsigned long) gloB->updates_per_second * 15
         && cs.clock % gloB->updates_per_second == 0)
         for (Tribe::CIt i = cs.tribes.begin(); i != cs.tribes.end(); ++i)
         if (!i->lixvec.empty()) {
            // The 0 goes where usually a lixvec ID would go, because this
            // is one of the very few sounds that isn't attached to a lixvec.
            effect.add_sound(upd, *trlo, 0, Sound::CLOCK);
            break;
        }
    }



    // Main iteration over players:
    // Create lix, nuke existing lixes, update lixes
    // No evaluation of replay/network data, that has already happened
    for (Tribe::It t = cs.tribes.begin(); t != cs.tribes.end(); ++t)
    {
        const int position = replay.get_permu()[t - cs.tribes.begin()];
        // Create new Lixxie if necessary
        if (t->lix_hatch != 0 && upd >= 60 &&
         (t->update_hatch == 0 || upd >= t->update_hatch + t->spawnint))
            // sometimes, spawnint can be more than 60. In this case, it's fine
            // to spawn earlier than usual if we haven't spawned anything at
            // all yet, this is the first || criterion.
        {
            t->update_hatch = upd;
            const EdGraphic& h = hatches[t->hatch_next];
            Lixxie& newlix = t->lixvec[level.initial - t->lix_hatch];
            newlix = Lixxie(&*t,
             h.get_x() + h.get_object()->get_trigger_x(),
             h.get_y() + h.get_object()->get_trigger_y());
            --t->lix_hatch;
            ++t->lix_out;

            // Lixes start walking to the left instead of right?
            bool turn_new_lix = false;
            if (h.get_rotation()) turn_new_lix = true;
            // This extra turning solution here is necessary to make
            // some L1 and ONML two-player levels better playable.
            if (hatches.size() < cs.tribes.size()
             && (position / hatches.size()) % 2 == 1) turn_new_lix = true;

            if (turn_new_lix) {
                newlix.turn();
                newlix.move_ahead();
            }

            // It's the next hatches turn
            t->hatch_next += cs.tribes.size();
            t->hatch_next %= hatches.size();
        }
    }



    // Do the nuke even before the normal physics update.
    // Instant nuke should not display a countdown fuse in any frame.
    for (Tribe::It t = cs.tribes.begin(); t != cs.tribes.end(); ++t) {
        // Assign exploders in case of nuke
        if (t->nuke == true)
         for (LixIt i = t->lixvec.begin(); i != t->lixvec.end(); ++i) {
            if (i->get_updates_since_bomb() == 0 && ! i->get_leaving()) {
                i->inc_updates_since_bomb();
                // Which exploder shall be assigned?
                if (cs.tribes.size() > 1) {
                    i->set_exploder_knockback();
                }
                else for (Level::CSkIt itr =  t->skills.begin();
                                       itr != t->skills.end(); ++itr
                ) {
                    if (itr->first == LixEn::EXPLODER2) {
                        i->set_exploder_knockback();
                        break;
                    }
                }
                break;
            }
        }
    }



    // Lixen updaten
    UpdateArgs ua(cs);

    // Erster Durchlauf: Nur die Arbeitstiere bearbeiten und markieren!
    for (Tribe::It t = cs.tribes.begin(); t != cs.tribes.end(); ++t) {
        for (LixIt i = t->lixvec.begin(); i != t->lixvec.end(); ++i) {
            if (i->get_ac() > LixEn::WALKER) {
                ua.id = i - t->lixvec.begin();
                i->mark();
                update_lix(*i, ua);
            }
            // Sonst eine vorhandene Markierung ggf. entfernen
            else i->unmark();
        }
    }
    // Zweiter Durchlauf: Unmarkierte bearbeiten
    for (Tribe::It t = cs.tribes.begin(); t != cs.tribes.end(); ++t) {
        for (LixIt i = t->lixvec.begin(); i != t->lixvec.end(); ++i) {
            if (!i->get_mark()) {
                ua.id = i - t->lixvec.begin();
                update_lix(*i, ua);
            }
        }
    }
    // Third pass (if necessary): finally becoming flingers
    if (Lixxie::get_any_new_flingers()) {
        for (Tribe::It t = cs.tribes.begin(); t != cs.tribes.end(); ++t)
         for (LixIt i = t->lixvec.begin(); i != t->lixvec.end(); ++i) {
            if (i->get_fling_new()) finally_fling(*i);
        }
    }





    // Ende Haupt-Lix-Update-Geschichten
    // Dies ist aber ebenfalls sehr wichtig fuer jedes Update, egal ob
    // normal oder nachberechnet: Traps and flingers depend on the animation
    // frame for their actions.
    for (TrigIt i = cs.trap.begin(); i != cs.trap.end(); ++i) i->animate();
    for (TrigIt i = cs.fling.begin(); i != cs.fling.end(); ++i) i->animate();
    for (TrigIt i = cs.trampoline.begin(); i != cs.trampoline.end(); ++i)
                                                                i->animate();

    if (verify_mode == INTERACTIVE_MODE)
        state_manager.calc_save_auto(cs);
}
// Ende update_once



// ############################################################################
// ######################################################### update_cs_one_data
// ############################################################################



void Gameplay::update_cs_one_data(Tribe& t, Tribe::Master* m, Replay::It i)
{
    const Ulng& upd = cs.update;

    if (i->action == Replay::SPAWNINT) {
        const int spint = i->what;
        if (spint >= t.spawnint_fast && spint <= t.spawnint_slow) {
            t.spawnint = spint;
            if (&t == trlo) pan.spawnint_cur.set_spawnint(t.spawnint);
        }
    }
    else if (i->action == Replay::NUKE) {
        if (!t.nuke) {
            t.lix_hatch = 0;
            t.nuke      = true;
            if (&t == trlo) {
                pan.nuke_single.set_on();
                pan.nuke_multi .set_on();
            }
            effect.add_sound(upd, t, 0, Sound::NUKE);
        }
    }
    else if (i->action == Replay::ASSIGN
          || i->action == Replay::ASSIGN_LEFT
          || i->action == Replay::ASSIGN_RIGHT) {
        if (!m) return;

        Level::SkIt psk = t.skills.find(static_cast <LixEn::Ac> (i->skill));
        if (psk == t.skills.end())
            // should never happen
            return;

        if (i->what < t.lixvec.size()) {
            Lixxie& lix = t.lixvec[i->what];
            // false: Do not respect the user's options like
            // disabling the multiple builder feature
            if (lix.get_priority(psk->first, false) > 1 && psk->second != 0
             && ! (lix.get_dir() ==  1 && i->action == Replay::ASSIGN_LEFT)
             && ! (lix.get_dir() == -1 && i->action == Replay::ASSIGN_RIGHT)
            ) {
                ++(t.skills_used);
                if (psk->second != LixEn::infinity) --(psk->second);
                lix.evaluate_click(psk->first);
                // Draw arrow if necessary, read arrow.h/effect.h for info
                if ((useR->arrows_replay  && replaying)
                 || (useR->arrows_network && (multiplayer && ! replaying)
                                          && m != malo)) {
                    Arrow arr(map, t.style, lix.get_ex(), lix.get_ey(),
                        psk->first, upd, i->what);
                    effect.add_arrow(upd, t, i->what, arr);
                }
                Sound::Id snd = Lixxie::get_ac_func(psk->first).sound_assign;
                if (m == malo)
                    effect.add_sound      (upd, t, i->what, snd);
                else if (&t == trlo)
                    effect.add_sound_quiet(upd, t, i->what, snd);
            }
        }
        // we will reset all skill numbers on the panel anyway after
        // this function has finished
    }

}
// end of update_cs_one_data
