/*
 * level/level_io.cpp
 *
 * Loading and saving of levels
 *
 * For loading levels in the binary level format (LVL), see level_bi.cpp.
 *
 * Do not confuse this file with other/io.cpp. The latter file contains
 * functions for generic L++-style IO.
 *
 */

#include "level.h"
#include "obj_lib.h"

#include "../other/help.h"      // Prae-Extension heraussuchen
#include "../other/language.h"
#include "../other/file/log.h" // report missing images

void Level::load_from_file(const Filename& filename)
{
    clear();
    status = GOOD;

    //level_filename = filename.get_rootless();

    FileFormat fmt = get_file_format(filename);
    if (fmt == FORMAT_BINARY) {
        // load an original .LVL file from L1/ONML/...
        load_from_binary(filename);
    }
    else if (fmt == FORMAT_LEMMINI) {
        // load an .INI file from Lemmini
        load_from_lemmini(filename);
    }
    else {
        // load the regular Lix format
        std::vector <IO::Line> lines;
        if (IO::fill_vector_from_file(lines, filename.get_rootful())) {
            load_from_vector(lines);
        }
        else status = BAD_FILE_NOT_FOUND;
    }
    load_finalize();
}



void Level::load_from_stream(std::istream& in)
{
    clear();
    status = GOOD;

    std::vector <IO::Line> lines;
    if (IO::fill_vector_from_stream(lines, in)) {
        load_from_vector(lines);
    }
    else status = BAD_FILE_NOT_FOUND;

    load_finalize();
}



static void load_tuto(std::vector <std::string>& into, const std::string& s)
{
    // this always goes into index 0
    if (into.empty()) into.push_back(s);
    else              into[0] = s;
}

static void load_hint(std::vector <std::string>& into, const std::string& s)
{
    // empty hints aren't allowed, all hints shall be in consecutive entries
    if (s.empty()) return;

    // hint 0 is the tutorial hint, this should be empty for most levels.
    if (into.empty()) into.push_back(gloB->empty_string);
    into.push_back(s);
}



void Level::load_from_vector(const std::vector <IO::Line>& lines)
{
    for (IO::LineIt i = lines.begin(); i != lines.end(); ++i) switch(i->type) {
    // Strings setzen
    case '$':
        if      (i->text1 == gloB->level_built       ) built       =i->text2;
        else if (i->text1 == gloB->level_author      ) author      =i->text2;
        else if (i->text1 == gloB->level_name_german ) name_german =i->text2;
        else if (i->text1 == gloB->level_name_english) name_english=i->text2;

        else if (i->text1 == gloB->level_hint_german)
            load_hint(hints_german, i->text2);
        else if (i->text1 == gloB->level_hint_english)
            load_hint(hints_english, i->text2);
        else if (i->text1 == gloB->level_tutorial_german)
            load_tuto(hints_german, i->text2);
        else if (i->text1 == gloB->level_tutorial_english)
            load_tuto(hints_english, i->text2);
        break;

    // Ganzzahlwert setzen
    case '#':
        if (i->text1 == gloB->level_players_intended) players_intended =i->nr1;

        if (i->text1 == gloB->level_start_x) {
            start_manual = true;
            start_x      = i->nr1;
        }
        else if (i->text1 == gloB->level_start_y) {
            start_manual = true;
            start_y      = i->nr1;
        }
        else if (i->text1 == gloB->level_size_x  ) size_x   = i->nr1;
        else if (i->text1 == gloB->level_size_y  ) size_y   = i->nr1;
        else if (i->text1 == gloB->level_torus_x ) torus_x  = i->nr1;
        else if (i->text1 == gloB->level_torus_y ) torus_y  = i->nr1;
        else if (i->text1 == gloB->level_bg_red  ) bg_red   = i->nr1;
        else if (i->text1 == gloB->level_bg_green) bg_green = i->nr1;
        else if (i->text1 == gloB->level_bg_blue ) bg_blue  = i->nr1;
        else if (i->text1 == gloB->level_seconds ) seconds  = i->nr1;
        else if (i->text1 == gloB->level_initial ) initial  = i->nr1;
        else if (i->text1 == gloB->level_required) required = i->nr1;
        else if (i->text1 == gloB->level_spawnint_slow) spawnint_slow = i->nr1;
        else if (i->text1 == gloB->level_spawnint_fast) spawnint_fast = i->nr1;

        else if (i->text1 == gloB->level_count_neutrals_only)
                                            count_neutrals_only = i->nr1;
        else if (i->text1 == gloB->level_transfer_skills)
                                            transfer_skills     = i->nr1;

        // legacy support
        else if (i->text1 == gloB->level_initial_legacy) initial  = i->nr1;
        else if (i->text1 == gloB->level_rate) {
            spawnint_slow = 4 + (99 - i->nr1) / 2;
        }

        // otherwise, add a skill
        else {
            LixEn::Ac ac = LixEn::string_to_ac(i->text1);
            if (ac != LixEn::AC_MAX) {
                // if it's zero, it'll be removed again in finalize()
                skills[ac] = i->nr1;

                // this is never removed, and it should be like that
                legacy_ac_vec.push_back(ac);
            }
        }
        break;

    // Neues Objekt
    case ':':
        add_object_from_ascii_line(i->text1,
                                   i->nr1, i->nr2, i->text2);
        break;

    default:
        break;
    }

    // LEGACY SUPPORT: Alte Levels haben das Terrain falschherum notiert.
    // Das zuerst genannte Terrain ist dort ganz vorne, das spaetere drunter.
    // Daher einmal alles umdrehen.
    // Exclude the zero time Date("")! Saved original .LVLs have a time of 0.
    // In early 2011, the maximal number of skills was raised. Prior to that,
    // infinity was 100, and finite skill counts had to be <= 99. Afterwards,
    // infinity was -1, and the maximum skill count was 999.
    if (built != Date("")
     && built <  Date("2009-08-23 00:00:00")) pos[Object::TERRAIN].reverse();
    if (built != Date("")
     && built <  Date("2011-01-08 00:00:00")) {
        for (SkIt itr = skills.begin(); itr != skills.end(); ++itr)
            if (itr->first == 100) itr->second = LixEn::infinity;
    }
}



void Level::add_object_from_ascii_line(
    const std::string& text1,
    const int nr1,
    const int nr2,
    const std::string& text2
) {
    const Object* ob = ObjLib::get(text1);
    if (ob && ob->cb) {
        Level::Pos newpos;
        newpos.ob = ob;
        newpos.x  = nr1;
        newpos.y  = nr2;
        if (ob->type == Object::TERRAIN)
         for (std::string::const_iterator stritr = text2.begin();
         stritr != text2.end(); ++stritr) switch (*stritr) {
            case 'f': newpos.mirr = !newpos.mirr;         break;
            case 'r': newpos.rot  = (newpos.rot + 1) % 4; break;
            case 'd': newpos.dark = !newpos.dark;         break;
            case 'n': newpos.noow = !newpos.noow;         break;
        }
        else if (ob->type == Object::HATCH)
         for (std::string::const_iterator stritr = text2.begin();
         stritr != text2.end(); ++stritr) switch (*stritr) {
            case 'r': newpos.rot  = !newpos.rot;          break;
        }
        pos[ob->type].push_back(newpos);
    }
    // Kein Bild vorhanden
    else record_missing_image(text1);
}



void Level::record_missing_image(
    const std::string& image
) {
    status = BAD_IMAGE;
    // Fehlermeldung in die Logdatei schreiben
    std::string t;
    t = Language::log_level_unknown_bitmap + ' ' + image;
    Log::log(Log::ERROR, t);
}



void Level::load_finalize()
{
    // Einige Standards setzen, wenn die Ladewerte komisch sind
    if (size_x   < min_xl)              size_x   = min_xl;
    if (size_y   < min_yl)              size_y   = min_yl;
    if (size_x   > max_xl)              size_x   = max_xl;
    if (size_y   > max_yl)              size_y   = max_yl;
    if (initial  < 1)                   initial  = 1;
    if (initial  > 999)                 initial  = 999;
    if (required > initial)             required = initial;
    if (spawnint_fast < spawnint_min)   spawnint_fast = spawnint_min;
    if (spawnint_slow > spawnint_max)   spawnint_slow = spawnint_max;
    if (spawnint_fast > spawnint_slow)  spawnint_fast = spawnint_slow;

    if (bg_red   < 0) bg_red   = 0; if (bg_red   > 255) bg_red   = 255;
    if (bg_green < 0) bg_green = 0; if (bg_green > 255) bg_green = 255;
    if (bg_blue  < 0) bg_blue  = 0; if (bg_blue  > 255) bg_blue  = 255;

    if (torus_x) start_x = Help::mod(start_x, size_x);
    if (torus_y) start_y = Help::mod(start_y, size_y);

    // remove skills with zero uses
    bool another_iteration = true;
    while (another_iteration) {
        another_iteration = false;
        for (SkIt itr = skills.begin(); itr != skills.end(); ++itr)
            if (itr->second == 0) {
                skills.erase(itr);
                another_iteration = true;
                break; // break the for, do another one of while
            }
    }

    // Einige Fehler setzen
    // FNF wurde oben schon mit Abbrechen der Funktion gesetzt
    if (status == GOOD) {
        int count = 0;
        for (int type = Object::TERRAIN; type != Object::MAX; ++type)
         count += pos[type].size();
        if      (count == 0)                 status = BAD_EMPTY;
        else if (pos[Object::HATCH].empty()) status = BAD_HATCH;
        else if (pos[Object::GOAL ].empty()) status = BAD_GOAL;
    }
}



// ############################################################################
// ############################################################################
// ############################################################################



void Level::save_to_file(const Filename& filename) const
{
    std::ofstream file(filename.get_rootful().c_str());
    file << *this;
    file.close();
}



static void print_hints(
    std::ostream& out,
    const std::vector <std::string>& vec,
    const std::string& str_tuto,
    const std::string& str_hint
) {
    typedef std::vector <std::string> ::const_iterator VSItr;
    for (VSItr itr = vec.begin(); itr != vec.end(); ++itr) {
        // index 0 is the tutorial hint
        if (itr == vec.begin()) {
            if (! itr->empty()) out << IO::LineDollar(str_tuto, *itr);
        }
        else out << IO::LineDollar(str_hint, *itr);
    }
}



std::ostream& operator << (std::ostream& out, const Level& l)
{
    Date best_built = l.built;
    // This is the date of the infinity change, see above in the load function.
    if (l.built < Date("2011-01-08 00:00:00")) best_built = Date("");

    out
     << IO::LineDollar(gloB->level_built,            best_built        )
//   players_intended shouldn't be yet written, game doesn't admit to set it
//   << IO::LineHash  (gloB->level_players_intended, l.players_intended)
     << IO::LineDollar(gloB->level_author,           l.author          )
     << IO::LineDollar(gloB->level_name_german,      l.name_german     )
     << IO::LineDollar(gloB->level_name_english,     l.name_english    )

     << std::endl;

    print_hints(out, l.hints_german,  gloB->level_tutorial_german,
                                      gloB->level_hint_german);
    print_hints(out, l.hints_english, gloB->level_tutorial_english,
                                      gloB->level_hint_english);
    if (! l.hints_german.empty() || ! l.hints_english.empty()) out
     << std::endl;

    out
     << IO::LineHash  (gloB->level_size_x,       l.size_x  )
     << IO::LineHash  (gloB->level_size_y,       l.size_y  )
     << IO::LineHash  (gloB->level_torus_x,      l.torus_x )
     << IO::LineHash  (gloB->level_torus_y,      l.torus_y );

    if (l.start_manual) out
     << IO::LineHash  (gloB->level_start_x,      l.start_x )
     << IO::LineHash  (gloB->level_start_y,      l.start_y );

    out
     << IO::LineHash  (gloB->level_bg_red,       l.bg_red  )
     << IO::LineHash  (gloB->level_bg_green,     l.bg_green)
     << IO::LineHash  (gloB->level_bg_blue,      l.bg_blue )

     << std::endl

     << IO::LineHash  (gloB->level_seconds,       l.seconds )
     << IO::LineHash  (gloB->level_initial,       l.initial )
     << IO::LineHash  (gloB->level_required,      l.required)
     << IO::LineHash  (gloB->level_spawnint_slow, l.spawnint_slow)
     << IO::LineHash  (gloB->level_spawnint_fast, l.spawnint_fast)

//   << std::endl
//   << IO::LineHash(gloB->level_count_neutrals_only, l.count_neutrals_only)
//   << IO::LineHash(gloB->level_transfer_skills,     l.transfer_skills)

     << std::endl;

    for (Level::CSkIt itr = l.skills.begin(); itr != l.skills.end(); ++itr) {
        if (itr->first == LixEn::NOTHING || itr->second == 0)
            continue;
        out << IO::LineHash(LixEn::ac_to_string(itr->first), itr->second);
    }

    // Erst Spezialobjekte, dann Terrain
    for (int type = Object::TERRAIN; type != Object::MAX; ++type) {
        if (type != Object::TERRAIN) out << l.pos[type];
    }
    out << l.pos[Object::TERRAIN];
    return out;
}



std::ostream& operator << (std::ostream& o, const Level::PosLi& li)
{
    if (!li.empty()) o << std::endl;
    for (Level::PosIt i = li.begin(); i != li.end(); ++i) {
        if (!i->ob) continue;

        std::string str = ObjLib::get_filename(i->ob);

        if (!str.empty()) {
            std::string modifiers;
            if (i->mirr) modifiers += 'f';
            for (int r = 0; r < i->rot; ++r) modifiers += 'r';
            if (i->dark) modifiers += 'd';
            if (i->noow) modifiers += 'n';
            o << IO::LineColon(str, i->x, i->y, modifiers);
        }
    }
    return o;
}



// ############################################################################
// ##################################### Statische Funktionen ausser Operatoren
// ############################################################################



Level::FileFormat Level::get_file_format(const Filename& filename)
{
    if (! ::exists(filename.get_rootful().c_str())) return FORMAT_NOTHING;
    std::ifstream file(filename.get_rootful().c_str(), std::ios::binary);
    // the length check before the read() was necessary for me on Linux
    // to get the Debugger past this, it got stuck on read() when nothing
    // was wrong.
    file.seekg (0, std::ios::end);
    if (file.tellg() < 8) {
        file.close();
        return FORMAT_NOTHING;
    }
    file.seekg(0, std::ios::beg);
    unsigned char buf[8];
    file.read((char*) &buf, 8);
    file.close();

    // A binary file has two-byte numbers in big endian
    // for rate, lixes, required, seconds at the beginning.
    // Neither should be > 0x00FF. If all of them are,
    // this is an ASCII file which shouldn't have '\0' chars.
    if (buf[0] == '\0' || buf[2] == '\0' || buf[4] == '\0' || buf[6] == '\0')
        return FORMAT_BINARY;

    // This isn't a binary file. Is it a Lemmini file?
    // Lemmini files start with "# LVL".
    else if (buf[0] == '#' && buf[1] == ' ' && buf[2] == 'L' && buf[3] == 'V')
        return FORMAT_LEMMINI;

    else return FORMAT_LIX;
}
