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

define( 'PATH', './' );
define( 'BITS', PATH . 'bits/' );
define( 'PAGES', PATH . 'pages/' );

/* load the bits */
$head = file_get_contents( BITS . 'head.html' );
$foot = file_get_contents( BITS . 'foot.html' );

/* iterate over the pages */
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

?>
