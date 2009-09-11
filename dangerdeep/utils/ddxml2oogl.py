#!/usr/bin/env python
# -*- coding: iso-8859-15 -*-
#
#    Copyright (C) 2009 Matthew Lawrence
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as
#    published by the Free Software Foundation, either version 3 of the
#    License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys, gts, xml.parsers.expat

# play with these values till your happy
# 	level:percent, lev.... etc.
# there is no limit on the number of levels.
# note first one is ignored as level 0 is the original model
levels = { 0:0, 1:125, 2:100, 3:75, 4:20 }

"""
Converts a ddxml model into a series of oogl [Geomview] format. Each oogl file
has a differing number of triangles.

Primary purpose of this utility is to test different Level of Detail [LoD] on
our models and determine appropriate settings.

To view the generated files you will need geomview from:
	http://www.geomview.org/

"""
class ddxml_decoder:
	"""
This is a expat parser class.
	"""
	current_node = None
	surface = None
	history = []
	vertices = []
	indices = []
	surfaces = []

	def start( self, name, attribs ):
		"""
Call back for the start of elements, keeps track of the current node and keeps
a history of nested nodes.
		"""
		self.history.append( name )
		self.current_node = name
		pass

	def stop( self, name ):
		"""
Call back for the end of elements, this invokes surface generation once both
indicie and vertice nodes have been parsed.
		"""
		# reset after mesh has being processed
		if 'mesh' == name:
			self.convert()
			self.surfaces.append( self.surface )
			self.surface = None
			self.vertices = []
			self.indices = []

		self.history.pop()
		try:
			current_node = self.history[ len( self.history) - 1 ]
		except:
			current_node = None
		pass
	
	def data( self, text ):
		"""
Call back for content between nodes. For vertex data this converts the text
into the appropriate objects from the gts library. For indice data the text is
split and converted into integers to be used as [index] lookups.
		"""
		# ignore white space
		text = text.strip()
		if 0 == len( text ):
			return

		if 'vertices' == self.current_node:
			point = []
			for vextex in text.split():
				point.append( float( vextex ) )

				if 3 == len( point ):
					self.vertices.append( gts.Vertex( 
						point[0], point[1], point[2] ) )
					point = []

		if 'indices' == self.current_node:
			for index in text.split():
				self.indices.append( int( index ) )

	def convert( self ):
		"""
Converts the data from the current/last mesh node. Each triangle is made up of
1 or more triangles.
		"""
		tri = []
		self.surface = gts.Surface()
		for index in self.indices:
			tri.append( index )

			# for every group of 3 indicies
			if 3 == len( tri ):
				e1 = gts.Edge( self.vertices[ tri[0] ], 
					self.vertices[ tri[1] ] )
				e2 = gts.Edge( self.vertices[ tri[1] ],
					self.vertices[ tri[2] ] )
				e3 = gts.Edge( self.vertices[ tri[2] ],
					self.vertices[ tri[0] ] )
				self.surface.add( gts.Face( e1, e2, e3 ) )
				tri = []

def convert( ddxml_in, oogl_out ):
	"""
Main function to convert the given in file into the given out file(s).
	"""
	global levels
	
	# setup expat
	pxml = xml.parsers.expat.ParserCreate()
	decoder = ddxml_decoder()

	pxml.StartElementHandler = decoder.start
	pxml.EndElementHandler = decoder.stop
	pxml.CharacterDataHandler = decoder.data

	# read in ddxml
	fd = open( ddxml_in, 'r' )
	data = fd.read()
	fd.close()

	# parse and sanity checks
	pxml.Parse( data )

	if 0 == len( decoder.surfaces ):
		print('Error: No surfaces found, missing mesh or bad ddxml')
		return

	mesh_count = 0
	for surface in decoder.surfaces:
		print( 'Mesh %d:' % mesh_count )

		# for each mesh produce a reduced model
		for level in levels:
			target  = levels[ level ] / 100.0
			target *= surface.stats()['n_faces']

			stemp = gts.Surface().copy( surface )

			if 0 != level:
				stemp.coarsen( int( target ) )

			fd = open( oogl_out % ( mesh_count, level ), 'w+' )
			stemp.write_oogl( fd )
			fd.close()

			print( '\tWrote level %d with %d faces' % 
				( level, stemp.stats()['n_faces'] ) )
		mesh_count += 1

if '__main__' == __name__:
	if 2 != len( sys.argv ):
		print( 'Usage: %s COD.ddxml' % ( sys.argv[0] ) )
	else:
		input = sys.argv[1]
		output = input[ 0 : input.rfind( '.' ) ] + '_m%d_l%d.oogl'
		convert( input, output )
