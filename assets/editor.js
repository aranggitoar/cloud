/*
Copyright (©) 2003-2020 Teus Benschop.

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


function assetsEditorAddNote (quill, style, caller, noteId, chapter, verse) // Todo
{
  // <p class="b-f"><span class="i-notebody1">1</span> + .</p>
  // <p class="b-f"><span class="i-notebody1">1</span> + <span class="i-fr">1.1 </span><span class="i-fk">keyword </span><span class="i-ft">Footnote text.</span></p>
  var length = quill.getLength ();
  quill.insertText (length, "\n", "paragraph", style, "user");
  quill.insertText (length, caller, "character", "notebody" + noteId, "user");
  length++;
  quill.insertText (length, " + ", "character", "", "user");
  length += 3;
  var reference = chapter + "." + verse;
  quill.insertText (length, reference, "character", "fr", "user");
  length += reference.length;
  quill.insertText (length, " ", "character", "fk", "user");
  length++;
  quill.insertText (length, "keyword", "character", "fk", "user");
  length += 7;
  quill.insertText (length, " ", "character", "ft", "user");
  length++;
  quill.insertText (length, "Text.", "character", "ft", "user");
}

