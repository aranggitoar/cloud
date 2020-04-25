/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


#include <nmt/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/text.h>
#include <filter/usfm.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/versifications.h>
#include <database/config/bible.h>
#include <database/mappings.h>


void nmt_logic_export (string referencebible, string translatingbible)
{
  Database_Logs::log ("Exporting reference Bible \"" + referencebible + "\" plus translated Bible \"" + translatingbible + "\" for a neural machine translation training job");
  
  Database_Bibles database_bibles;
  Database_Versifications database_versifications;
  Database_Mappings database_mappings;

  vector <string> reference_lines;
  vector <string> translation_lines;
  
  // Get the versification systems of both Bibles.
  string reference_versification = Database_Config_Bible::getVersificationSystem (referencebible);
  string translating_versification = Database_Config_Bible::getVersificationSystem (translatingbible);
  
  vector <int> books = database_bibles.getBooks (referencebible);
  for (auto book : books) {
  
    // Take books that contain text, leave others, like front matter, out.
    string type = Database_Books::getType (book);
    if ((type != "ot") && (type != "nt") && (type != "ap")) continue;
    
    string bookname = Database_Books::getEnglishFromId (book);
    Database_Logs::log ("Exporting " + bookname);
    
    vector <int> chapters = database_bibles.getChapters (referencebible, book);
    for (auto reference_chapter : chapters) {
      
      // Skip chapter 0.
      // It won't contain Bible text.
      if (reference_chapter == 0) continue;
      
      vector <int> verses = database_versifications.getMaximumVerses (book, reference_chapter);
      for (auto & reference_verse : verses) {
       
        // Verse 0 won't contain Bible text: Skip it.
        if (reference_verse == 0) continue;

        // Use the versification system to get the matching chapter and verse of the Bible in translation.
        vector <Passage> translation_passages;
        if ((reference_versification != translating_versification) && !translating_versification.empty ()) {
          translation_passages = database_mappings.translate (reference_versification, translating_versification, book, reference_chapter, reference_verse);
        } else {
          translation_passages.push_back (Passage ("", book, reference_chapter, convert_to_string (reference_verse)));
        }

        // If the conversion from one versification system to another
        // leads to one versee for the reference Bible,
        // and two verses for the Bible in translation,
        // then this would indicate a mismatch in verse contents between the two Bibles.
        // This mismatch would disturb the neural machine translation training process.
        // So such versea are skipped for that reason.
        if (translation_passages.size() != 1) {
          string referencetext = filter_passage_display_inline (translation_passages);
          string message = "Skipping reference Bible verse " + convert_to_string (reference_verse) + " and translated Bible " + referencetext;
          Database_Logs::log (message);
          continue;
        }
        
        int translation_chapter = translation_passages[0].chapter;
        int translation_verse = convert_to_int (translation_passages[0].verse);

        // Convert the verse USFM of the reference Bible to plain verse text.
        string reference_text;
        {
          string chapter_usfm = database_bibles.getChapter (referencebible, book, reference_chapter);
          string verse_usfm = usfm_get_verse_text (chapter_usfm, reference_verse);
          string stylesheet = styles_logic_standard_sheet ();
          Filter_Text filter_text = Filter_Text ("");
          filter_text.text_text = new Text_Text ();
          filter_text.addUsfmCode (verse_usfm);
          filter_text.run (stylesheet);
          reference_text = filter_text.text_text->get ();

          // The text may contain new lines, so remove these,
          // because the NMT training files should not contain new lines mid-text,
          // as that would cause misalignments in the two text files used for training.
          reference_text = filter_string_str_replace ("\n", " ", reference_text);

          // The text contains verse numbers.
          // Remove these.
          if (!reference_text.empty ()) {
            size_t pos = reference_text.find(" ");
            if (pos != string::npos) {
              reference_text.erase (0, ++pos);
            }
          }
        }

        // Convert the verse USFM of the Bible being translated to plain verse text.
        string translation_text;
        {
          string chapter_usfm = database_bibles.getChapter (translatingbible, book, translation_chapter);
          string verse_usfm = usfm_get_verse_text (chapter_usfm, translation_verse);
          string stylesheet = styles_logic_standard_sheet ();
          Filter_Text filter_text = Filter_Text ("");
          filter_text.text_text = new Text_Text ();
          filter_text.addUsfmCode (verse_usfm);
          filter_text.run (stylesheet);
          translation_text = filter_text.text_text->get ();

          // The text may contain new lines, so remove these,
          // because the NMT training files should not contain new lines mid-text,
          // as that would cause misalignments in the two text files used for training.
          translation_text = filter_string_str_replace ("\n", " ", translation_text);

          // The text contains verse numbers.
          // Remove these.
          if (!translation_text.empty ()) {
            size_t pos = translation_text.find(" ");
            if (pos != string::npos) {
              translation_text.erase (0, ++pos);
            }
          }
        }
        
        if (reference_text.empty ()) continue;
        if (translation_text.empty ()) continue;
        reference_lines.push_back (reference_text);
        translation_lines.push_back (translation_text);
      }
    }
  }

  string reference_text = filter_string_implode (reference_lines, "\n");
  string translation_text = filter_string_implode (translation_lines, "\n");
  string reference_path = filter_url_create_root_path (filter_url_temp_dir (), "reference_bible_nmt_training_text.txt");
  string translation_path = filter_url_create_root_path (filter_url_temp_dir (), "translation_bible_nmt_training_text.txt");
  filter_url_file_put_contents (reference_path, reference_text);
  filter_url_file_put_contents (translation_path, translation_text);
  Database_Logs::log ("The text of the reference Bible was exported to ", reference_path);
  Database_Logs::log ("The text of the Bible being translated was exported to ", translation_path);
  Database_Logs::log ("Ready exporting for neural machine translation training");
}
