/*
Copyright (©) 2003-2021 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include <database/abbottsmith.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


// This is the database that contains Abbott-Smith's Manual Greek Lexicon.
// Resilience: It is not written to.
// Chances of corruption are nearly zero.


const char * Database_AbbottSmith::filename ()
{
  return "abbottsmith";
}


void Database_AbbottSmith::create ()
{
  filter_url_unlink (database_sqlite_file (filename ()));
  
  SqliteDatabase sql = SqliteDatabase (filename ());

  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS entry (lemma text, lemmacf text, strong text, contents string);");
  sql.execute ();
}


void Database_AbbottSmith::optimize ()
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("VACUUM;");
  sql.execute ();
}


void Database_AbbottSmith::store (string lemma, string lemma_casefold, string strong, string contents)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("PRAGMA temp_store = MEMORY;");
  sql.execute ();
  sql.clear ();
  sql.add ("PRAGMA synchronous = OFF;");
  sql.execute ();
  sql.clear ();
  sql.add ("PRAGMA journal_mode = OFF;");
  sql.execute ();
  sql.clear ();
  sql.add ("INSERT INTO entry (lemma, lemmacf, strong, contents) VALUES (");
  sql.add (lemma);
  sql.add (",");
  sql.add (lemma_casefold);
  sql.add (",");
  sql.add (strong);
  sql.add (",");
  sql.add (contents);
  sql.add (");");
  sql.execute ();
}


string Database_AbbottSmith::get (string lemma, string strong)
{
  string contents;
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT contents FROM entry WHERE");
  if (lemma.empty()) {
    // No lemma: Select on Strong's number only.
    sql.add ("strong =");
    sql.add (strong);
  } else if (strong.empty()) {
    // No Strong's number: Select on lemma only.
    sql.add ("lemma =");
    sql.add (lemma);
  } else {
    // Both Strong's number and lemma given: Select on any of those.
    sql.add ("lemma =");
    sql.add (lemma);
    sql.add ("OR");
    sql.add ("strong =");
    sql.add (strong);
  }
  sql.add (";");
  vector <string> results = sql.query () ["contents"];
  for (auto result : results) contents.append (result);
  return contents;
}

