// Note: this is really ugly, maya api sucks so much there are no words to 
// describe it, it is stupid like hell, so do not ask! This exporter has no
// error handling whatsoever. I hope it works, if not, fix it. ddxml is also
// ugly ... do not blame me

#include <math.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <stack>
#include <vector>
#include <algorithm>

#include <maya/MSimple.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MPoint.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MMatrix.h>
#include <maya/MVector.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshFaceVertex.h>
#include <maya/MItDag.h>
#include <maya/MFnTransform.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>

using namespace std;

struct material {
	string name;
	float diffuseColor[3];
	float specularColor[3];
	int shininess;
};

struct mesh {
	string name;
	list<float> vertices;
	list<int> indices;
	list<float> texcoords;
	list<float> normals;
};

class ErisianExport : MPxCommand {
	public:
		virtual MStatus doIt(const MArgList&);
		static void *creator() {return new ErisianExport;}
		void readMesh(const MDagPath &);
		void dagWalk();
		void write();
		void createSubtree();
	private:
		fstream ostr;
		list<mesh> meshList;
		list<material> materialList;
		int genId;
};

MStatus ErisianExport::doIt(const MArgList&) {
	ostr.open("/tmp/result.xml", ios_base::out);
	ostr.setf(ios::fixed);
	dagWalk();
	write();
}

MStatus initializePlugin(MObject obj) {
	MFnPlugin pluginFn(obj, "Eris", "2");
	MStatus stat;
	stat = pluginFn.registerCommand("ErisianExport", ErisianExport::creator);
	if(!stat)
		stat.perror("register command failed");
	return stat;
}

MStatus uninitializePlugin(MObject obj) {
	MFnPlugin pluginFn(obj);
	MStatus stat;
	stat = pluginFn.deregisterCommand("ErisianExport");
	if(!stat) {
		stat.perror("deregister command failed");
	}
	return stat;
}

void ErisianExport::dagWalk() {
	genId = 0;
	MStatus status;
	MSelectionList activeList;
	MGlobal::getActiveSelectionList(activeList);
	MItSelectionList iter(activeList);

	for(; !iter.isDone(); iter.next()) {
		MDagPath item;
		MObject component;
		iter.getDagPath(item, component);

		MString pathString = item.fullPathName();
		if(item.hasFn(MFn::kLight)) {
		}
		else if(item.hasFn(MFn::kMesh)) {
			readMesh(item);
			MFnDagNode testNode(item);
		}
	}
}

void ErisianExport::createSubtree() {
	MStatus status;
	MSelectionList activeList;
	MGlobal::getActiveSelectionList(activeList);
	MItSelectionList iter(activeList);
	
	double temp[4];
	int pathLength = 0, meshId = 0;
	stack<string> pathNames;
	ostr << "  " << "<objecttree>" << endl;
	MPoint zero(0, 0, 0, 1);
	vector<MPoint> mpVector(23, zero);
	for(; !iter.isDone(); iter.next()) {
		MDagPath item;
		MObject component;
		iter.getDagPath(item, component);
		if(item.hasFn(MFn::kMesh)) {
			MString pathString = item.partialPathName();
			MFnMesh meshFn(item);
			MObject pivot = item.transform();
			MFnTransform parentTransform(pivot, &status);
			MPoint pivotPoint = parentTransform.rotatePivot(MSpace::kObject, &status);

			if(pathLength < item.length()) {
				pathLength = item.length();
				mpVector[pathLength] = pivotPoint;
				pathNames.push(pathString.asChar() );
				for(int i=0; i<pathLength; i++) {
					ostr << "  "; 
				}
				ostr << "  " << "<object " << "id=\"" << genId << "\" "
					"name=\"" << pathString << "\" mesh=\"" << meshId << "\">" << endl;
				for(int i=0; i<pathLength+1; i++) {
					ostr << "  "; 
				}
				(pivotPoint - mpVector[pathLength-1]).get(temp);
				ostr << "  " << "<translation vector=\"" << temp[0] << " " << temp[1] << " " << temp[2]  << "\"/>" << endl;
			}
			else {
				int diff = -item.length() + pathLength + 1;
				while(diff>0) {
					for(int i=0; i<pathLength; i++) {
						ostr << "  "; 
					}
					ostr << "  " << "</object>" << endl;
					pathLength--; diff--;
					pathNames.pop();
				}
				pathLength = item.length();
				pathNames.push(pathString.asChar() );
				for(int i=0; i<pathLength; i++) {
					ostr << "  ";
				}
				ostr << "  " << "<object " << "id=\"" << genId << "\" "
					"name=\"" << pathString << "\" mesh=\"" << meshId << "\">" << endl;
				for(int i=0; i<pathLength+1; i++) {
					ostr << "  ";
				}
				(pivotPoint - mpVector[pathLength-1]).get(temp);
				ostr << "  " << "<translation vector=\"" << temp[0] << " " << temp[1] << " " << temp[2]  << "\"/>" << endl;
			}
		}
		genId++;
		meshId++;
	}
	for(int i = pathNames.size(); i > 0; i--) {
		for(int j=pathLength; j>0; j--) {	
			ostr << "  ";
		}
		pathLength--;
		ostr << "  " << "</object>" << endl;
		pathNames.pop();
	}
	ostr << "  " << "</objecttree>" << endl;
}


struct v_unite
{
	unsigned idx_vertex;
	unsigned idx_normal;
	unsigned idx_texcoord;
	unsigned idx_trans;
	v_unite(unsigned iv, unsigned in, unsigned it, unsigned tr)
		: idx_vertex(iv), idx_normal(in), idx_texcoord(it), idx_trans(tr) {}
	bool operator< (const v_unite& o) const {
		return (idx_vertex != o.idx_vertex) ? (idx_vertex < o.idx_vertex) :
			( (idx_normal != o.idx_normal) ? (idx_normal < o.idx_normal) :
			  ( (idx_texcoord != o.idx_texcoord) ? (idx_texcoord < o.idx_texcoord) :
			    false ) );
	}
	bool operator== (const v_unite& o) const {
		return idx_vertex == o.idx_vertex &&
			idx_normal == o.idx_normal &&
			idx_texcoord == o.idx_texcoord;
	}
};



void ErisianExport::readMesh(const MDagPath & dagForMesh) {
	MStatus status;
	mesh thisMesh;
	MFnMesh meshFn(dagForMesh);
	thisMesh.name = dagForMesh.partialPathName().asChar();
	MObject pivot = dagForMesh.transform();
	MFnTransform parentTransform(pivot, &status);
	MPoint pivotPoint = parentTransform.rotatePivot(MSpace::kObject, &status);

	// iterate polygons to get number of triangles for each poly
	MItMeshPolygon polyIter(dagForMesh, MObject::kNullObj, &status);
	MString uvSetName;
	meshFn.getCurrentUVSetName(uvSetName);
	int num_faceverts = meshFn.numFaceVertices(&status);
	vector<v_unite> v_union;
	v_union.reserve(num_faceverts);
	for ( ; !polyIter.isDone(); polyIter.next()) {
		MIntArray verts;
		status = polyIter.getVertices(verts);
		// iterate vertices
		for (unsigned j = 0; j < verts.length(); ++j) {
			int nrml = polyIter.normalIndex(j, &status);
			int texc = 0;
			status = polyIter.getUVIndex(j, texc, &uvSetName);
			v_union.push_back(v_unite(verts[j], nrml, texc, v_union.size()));
		}
	}
	// sort to unite the vector
	sort(v_union.begin(), v_union.end());
	// now iterate the vector and count real vertices we have
	unsigned k = 0;
	unsigned equal = 0, newverts = 1;
	// build the reciprocal table simultaneously
	// that table tells for which per-face-vertex (corner-vertex) which new vertex to use
	vector<unsigned> rptab(v_union.size());
	rptab[v_union[0].idx_trans] = 0;
	vector<unsigned> startsnewvertex(v_union.size());
	startsnewvertex[0] = 1;
	MFloatVectorArray nrmls; // each member has x,y,z attributes
	meshFn.getNormals(nrmls);
	const double NRML_DIFF = 0.0001;
	for (unsigned i = 1; i < v_union.size(); ++i) {
		if (v_union[k] == v_union[i]) {
			// entries are equal
			++equal;
		} else {
			// check if normals really differ by their value, not only by index
			if (v_union[k].idx_vertex == v_union[i].idx_vertex &&
			    v_union[k].idx_texcoord == v_union[i].idx_texcoord &&
			    fabs(nrmls[v_union[k].idx_normal].x - nrmls[v_union[i].idx_normal].x) < NRML_DIFF &&
			    fabs(nrmls[v_union[k].idx_normal].y - nrmls[v_union[i].idx_normal].y) < NRML_DIFF &&
			    fabs(nrmls[v_union[k].idx_normal].z - nrmls[v_union[i].idx_normal].z) < NRML_DIFF) {
			    	// normals have different index, but same value
			    	v_union[i].idx_normal = v_union[k].idx_normal;
			    	++equal;
			} else {
				++newverts;
				k = i;
				startsnewvertex[i] = 1;
			}
		}
		rptab[v_union[i].idx_trans] = newverts-1;
	}
	// to write vertices (with normals,texcoords), just write vertex/normal/texcoord data,
	// but merged/unified!
	MFloatArray uValues, vValues;
	meshFn.getUVs(uValues, vValues, &uvSetName);
	MItMeshVertex vertIter(dagForMesh, MObject::kNullObj, &status );
	vector<float> vtx_data;
	vtx_data.reserve(3*meshFn.numVertices(&status));
	for(; !vertIter.isDone(); vertIter.next() ) {
		MPoint vertPoint = vertIter.position(MSpace::kObject, &status );
		vertPoint -= pivotPoint;
		vtx_data.push_back(vertPoint.x);
		vtx_data.push_back(vertPoint.y);
		vtx_data.push_back(vertPoint.z);
	}
	//ostr << "<!-- orignal verts=" << vtx_data.size()/3 << " nrmls=" << nrmls.length() << " texcs=" << uValues.length() << " -->\n";
	// write loop
	for (unsigned i = 0; i < v_union.size(); ++i) {
		if (startsnewvertex[i]) {
			unsigned vi = v_union[i].idx_vertex;
			thisMesh.vertices.push_back(vtx_data[3*vi+0]);
			thisMesh.vertices.push_back(vtx_data[3*vi+1]);
			thisMesh.vertices.push_back(vtx_data[3*vi+2]);
			unsigned ni = v_union[i].idx_normal;
			thisMesh.normals.push_back(nrmls[ni].x);
			thisMesh.normals.push_back(nrmls[ni].y);
			thisMesh.normals.push_back(nrmls[ni].z);
			unsigned ti = v_union[i].idx_texcoord;
			thisMesh.texcoords.push_back(uValues[ti]);
			thisMesh.texcoords.push_back(1.0f - vValues[ti]); // Maya seems to use values reversed...
		}
	}
#if 0
	// show vector
	ostr << "verts=" << v_union.size() << " eql=" << equal << " newverts=" << newverts << "\n";
	for (unsigned i = 0; i < v_union.size(); ++i) {
		ostr << "i=" << i << " vert=" << v_union[i].idx_vertex
		     << " nrml=" << v_union[i].idx_normal
		     << " texc=" << v_union[i].idx_texcoord
		     << " trans=" << v_union[i].idx_trans
		     << " newvert=" << rptab[i]
		     << "\n";
	}
#endif
	// to write mesh indices, iterate all polys, there all triangles
	// and write out their vertex indices, but translated via rptab
	MItMeshPolygon faceIter(dagForMesh, MObject::kNullObj, &status);
	unsigned facevertoff = 0;
	for( ; !faceIter.isDone(); faceIter.next() ) {
		MIntArray verts;
		status = faceIter.getVertices(verts);
		int num_triangles = 0;
		status = faceIter.numTriangles(num_triangles);
		for (int j = 0; j < num_triangles; ++j) {
			MPointArray triPoints;
			MIntArray vtxList;
			status = faceIter.getTriangle(j, triPoints, vtxList, MSpace::kObject);
			// shit, these are _original_ vertex indices...
			// we need vidx per poly instead!
			// we try to find these by searching in verts, because
			// we assume each (global) vertex is ref'd only once per polygon.
			for (unsigned k = 0; k < vtxList.length(); ++k) {
				int global_vidx = vtxList[k];
				int local_vidx = 0;//-1;
				for (unsigned m = 0; m < verts.length(); ++m) {
					if (verts[m] == global_vidx) {
						local_vidx = int(m);
					}
				}
				//thisMesh.indices.push_back(facevertoff+local_vidx);//test
				thisMesh.indices.push_back(rptab[facevertoff+local_vidx]);
			}
		}
		facevertoff += verts.length();
	}
	meshList.push_back(thisMesh);
}

void ErisianExport::write() {
	int id=0;
	
	ostr << "<dftd-model version=\"1.0\">" << endl;
	ostr << "  " << "<material name=\"phong1SG\" id=\"0\">" << endl;
	ostr << "    " << "<diffuse color=\"1 1 1\" />" << endl;
	ostr << "    " << "<specular color=\"1 0.95 0.9\" />" << endl;
	ostr << "    " << "<shininess exponent=\"30\" />" << endl;
	ostr << "  " << "</material>" << endl;
	list<mesh>::iterator elem;
	for( elem = meshList.begin(); elem != meshList.end(); elem++ ) {
		ostr << "  " << "<mesh name=\"" << elem->name << "\" id=\"" << id++
			<< "\" material=\"0\">" << endl;
		ostr << "    " << "<vertices nr=\"" << elem->vertices.size() / 3<< "\">";
		mesh local  = (mesh)*elem;
		while(!local.vertices.empty()) {
			float p = local.vertices.front();
			ostr << p << " "; local.vertices.pop_front();
		}
		ostr << "</vertices>" << endl;
		ostr << "  " << "<indices nr=\""<< local.indices.size() << "\"> ";
		while(!local.indices.empty()) {
			int p = local.indices.front();
			ostr << p << " "; local.indices.pop_front();
		}
		ostr << "</indices>" << endl;
		ostr << "    " << "<texcoords> " ;
		while(!local.texcoords.empty()) {
			float p = local.texcoords.front();
			ostr << p << " "; local.texcoords.pop_front();
		}
		ostr << "</texcoords>" << endl;
		ostr << "    " << "<normals> " ;
		while(!local.normals.empty()) {
			float p = local.normals.front();
			ostr << p << " "; local.normals.pop_front();
		}
		ostr << "</normals>" << endl;
		ostr << "  " << "</mesh>" << endl;
	}
	createSubtree();
	ostr << "</dftd-model>" << endl;
}
