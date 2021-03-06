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


#include <editor/html2format.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <locale/translate.h>
#include <styles/logic.h>
#include <database/logs.h>
#include <pugixml/utils.h>
#include <quill/logic.h>


void Editor_Html2Format::load (string html)
{
  // The web editor may insert non-breaking spaces. Convert them to normal spaces.
  html = filter_string_str_replace (unicode_non_breaking_space_entity (), " ", html);
  
  // The web editor produces <hr> and other elements following the HTML specs,
  // but the pugixml XML parser needs <hr/> and similar elements.
  html = html2xml (html);
  
  // The user may add several spaces in sequence. Convert them to single spaces.
  // But the way the footnotes are entered cause at times two spaces in sequence.
  // Those spaces are important.
  // See issue https://github.com/bibledit/cloud/issues/460.
  // So now only change three subsequent spaces to two.
  html = filter_string_str_replace ("   ", "  ", html);
  html = filter_string_str_replace ("   ", "  ", html);

  string xml = "<body>" + html + "</body>";
  // Parse document such that all whitespace is put in the DOM tree.
  // See http://pugixml.org/docs/manual.html for more information.
  // It is not enough to only parse with parse_ws_pcdata_single, it really needs parse_ws_pcdata.
  // This is significant for, for example, the space after verse numbers, among other cases.
  xml_parse_result result = document.load_string (xml.c_str(), parse_ws_pcdata);
  // Log parsing errors.
  pugixml_utils_error_logger (&result, xml);
}


void Editor_Html2Format::run ()
{
  preprocess ();
  process ();
  postprocess ();
}


void Editor_Html2Format::process ()
{
  // Iterate over the children to retrieve the "p" elements, then process them.
  xml_node body = document.first_child ();
  for (xml_node node : body.children()) {
    // Process the node.
    processNode (node);
  }
}


void Editor_Html2Format::processNode (xml_node node)
{
  switch (node.type ()) {
    case node_element:
    {
      // Skip a note with class "ql-cursor" because that is an internal Quill node.
      // The user didn't insert it.
      string classs = node.attribute("class").value();
      if (classs == "ql-cursor") break;
      // Process node normally.
      openElementNode (node);
      for (xml_node child : node.children()) {
        processNode (child);
      }
      closeElementNode (node);
      break;
    }
    case node_pcdata:
    {
      // Add the text with the current character format to the containers.
      string text = node.text ().get ();
      texts.push_back(text);
      formats.push_back(current_character_format);
      break;
    }
    default:
    {
      string nodename = node.name ();
      Database_Logs::log ("Unknown XML node " + nodename + " while saving editor text");
      break;
    }
  }
}


void Editor_Html2Format::openElementNode (xml_node node)
{
  // The tag and class names of this element node.
  string tagName = node.name ();
  string className = update_quill_class (node.attribute ("class").value ());
  
  if (tagName == "p")
  {
    // In the editor, it may occur that the p element does not have a class.
    // Use the 'p' class in such a case.
    if (className.empty ()) className = "p";
    texts.push_back("\n");
    formats.push_back(className);
    // A new line starts: Clear the character formatting.
    current_character_format.clear();
  }
  
  if (tagName == "span")
  {
    openInline (className);
  }
}


void Editor_Html2Format::closeElementNode (xml_node node)
{
  // The tag and class names of this element node.
  string tagName = node.name ();
  string className = update_quill_class (node.attribute ("class").value ());

  if (tagName == "p")
  {
    // While editing it happens that the p element does not have a class.
    // Use the 'p' class in such cases.
    if (className.empty()) className = "p";
    // Clear active character styles.
    current_character_format.clear();
  }
  
  if (tagName == "span")
  {
    // End of span: Clear character formatting.
    current_character_format.clear();
  }
}


void Editor_Html2Format::openInline (string className)
{
  current_character_format = className;
}


void Editor_Html2Format::preprocess ()
{
  texts.clear();
  formats.clear();
}


void Editor_Html2Format::postprocess ()
{
}


string Editor_Html2Format::update_quill_class (string classname)
{
  classname = filter_string_str_replace (quill_logic_class_prefix_block (), "", classname);
  classname = filter_string_str_replace (quill_logic_class_prefix_inline (), "", classname);
  return classname;
}
