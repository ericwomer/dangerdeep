<?php
/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2007 Thorsten Jordan, Luis Barrancos and others.

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

define( 'PATH', './' );
define( 'BITS', PATH . 'bits/' );
define( 'PAGES', PATH . 'pages/' );
define( 'EOL', "\n" );

$langs = array( 'en', 'fr' );
$tran = array( 'en' => 'English', 'fr' => 'Français' );
$default = 'en';

/* load the bits */

foreach( $langs as $lang )
{
	$head = file_get_contents( BITS . $lang . '/head.html' );
	$foot = file_get_contents( BITS . $lang . '/foot.html' );

	$shead = $head;
	$sfoot = $foot;

	echo 'Doing lang: ' . $lang . EOL;

	$fd = opendir( PAGES . $lang );

	while( $dir = readdir( $fd ) )
	{
		if ( '.' == $dir{0} || 'CVS' == $dir )
			continue;

		$page = file_get_contents( PAGES . $lang . '/' . $dir );


		foreach( $langs as $temp_lang )
		{
			if ( $default != $temp_lang )
			{
				$head = str_replace( strtoupper( $temp_lang ) . '_URL', str_replace( '.', '.' . $temp_lang . '.', $dir ), $head );
			} else {
				$head = str_replace( strtoupper( $temp_lang ) . '_URL', $dir, $head );
			}

			if ( array_key_exists( $temp_lang, $tran ) )
			{
				$head = str_replace( strtoupper( $temp_lang ) . '_TXT', $tran[$temp_lang], $head );
			} else {
				$head = str_replace( strtoupper( $temp_lang ) . '_TXT', '[MISSING]', $head );
			}
		}

		if ( $default != $lang )
		{
			$nu_file = str_replace( '.', '.' . $lang . '.', $dir );

			@unlink( PATH . $nu_file );
			file_put_contents( PATH . $nu_file , $head . $page . $foot );
			echo 'Wrote: ' . PATH . $nu_file . EOL;
		} else {

			@unlink( PATH . $dir );
			file_put_contents( PATH . str_replace( '.', '.', $dir ) , $head . $page . $foot );
			echo 'Wrote: ' . PATH . $dir . EOL;
		}

		unset( $page );
		$head = $shead;
		$foot = $sfoot;
	}

	closedir( $fd );

	unset( $head );
	unset( $foot );
}

/*
$head = file_get_contents( BITS . 'head.html' );
$foot = file_get_contents( BITS . 'foot.html' );

*/
/* iterate over the pages */
/*
$fd = opendir( PAGES );

while( $dir = readdir( $fd ) )
{
	if ( '.' == $dir{0} || 'CVS' == $dir )
		continue;

	$page = file_get_contents( PAGES . $dir );
	@unlink( PATH . $dir );
	file_put_contents( PATH . $dir, $head . $page . $foot );

	unset( $page );
}

closedir( $fd );
*/
?>
