/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <unittests/editone.h>
#include <unittests/utilities.h>
#include <styles/logic.h>
#include <filter/url.h>
#include <editone/logic.h>


// Test the logic used in the visual verse editor.
void test_editone_logic () // Todo
{
  trace_unit_tests (__func__);

  string stylesheet = styles_logic_standard_sheet ();
  string directory = filter_url_create_root_path ("unittests", "tests");

  // Prefix.
  {
    string usfm = filter_url_file_get_contents (filter_url_create_path (directory, "editone01.usfm"));
    string html;
    string last_paragraph_style;
    editone_logic_prefix_html (usfm, stylesheet, html, last_paragraph_style);
    string standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone01.html"));
    evaluate (__LINE__, __func__, standard, html);
    evaluate (__LINE__, __func__, "p", last_paragraph_style);
  }
  
  // Editable verse text.
  {
    // Convert USFM to html.
    string usfm = filter_url_file_get_contents (filter_url_create_path (directory, "editone02.usfm"));
    string html;
    string last_paragraph_style;
    string focused_verse_applied_p_style;
    editone_old_logic_editable_html ("p", usfm, stylesheet, html, last_paragraph_style, focused_verse_applied_p_style);
    string standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone02.html"));
    evaluate (__LINE__, __func__, standard, html);
    evaluate (__LINE__, __func__, "p", last_paragraph_style);
    evaluate (__LINE__, __func__, "p", focused_verse_applied_p_style);
    
    // Convert the html back to USFM again.
    string round_tripped_usfm = editone_old_logic_html_to_usfm (stylesheet, html, focused_verse_applied_p_style);
    evaluate (__LINE__, __func__, usfm, round_tripped_usfm);
  }
  
  // Suffix.
  {
    string usfm = filter_url_file_get_contents (filter_url_create_path (directory, "editone03.usfm"));
    string html;
    string last_paragraph_style;
    editone_logic_suffix_html ("q1", usfm, stylesheet, html);
    string standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone03.html"));
    evaluate (__LINE__, __func__, standard, html);
    evaluate (__LINE__, __func__, "", last_paragraph_style);
  }
  
  // Editable verse text including a \b.
  {
    // Convert USFM to html.
    string usfm = filter_url_file_get_contents (filter_url_create_path (directory, "editone04.usfm"));
    string html;
    string last_paragraph_style;
    string focused_verse_applied_p_style;
    editone_old_logic_editable_html ("p", usfm, stylesheet, html, last_paragraph_style, focused_verse_applied_p_style);
    string standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone04.html"));
    evaluate (__LINE__, __func__, standard, html);
    evaluate (__LINE__, __func__, "q1", last_paragraph_style);
    evaluate (__LINE__, __func__, "p", focused_verse_applied_p_style);
    
    // Convert the html back to USFM again.
    string round_tripped_usfm = editone_old_logic_html_to_usfm (stylesheet, html, focused_verse_applied_p_style);
    evaluate (__LINE__, __func__, usfm, round_tripped_usfm);
  }

  // Removing notes from the prefix and appending them to the notes in the suffix.
  {
    string prefix;
    string suffix;
    string standard;
    prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone01prefix1.html"));
    suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone01suffix1.html"));
    editone_logic_move_notes (prefix, suffix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone01prefix2.html"));
    evaluate (__LINE__, __func__, standard, prefix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone01suffix2.html"));
    evaluate (__LINE__, __func__, standard, suffix);
  }

  // Prefix without notes, so moving nothing to the notes in the suffix.
  {
    string prefix;
    string suffix;
    string standard;
    prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone02prefix1.html"));
    suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone02suffix1.html"));
    editone_logic_move_notes (prefix, suffix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone02prefix2.html"));
    evaluate (__LINE__, __func__, standard, prefix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone02suffix2.html"));
    evaluate (__LINE__, __func__, standard, suffix);
  }
  
  // Move notes from the prefix to a suffix that does not have notes of its own.
  {
    string prefix;
    string suffix;
    string standard;
    prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone03prefix1.html"));
    suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone03suffix1.html"));
    editone_logic_move_notes (prefix, suffix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone03prefix2.html"));
    evaluate (__LINE__, __func__, standard, prefix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone03suffix2.html"));
    evaluate (__LINE__, __func__, standard, suffix);
  }
  
  // Test that a empty prefix works fine when trying to move notes from prefix to suffix.
  {
    string prefix;
    string suffix;
    string standard;
    prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone04prefix1.html"));
    suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone04suffix1.html"));
    editone_logic_move_notes (prefix, suffix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone04prefix2.html"));
    evaluate (__LINE__, __func__, standard, prefix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone04suffix2.html"));
    evaluate (__LINE__, __func__, standard, suffix);
  }
  
  // Test that notes from the prefix get moved even to an empty suffix.
  {
    string prefix;
    string suffix;
    string standard;
    prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone05prefix1.html"));
    suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone05suffix1.html"));
    editone_logic_move_notes (prefix, suffix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone05prefix2.html"));
    evaluate (__LINE__, __func__, standard, prefix);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone05suffix2.html"));
    evaluate (__LINE__, __func__, standard, suffix);
  }
}
