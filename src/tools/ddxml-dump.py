#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys,math
import maya.cmds as cmds
import maya.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx

from operator import attrgetter

from xml.dom.minidom import getDOMImplementation
from xml.dom import Node

# Usage:
# Use The following MEL script, save to the shelf or something:

# loadPlugin "ddxml-dump.py";
# dumpDDXML();
# unloadPlugin "ddxml-dump.py";

# Put the file 'ddxml-dump.py' (this file) into $MAYA_HOME/plug-ins
# Select the object to export 
# Run the MEL script
# Pray.


# Assumptions:
#
# * All transformation objects will have a mesh
# * From the root object down, each node will only have one mesh

ddxml_vendor = "Danger from the Deep Project"
ddxml_version = "3"
ddxml_command = "dumpDDXML"

# command object
class dumpDDXML(OpenMayaMPx.MPxCommand):
	def __init__(self):
		OpenMayaMPx.MPxCommand.__init__(self)
		pass
	
	def doIt(self,argList):
		self.filename=None
		self.filenamePrompt()
		if None==self.filename:
			return
		self.output = ddxmlWriter()
		self.output.createDefaultMaterial()
		try:
			self.walk()
		except Exception, e:
			log( e.message )
			cmds.headsUpMessage("ERROR: "+str(e.message))
			raise

		f=open(self.filename,'w')
		self.output.dump(f)
		f.close()
	
	def dagIsNone(self,dag):
		# if dag is a real obj, comparing to None causes a error.. go figure
		try:
			isNone=(None==dag)
		except ValueError:
			isNone=False

		return isNone

	def filenamePrompt(self):
		res=cmds.promptDialog(title='Exported filename',message='Filename:',text="/tmp/dftd-rocks.xml",button=['OK', 'Cancel'],defaultButton='OK',cancelButton='Cancel',dismissString='Cancel')
		if 'OK'==res:
			self.filename = cmds.promptDialog(query=True, text=True)

	def walk(self,dag=None,parent=None):
		if self.dagIsNone(dag):
			dag=self.getRootDAG()

		name=dag.partialPathName()
		mesh=self.getMesh(dag)
		mesh_name=mesh.partialPathName()
		mesh_obj=ddxmlMesh(mesh_name)
		self.readMesh(mesh_obj,mesh,dag)
		self.output.addMesh(mesh_obj)

		if None != parent:
			self.output.addObject(name,mesh_name,parent.partialPathName())
		else:
			self.output.addObject(name,mesh_name)

		self.createTransformation(dag,parent)

		for xform in self.listTransforms(dag):
			self.walk(xform,dag)

	def createTransformation(self,dag,pdag):
		xform=OpenMaya.MFnTransform(dag)

		if self.dagIsNone(pdag):
			ppoint=OpenMaya.MPoint(0.0,0.0,0.0,1.0)
		else:
			pxform=OpenMaya.MFnTransform(pdag)
			ppoint=pxform.rotatePivot(OpenMaya.MSpace.kObject)

		point=xform.rotatePivot(OpenMaya.MSpace.kObject) - ppoint
		vec3=[point.x, point.y, point.z]

		total=0
		for dt in vec3:
			total=total+math.fabs(dt)

		if total > 0.01:
			self.output.addTranslation(dag.partialPathName(), vec3)

		for minT,maxT in [(OpenMaya.MFnTransform.kRotateMinX,OpenMaya.MFnTransform.kRotateMaxX),(OpenMaya.MFnTransform.kRotateMinY,OpenMaya.MFnTransform.kRotateMaxY),(OpenMaya.MFnTransform.kRotateMinZ,OpenMaya.MFnTransform.kRotateMaxZ)]:
			limMin,limMax=self.getLimit(xform,minT,maxT)
			if None != limMin and None != limMax:
				self.output.addRotation(dag.partialPathName(),self.makeAxis(minT,maxT),0.0,RtD*limMin,RtD*limMax)
				# FIXME hardcoded angle of 0.0 ...

	def makeAxis(self,minT,maxT):
		if minT==OpenMaya.MFnTransform.kRotateMinX and maxT==OpenMaya.MFnTransform.kRotateMaxX:
			return "1 0 0"
		if minT==OpenMaya.MFnTransform.kRotateMinY and maxT==OpenMaya.MFnTransform.kRotateMaxY:
			return "0 1 0"
		if minT==OpenMaya.MFnTransform.kRotateMinZ and maxT==OpenMaya.MFnTransform.kRotateMaxZ:
			return "0 0 1"
		raise Exception("Unknown axis")
	
	def getLimit(self,xform,minType,maxType):
		if xform.isLimited(minType):
			minVal=xform.limitValue(minType)
		else:
			minVal=None
		if xform.isLimited(maxType):
			maxVal=xform.limitValue(maxType)
		else:
			maxVal=None
		return (minVal,maxVal)

	def readMesh(self,mesh,dag,parent):
		# Mesh obj
		rmesh=OpenMaya.MFnMesh(dag)
		# parent transform
		xform=OpenMaya.MFnTransform(parent)
		# get the pivot point so we can fix the vertice offset
		op=xform.rotatePivot(OpenMaya.MSpace.kObject)
		# get current UV set name
		uvSet=rmesh.currentUVSetName()

		# one vertex may have more than one normal and more than one UV
		# Maya can do this because it has independent index lists for vertices, normals and UVs
		# We only have one so we must unify/merge into one
		unite=[]
		for poly in WrapIt(OpenMaya.MItMeshPolygon(dag)):
			if not poly.hasUVs():
				log("WARNING: Poly with no UVs, exporting junk; poly="+str(poly.index()))

			verts=OpenMaya.MIntArray()
			poly.getVertices(verts)

			for i in range(0,verts.length()):
				if not poly.hasUVs():
					unite.append(Indexes(verts[i],poly.normalIndex(i),0,len(unite)))
				else:
					uv=intp()
					poly.getUVIndex(i,uv[0],uvSet)
					unite.append(Indexes(verts[i],poly.normalIndex(i),intp(uv),len(unite)))

		unite=sorted(unite, key=attrgetter('vid','nid','tid')) 

		# Create a list of new vertices to use
		k=0
		newverts=1
		rptab={}
		rptab[unite[0].i]=0
		start_new_vertex={}
		start_new_vertex[0]=1
		normals=OpenMaya.MFloatVectorArray()
		rmesh.getNormals(normals)

		for i in range(1,len(unite)):
			if not unite[k]==unite[i]:
				if unite[i].nearlyEq(unite[k],normals):
					unite[i].nid=unite[k].nid
				else:
					newverts+=1
					k=i
					start_new_vertex[i]=1
			rptab[unite[i].i] = newverts - 1

		vertexData=[]
		for vertex in WrapIt(OpenMaya.MItMeshVertex(dag)):
			point=vertex.position(OpenMaya.MSpace.kObject)
			np = point-op
			vertexData.append(np.x)
			vertexData.append(np.y)
			vertexData.append(np.z)

		# Outputs
		mesh.v=[]	# 1 vertex as a string "%f %f %f"
		mesh.n=[]	# 1 normal as a string "%f %f %f"
		mesh.i=[]	# indexes "%i"
		mesh.uv=[]	# uv pairs "%f %f"

		# original UV arrays
		u=OpenMaya.MFloatArray()
		v=OpenMaya.MFloatArray()
		rmesh.getUVs(u,v)

		for i in range(0,len(unite)):
			if i in start_new_vertex:
				iObj=unite[i]
				mesh.v.append("%f %f %f"%(vertexData[(3*iObj.vid)+0],vertexData[(3*iObj.vid)+1],vertexData[(3*iObj.vid)+2]))
				mesh.n.append("%f %f %f"%(normals[iObj.nid].x,normals[iObj.nid].y,normals[iObj.nid].z))
				mesh.uv.append("%f %f"%(u[iObj.tid],1.0-v[iObj.tid]))

		facevertoff=0
		for poly in WrapIt(OpenMaya.MItMeshPolygon(dag)):
			verts=OpenMaya.MIntArray()
			poly.getVertices(verts)
			numTri=intp()
			poly.numTriangles(numTri[0])
			for j in range(0,intp(numTri)):
				triPoints=OpenMaya.MPointArray()
				vtx_list=OpenMaya.MIntArray()
				poly.getTriangle(j,triPoints,vtx_list,OpenMaya.MSpace.kObject)
				for k in range(0,vtx_list.length()):
					global_vidx=vtx_list[k]
					local_vidx=0
					for m in range(0,verts.length()):
						if verts[m]==global_vidx:
							local_vidx=m
					mesh.i.append(str(rptab[facevertoff+local_vidx]))

			facevertoff+=verts.length()

	def getMesh(self,m_dag):
		log("Extacting mesh object from; "+m_dag.partialPathName())
		k_mesh=OpenMaya.MDagPath()
		found_mesh=False
		for i in range(0, m_dag.childCount()):
			m_obj=m_dag.child(i)
			if m_obj.hasFn(OpenMaya.MFn.kMesh) and not found_mesh:
				m_dag.getAPathTo(m_obj,k_mesh)
				found_mesh=True
				log("found mesh; "+k_mesh.partialPathName())
			elif m_obj.hasFn(OpenMaya.MFn.kMesh) and found_mesh:
				raise Exception("multiple meshs found on object; "+m_dag.partialPathName())
		
		if not found_mesh:
			raise Exception("no mesh found for object; "+m_dag.partialPathName())
		return k_mesh

	def listTransforms(self,m_dag):
		log("Extacting transforms from; "+m_dag.partialPathName())
		xforms=[]
		for i in range(0, m_dag.childCount()):
			m_obj=m_dag.child(i)
			if m_obj.hasFn(OpenMaya.MFn.kTransform):
				m_dag_xform=OpenMaya.MDagPath()
				m_dag.getAPathTo(m_obj,m_dag_xform)
				xforms.append(m_dag_xform)
				log("found transform; "+m_dag_xform.partialPathName())
		
		return xforms

	def getRootDAG(self):
		selection=OpenMaya.MSelectionList()
		OpenMaya.MGlobal.getActiveSelectionList(selection)

		if selection.isEmpty():
			raise Exception("Nothing selected")

		if 1 != selection.length():
			raise Exception("Multiple selections, only select the root node")

		dagPath=OpenMaya.MDagPath()
		selection.getDagPath(0,dagPath)

		return dagPath

class ddxmlMesh:
	def __init__(self,name,material="phong1SG"):
		self.name=name
		self.material=material

class ddxmlWriter:
	def __init__(self):
		self.root=getDOMImplementation().createDocument(None, "dftd-model", None)
		version=self.root.createAttribute("version")
		version.value="1.1"
		self.root.documentElement.setAttributeNode(version)

		self.objecttree=self.root.createElement("objecttree")
		self.root.documentElement.appendChild(self.objecttree)

		self.material_count=0
		self.mesh_count=0
		self.object_count=0
	
	def addRotation(self,obj,axis,angle,minA,maxA):
		node=self.find("object",obj)

		if None==node:
			raise Exception("Cannot find object; "+str(obj))
		
		rot=self.root.createElement("rotation")
		eaxis=self.root.createAttribute("axis")
		eaxis.value=axis
		eangle=self.root.createAttribute("angle")
		eangle.value=str(angle)
		eminA=self.root.createAttribute("minangle")
		eminA.value=str(minA)
		emaxA=self.root.createAttribute("maxangle")
		emaxA.value=str(maxA)

		rot.setAttributeNode(eaxis)
		rot.setAttributeNode(eangle)
		rot.setAttributeNode(eminA)
		rot.setAttributeNode(emaxA)

		node.appendChild(rot)

	def addTranslation(self,obj,vec3):
		node=self.find("object",obj)

		if None==node:
			raise Exception("Cannot find object; "+str(obj))
		
		xform=self.root.createElement("translation")
		vector=self.root.createAttribute("vector")
		vector.value="%f %f %f" % (vec3[0],vec3[1],vec3[2]) 
		xform.setAttributeNode(vector)

		node.appendChild(xform)

	def addObject(self,name,mesh,parent=None):
		if None==parent:
			parentNode=self.objecttree
		else:
			parentNode=self.find("object",parent)

		if None==parentNode:
			raise Exception("Cannot find parent; "+str(parent))

		obj=self.root.createElement("object")

		xid=self.root.createAttribute("id")
		xid.value=str(self.object_count)
		self.object_count=self.object_count+1
		obj.setAttributeNode(xid)

		ename=self.root.createAttribute("name")
		ename.value=name
		obj.setAttributeNode(ename)

		meshId=self.find("mesh",mesh)

		if None == meshId:
			raise Exception("Failed to look up mesh with the name; "+mesh)

		emesh=self.root.createAttribute("mesh")
		emesh.value=meshId.getAttribute("id")
		obj.setAttributeNode(emesh)

		parentNode.appendChild(obj)
		

	def addMesh(self,mesh):
		emesh=self.root.createElement("mesh")

		name=self.root.createAttribute("name")
		name.value=mesh.name
		emesh.setAttributeNode(name)

		xid=self.root.createAttribute("id")
		xid.value=str(self.mesh_count)
		self.mesh_count=self.mesh_count+1
		emesh.setAttributeNode(xid)

		matId=self.find("material",mesh.material)

		if None == matId:
			raise Exception("Failed to look up material with the name; "+mesh.material)

		material=self.root.createAttribute("material")
		material.value=matId.getAttribute("id")
		emesh.setAttributeNode(material)

		vertices=self.root.createElement("vertices")
		nr=self.root.createAttribute("nr")
		nr.value=str(len(mesh.v))
		data=self.root.createTextNode(" ".join(mesh.v))
		vertices.setAttributeNode(nr)
		vertices.appendChild(data)

		indices=self.root.createElement("indices")
		nr=self.root.createAttribute("nr")
		nr.value=str(len(mesh.i))
		data=self.root.createTextNode(" ".join(mesh.i))
		indices.setAttributeNode(nr)
		indices.appendChild(data)

		normals=self.root.createElement("normals")
		data=self.root.createTextNode(" ".join(mesh.n))
		normals.appendChild(data)

		texcoords=self.root.createElement("texcoords")
		data=self.root.createTextNode(" ".join(mesh.uv))
		texcoords.appendChild(data)

		emesh.appendChild(vertices)
		emesh.appendChild(indices)
		emesh.appendChild(normals)
		emesh.appendChild(texcoords)

		self.root.documentElement.appendChild(emesh)
	
	def find(self,element,name,parent=None):
		if None==parent:
			parent = self.root.documentElement
		for child in parent.childNodes:
			if Node.ELEMENT_NODE == child.nodeType and element == child.nodeName:
				if child.getAttribute("name") == name:
					return child
			if child.hasChildNodes:
				rval=self.find(element,name,child)
				if None != rval:
					return rval
		return None

	def createDefaultMaterial(self):
		mat=self.root.createElement("material")
		name=self.root.createAttribute("name")
		name.value="phong1SG"
		xid=self.root.createAttribute("id")
		xid.value=str(self.material_count)
		self.material_count=self.material_count+1
		mat.setAttributeNode(name)
		mat.setAttributeNode(xid)

		diffuse=self.root.createElement("diffuse")
		color=self.root.createAttribute("color")
		color.value="1 1 1"
		diffuse.setAttributeNode(color)

		specular=self.root.createElement("specular")
		color=self.root.createAttribute("color")
		color.value="1 0.95 0.9"
		specular.setAttributeNode(color)

		shininess=self.root.createElement("shininess")
		exponent=self.root.createAttribute("exponent")
		exponent.value="30"
		shininess.setAttributeNode(exponent)

		mat.appendChild(diffuse)
		mat.appendChild(specular)
		mat.appendChild(shininess)

		self.root.documentElement.appendChild(mat)

	def dump(self,writer=sys.stdout):
		self.root.writexml(writer,addindent="\t",newl="\n")

class Indexes:
	def __init__(self,vid,nid,tid,i):
		self.vid=vid
		self.nid=nid
		self.tid=tid
		self.i=i
	
	def __eq__(self,other):
		return (self.vid==other.vid and self.nid==other.nid and self.tid==other.tid)

	def nearlyEq(self,other,normals):
		if not self.vid==other.vid or not self.tid==other.tid:
			return False
		nSelf=normals[self.nid]
		nOther=normals[other.nid]

		if math.fabs(nOther.x-nSelf.x) < normalDelta and math.fabs(nOther.y-nSelf.y) < normalDelta and math.fabs(nOther.z-nSelf.z) < normalDelta:
			return True
		return False

#util
def log(msg):
	OpenMayaMPx.MPxCommand.displayInfo("===># "+str(msg))

def intp(read=None):
	if None != read:
		return OpenMaya.MScriptUtil(read[0]).asInt()
	else:
		util=OpenMaya.MScriptUtil()
		util.createFromInt(0)
		return [util.asIntPtr(),util]

class WrapIt:
	def __init__(self,obj):
		self.obj=obj
		self.first=True

	def __iter__(self):
		return self

	def next(self):
		if self.obj.isDone():
			raise StopIteration

		if self.first:
			self.first=False
			return self.obj
		self.obj.next()
		return self.obj

RtD=180.0/math.pi
normalDelta=0.0001

# maya glue
def dumpDDXMLFactory():
	return OpenMayaMPx.asMPxPtr(dumpDDXML())

def initializePlugin(m_obj):
	m_plugin = OpenMayaMPx.MFnPlugin(m_obj,ddxml_vendor,ddxml_version)
	try:
		m_plugin.registerCommand(ddxml_command,dumpDDXMLFactory)
	except:
		sys.stderr.write( "Plugin register failed; " + ddxml_command )
		raise

def uninitializePlugin(m_obj):
	m_plugin = OpenMayaMPx.MFnPlugin(m_obj)
	try:
		m_plugin.deregisterCommand( ddxml_command )
	except:
		sys.stderr.write( "Plugin deregister failed; " + ddxml_command )
		raise
