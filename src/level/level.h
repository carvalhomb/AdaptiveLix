/*
 * level.h
 *
 * This struct represents a level that has been loaded from a file.
 * The functions for loading are also a member of this struct.
 *
 * For further reading:
 *
 *   level.cpp      defines some general Level functions.
 *
 *   level_bi.cpp   defines the loading of L1/ONML/etc. level files (.LVL).
 *                  For un-crunching (uncompressing) .DAT level packs,
 *                  see crunch.h and crunch.cpp.
 *
 *   level_io.cpp   defines saving and loading of L++ level files. These are
 *                  ascii files and not compatible with L1/ONML/etc.
 *
 *   level_dr.cpp   defines the drawing routines for a level.
 *
 * The other files in this folder are for loading the original games' graphics
 * and uncompressing .DAT level packs.
 *
 *   crunch.h/cpp   implement the compressor algorithm of L1/ONML/etc.
 *
 *   object.h/cpp   define the struct Object that represents a single terrain
 *                  piece or special object from either the L++ bitmap folder
 *                  or a graphics set from the original games.
 *
 *   obj_lib.h/cpp  define the class ObjLib. This class handles complete
 *                  graphics sets from the original games. It also manages
 *                  all loaded Objects (see object.h) from the L++ bitmap
 *                  folder.
 *
 * The rest of this comment explains the member variables and functions
 * for struct Level, like most L++ header files do for their struct.
 *
 * Date built
 *
 *   Beinhaltet die Information, wann der Level zum letzten Mal veraendert
 *   bzw. gespeichert wurde. Dies ist die einzige Level-Eigenschaft, die
 *   dem operator == egal ist. Zwei Levels unterschiedlicher Bauzeiten koennen
 *   gleich sein.
 *
 *   Der Editor achtet selbst darauf, dass bei einer Aenderung immer auch
 *   built mitgeaendert wird! Der Level aktualisiert dies beim Speichern nicht
 *   selbst! Ganz gut so, etwa wenn ein Netzwerklevel kommt und unveraendert
 *   gespeichert und gespielt werden soll.
 *
 *      Level    (std::string&)
 * void load_from(std::string&)
 *
 *   Dies erschafft bzw. ersetzt die Leveldaten mit dem Inhalt der angegebenen
 *   Datei, die somit eingelesen und verarbeitet wird.
 *
 * void draw_to(BITMAP*, BITMAP* = 0)
 *
 *   Zeichnet die Landschaft des Levels (alle Elemente der Terrain-Liste) auf
 *   das erste der -bergebenen Bitmaps. Falls auch ein zweites Bitmap uber-
 *   geben wird, so wird dies als Stahlmaske benutzt. Wichtig: F-
 *   ist dieses Zeichnen nicht brauchbar, denn die Landschaftsobjekte werden
 *   zu einem einzigen grossen Land-Klumpatsch verschmolzen.
 *
 * void create_preview(unsigned w, unsigned h, int color)
 *
 *   Erschafft ein neues Bitmap und liefert einen Zeiger darauf zurueck.
 *   Das Bitmap hat die uebergebenen Abmessungen und zeigt den Level im
 *   verkleinerten Format. Gegebenenfalls wird oben/unten oder rechts/links
 *   ein Trauerrand in der Farbe color gemalt, damit die Proportionen stimmen.
 *
 * void save_to(std::string&)
 *
 *   Schreibt den Level in die angegebene Datei.
 *
 * void clear()
 *
 *   Versetzt die Datenstruktur in den jungfraulichen Zustand, den ein neues
 *   Objekt, das ohne Laden eines Levels erschaffen worden ist, besitzt.
 *
 * static std::string get_name(const std::string&)
 *
 *   Liefert den Level-Namen, der in der angegebenen Leveldatei steht. Dazu
 *   wird eine vereinfachtere Suche in der Datei durchgefuehrt, als es bei
 *   den Konstruktoren eines Level-Objektes der Fall waere.
 *
 */

#pragma once

#include <list>
#include <string>
#include <map>

#include "object.h"

#include "../lix/lix_enum.h" // Skill::ac

#include "../other/file/date.h"
#include "../other/globals.h"    // empty_string
#include "../other/file/io.h"
#include "../other/types.h" // AlCol

class Lookup;

struct Level {

    enum Status {
        GOOD,
        BAD_HATCH,          // Keine Klappe gesetzt
        BAD_GOAL,           // Kein Ziel gesetzt
        BAD_IMAGE,          // Bilddateien fehlen
        BAD_FILE_NOT_FOUND, // Leveldatei selbst fehlt
        BAD_EMPTY           // Leveldatei vorhanden, aber leer
    };

    enum FileFormat {
        FORMAT_NOTHING,
        FORMAT_LIX,
        FORMAT_BINARY,
        FORMAT_LEMMINI
    };

    struct Pos {
        const Object* ob; // Name des Objektes ("s" wie "String")
        int  x;
        int  y;
        bool mirr; // Spiegelung?
        int  rot;  // Rotation? 0 = normal, 1 bis 3 = gedreht
        bool dark; // Terrain loeschen anstatt neues malen
        bool noow; // Nicht ueberzeichnen?

        Pos();
        bool operator == (const Pos& t) const;
        bool operator != (const Pos& t) const { return !(*this == t); }
    };

    static const int min_xl;
    static const int min_yl;
    static const int max_xl;
    static const int max_yl;
    static const int spawnint_min;
    static const int spawnint_max;

    Date        built;
    int         players_intended;

    std::string author;
    std::string name_german;
    std::string name_english;
    //std::string level_filename; //Records the full path of the level

    std::vector <std::string> hints_german;
    std::vector <std::string> hints_english;

    int  size_x;
    int  size_y;
    bool torus_x;
    bool torus_y;

    bool start_manual; // if not set, ignore start_x and start_y.
    int  start_x;      // start_manual is set by naming at least either start_x
    int  start_y;      // or start_y in a level file.
    int  bg_red;
    int  bg_green;
    int  bg_blue;

    int  seconds;
    int  initial;
    int  required;
    int  spawnint_slow;
    int  spawnint_fast;


    bool      nuke_delayed; // true == nuke button triggers overtime if any
    LixEn::Ac nuke_skill;   // NOTHING == use most appropriate exploder

    bool count_neutrals_only;
    bool transfer_skills;

    typedef std::list <Level::Pos>                          PosLi;
    typedef std::list <Level::Pos> ::const_iterator         PosIt;
    typedef std::list <Level::Pos> ::const_reverse_iterator PosRIt;

    std::vector <PosLi> pos;

    typedef std::map <LixEn::Ac, int> ::iterator       SkIt;
    typedef std::map <LixEn::Ac, int> ::const_iterator CSkIt;

    std::map <LixEn::Ac, int> skills;
    std::vector <LixEn::Ac>   legacy_ac_vec; // never saved, only loaded

    Level(const Filename& = gloB->empty_filename);
    ~Level();

    bool operator == (const Level&) const;
    bool operator != (const Level&) const;

    inline       Status get_status() const { return status;         }
    inline       bool   get_good()   const { return status == GOOD; }
    const std::string&  get_name()   const;


    const std::vector <std::string>& get_hints()  const;

    void    draw_to       (Torbit&, Lookup* = 0) const;
    Torbit  create_preview(int, int, AlCol) const;

    void load_from_stream(std::istream&);
    void load_from_file  (const Filename&);
    void save_to_file    (const Filename&) const;
    void export_image    (const Filename&) const;
    void clear();

private:

    friend class LevelMetaData;

    Status status;

    // Diese Funktionen sind Helfer. drit = draw_iterator_to_bitmap
    void drit(PosIt, Torbit&, Lookup* = 0) const;

    void load_from_vector (const std::vector <IO::Line>&);
    void load_from_binary (const Filename&);
    void load_from_lemmini(const Filename&);

    void load_finalize_binary_or_lemmini(const Filename&);
    void load_finalize();

    void add_object_from_ascii_line(const std::string&,
                          int, int, const std::string&);
    void record_missing_image      (const std::string&);

    static FileFormat  get_file_format (const Filename&);

};

std::ostream& operator << (std::ostream&, const Level&);
std::ostream& operator << (std::ostream&, const Level::PosLi&);

