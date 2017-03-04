/*
Copyright (©) 2003-2016 Teus Benschop.

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


#include <filter/webview.h>
#include <database/logs.h>


// Whether the information has been logged.
bool filter_webview_logged = false;


bool filter_webview_downgrade (string user_agent)
{
  // Log the browser's user agent once.
  if (!filter_webview_logged) {
    Database_Logs::log (user_agent);
    filter_webview_logged = true;
  }
  // Downgrading based on the user agent is not easy.
  // So don't downgrade at all.
  return false;
}
