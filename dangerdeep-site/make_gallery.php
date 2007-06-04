<?php
/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2006 Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

if ( '/usr/bin/php' != $_SERVER['_'] )
{
	die( 'Not in shell.' );
}


die( 'Comment this line out if you really do mean to run this script (*hint* Know what your doing first.)' );

$fd = opendir( '.' );
$eol = "\n";

$thumb_size = '128x96';
$url_path = 'gallery/0.3.0/';

if ( !$fd )
	die( 'Dir Open failed.' . $eol );

$ix = 0;
$html = '<div class="thumbnails">';

while( false !== ( $file = readdir( $fd ) ) )
{
	if ( is_dir( $file ) )
		continue;

	if ( false !== strpos( $file, '.php' ) )
		continue;

	$ix++;

	$new_name = 'screen_' . $ix . '.jpg';
	$new_name_thumb = 'screen_' . $ix . '_thumb.jpg';
	$convert_cmd = 'convert ' . $new_name . ' -adaptive-resize ' . $thumb_size . ' ' . $new_name_thumb;

	if ( file_exists( $new_name ) )
		die( 'New filename exists: ' . $new_name . $eol );

	if ( !rename( $file, $new_name ) )
		die( 'Count not rename ' . $file . ' to ' . $new_name . $eol );

	if ( false === system( $convert_cmd ) )
		die( 'Convert command failed "' . $convert_cmd . '".' . $eol );

	$html .= '<a href="' . $url_path . $new_name . '" title=""><img src="' . $url_path . $new_name_thumb . '" alt="" width="128" height="96" /></a>';
}

$html .= '</div>';

closedir( $fd );

file_put_contents( 'frag.html', $html );

?>
