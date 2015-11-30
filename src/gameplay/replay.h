/*
 * gameplay/replay.h
 *
 * Struktur ReplayData
 *
 * Klasse Replay
 *
 *   Aufzeichnung aller Spielgeschehenisse, um eindeutig den Spielverlauf zu
 *   rekonstruieren. Es werden viele verschiedene ReplayData-Instanzen
 *   verwaltet, die abgerufen werden koennen. Immer, wenn etwas Notwendiges
 *   passiert, wird eine neue Instanz von ReplayData gespeichert.
 *
 *   Es kann per Funktion ein kleiner ReVec abgerufen werden, der alle
 *   Ereignisse fuer ein Update enthaelt.
 *
 * bool holds_level
 *
 *   Wenn kein Ort mit einer Leveldatei angegeben wurde, gehen wir davon aus,
 *   dass wir soeben die Leveldaten in der Replaydatei mitgeparst haben. Dann
 *   soll der, der die Replaydatei laedt, anhand dieser Variable sehen, dass
 *   zum Level-Laden auch der Replay-Dateiname verwendet werden muss.
 *
 *   Die Replay-Speicherfunktion erkennt, ob diese Variable gesetzt wird oder
 *   aber auch, ob wir den temporaeren Netzwerklevel spielen! In beiden Faellen
 *   wird der Level mitgespeichert.
 *
 * ULong what
 *
 *   Beschreibt, welche/r Lix/Skill/Rate gewaehlt wurde.
 *   Im Falle von AIM (Zielen mit der Maus) wird folgendes Format angewandt:
 *
 *     what = (y * xl * maxlix) + (x * maxlem) + lixid
 *
 *   Hier sind (x, y) die Koordinaten des Punktes, auf den gezielt wird,
 *   und xl ist die Breite des Levels. lixid ist die ID der Lix, der
 *   zielen soll, und maxlix ist die Anfangsanzahl an Lixen des Spielers.
 *   Das passt auf jeden Fall in einen 32-Bit-Unsigned.
 *
 */

#pragma once

#include <fstream>
#include <set>
#include <vector>

#include "../lix/lix_enum.h"
#include "../network/net_t.h" // work with struct ReplayData
#include "../network/permu.h"
#include "../other/file/date.h"
#include "../other/file/filename.h"
#include "../other/types.h"

class Level;
class GameState;

class Replay {

public:

    typedef ReplayData Data;

    enum Action {
        NOTHING,
        SPAWNINT,
        SKILL_LEGACY_SUPPORT, // only while reading files, never used after
        ASSIGN,
        ASSIGN_LEFT,
        ASSIGN_RIGHT,
        NUKE,
		WRONG_ASSIGN
    };

    struct Player {
        PlNr        number;
        LixEn::Style  style;
        std::string name;
        Player() : number(0), style(LixEn::GARDEN) {}
        Player(PlNr nr, LixEn::Style s, const std::string& n)
         : number(nr), style(s), name(n) {}
        bool operator < (const Player& rhs) const
         { return number < rhs.number; }
    };

    typedef std::vector <Data>                  Vec;
    typedef std::vector <Data> ::iterator       It;
    typedef std::vector <Data> ::const_iterator ConstIt;

    typedef std::set <Player>  ::const_iterator PlayerCIt;

private:

    bool              file_not_found; // Stammt aus nichtext. Datei?

    Ulng              version_min;
    Date              built_required; // Welche Version vom Level ist OK?
    Filename          level_filename;

    std::set <Player> players;
    Permu             permu;

    Vec               data;
    Uint              max_updates;
    LixEn::Ac         first_skill_bc; // bc = backwards compatibility skill,
                                      // what skill to assign if no SKILL
                                      // command has occured yet
    PlNr              player_local;

public:

    Replay();
    Replay(const Filename&);
    ~Replay();

    inline void set_level_filename(const Filename& s) { level_filename = s; }
    inline const Filename& get_level_filename()      { return level_filename;}
    inline const std::set <Player>& get_players()    { return players;       }
           void  add_player(Uint pos, LixEn::Style s, const std::string& name);
    const  std::string& get_player_local_name();
    inline Ulng         get_version_min()          { return version_min;     }
    inline const Date&  get_built_required()       { return built_required;  }

    inline const Permu& get_permu() const          { return permu;           }
    inline void         set_permu(const Permu& p)  { permu = p;              }
    inline void         shorten_permu_to(size_t n) { permu.shorten_to(n);    }

    inline PlNr get_player_local   ()              { return player_local;    }
    inline void set_player_local   (PlNr u)        { player_local = u;       }
    inline void set_max_updates    (const Uint i)  { max_updates = i;        }
    inline Uint get_max_updates    () const        { return max_updates;     }

    inline bool get_file_not_found () const        { return file_not_found;  }

    bool        equal_before                   (const Replay&, Ulng) const;

    void        clear                          ();
    void        increase_early_data_to_update  (const Ulng);
    void        erase_data_after_update        (const Ulng);
    void        erase_early_singleplayer_nukes ();
    bool        get_on_update_lix_clicked      (const Ulng, const Uint,
                                                const LixEn::Ac);
    Vec         get_and_erase_data_until_update(const Ulng);
    Vec         get_data_for_update            (const Ulng) const;
    inline const Vec& get_data                 () { return data; }
    void        add                            (const Data&);
    void        add                            (const Vec&);

    // call this after loading the replay from a file, after knowing what
    // the first non-zero skill has been in the level.
    void fix_legacy_replays_according_to_current_state(
        const GameState&, const std::vector <LixEn::Ac>& legacy_ac_vec);

    std::string get_canonical_save_filename    ();

    void        save_as_auto_replay            (const Level* const = 0);
    void        save_to_file                   (const Filename&,
                                                const Level* const = 0);
    void        load_from_file                 (const Filename&);

};
