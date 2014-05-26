#include "VoxelEncoder.h"
#include "VoxelCallbacks.h"

#include "prtx/GenerateContext.h"
#include "prtx/ShapeIterator.h"
#include "prtx/EncodeOptions.h"
#include "prtx/EncoderInfoBuilder.h"
#include "prtx/EncodePreparator.h"

#include <iostream>
#include <fstream>
#include <sstream>


namespace {

const wchar_t* EO_I_RES = L"voxelResolution";

void toSTL(const std::vector<prtx::EncodePreparator::FinalizedInstance>& instances, std::ostream& out) {
	static const char NL = '\n';

	out << std::scientific;
	out << "solid bunnytmp" << NL;
	for (const prtx::EncodePreparator::FinalizedInstance& instance : instances) {
		for (const prtx::MeshPtr& m : instance.getGeometry()->getMeshes()) {
			for (uint32_t fi = 0, n = m->getFaceCount(); fi < n; fi++) {
				const prtx::DoubleVector& vc = m->getVertexCoords();

				const uint32_t* fvi = m->getFaceVertexIndices(fi);
				assert(m->getFaceVertexCount(fi) == 3);
				uint32_t vi0 = 3 * fvi[0];
				uint32_t vi1 = 3 * fvi[1];
				uint32_t vi2 = 3 * fvi[2];

				const uint32_t* fvni = m->getFaceVertexNormalIndices(fi);
				const double* fn = &m->getVertexNormalsCoords()[3 * fvni[0]];

				out << "facet normal " << fn[0] << " " << fn[1] << " " << fn[2] << NL;
				out << "  outer loop" << NL;
				out << "    vertex " << vc[vi0] << " " << vc[vi0+1] << " " << vc[vi0+2] << NL;
				out << "    vertex " << vc[vi1] << " " << vc[vi1+1] << " " << vc[vi1+2] << NL;
				out << "    vertex " << vc[vi2] << " " << vc[vi2+1] << " " << vc[vi2+2] << NL;
				out << "  endloop" << NL;
				out << "endfacet" << NL;
			}
		}
	}
	out << "endsolid" << NL;
}

uint8_t* read_binvox(const std::string& filespec, size_t& resultSize) {
  std::ifstream *input = new std::ifstream(filespec.c_str(), std::ios::in | std::ios::binary);

  //
  // read header
  //
  std::string line;
  *input >> line;  // #binvox
  if (line.compare("#binvox") != 0) {
    std::cout << "Error: first line reads [" << line << "] instead of [#binvox]" << std::endl;
    delete input;
    return 0;
  }
  int version;
  *input >> version;
  std::cout << "reading binvox version " << version << std::endl;

  int depth, height, width;
  depth = -1;
  int done = 0;
  while(input->good() && !done) {
    *input >> line;
    if (line.compare("data") == 0) done = 1;
    else if (line.compare("dim") == 0) {
      *input >> depth >> height >> width;
    }
    else {
      std::cout << "  unrecognized keyword [" << line << "], skipping" << std::endl;
      char c;
      do {  // skip until end of line
        c = input->get();
      } while(input->good() && (c != '\n'));

    }
  }
  if (!done) {
    std::cout << "  error reading header" << std::endl;
    return 0;
  }
  if (depth == -1) {
    std::cout << "  missing dimensions in header" << std::endl;
    return 0;
  }

  resultSize = width * height * depth;
  uint8_t *voxels = new uint8_t[resultSize];
  if (!voxels) {
    std::cout << "  error allocating memory" << std::endl;
    return 0;
  }

  //
  // read voxel data
  //
  uint8_t value;
  uint8_t count;
  size_t index = 0;
  size_t end_index = 0;
  int nr_voxels = 0;

  input->unsetf(std::ios::skipws);  // need to read every byte now (!)
  *input >> value;  // read the linefeed char

  while((end_index < resultSize) && input->good()) {
    *input >> value >> count;

    if (input->good()) {
      end_index = index + count;
      if (end_index > resultSize)
		  return 0;
      for(size_t i=index; i < end_index; i++) voxels[i] = value;

      if (value) nr_voxels += count;
      index = end_index;
    }  // if file still ok

  }  // while

  input->close();

  return voxels;
}

}


const std::wstring VoxelEncoder::ID				= L"VoxelEncoder";
const std::wstring VoxelEncoder::NAME			= L"Voxel Encoder";
const std::wstring VoxelEncoder::DESCRIPTION	= L"Returns voxelized model";

VoxelEncoder::VoxelEncoder(const std::wstring& id, const prt::AttributeMap* defaultOptions, prt::Callbacks* callbacks)
: prtx::GeometryEncoder(id, defaultOptions, callbacks)
{
}

VoxelEncoder::~VoxelEncoder() {
}

void VoxelEncoder::init(prtx::GenerateContext&) {
	mVoxelRes = getOptions()->getInt(EO_I_RES);
}

void VoxelEncoder::encode(prtx::GenerateContext& context, size_t initialShapeIndex) {
	const prtx::InitialShape* is = context.getInitialShape(initialShapeIndex);

	uint32_t totalShapeCount = 0;
	try {
		prtx::BreadthFirstIteratorPtr bfi = prtx::BreadthFirstIterator::create(context, initialShapeIndex);
		for (prtx::ShapePtr shape = bfi->getNext(); shape.get() != 0; shape = bfi->getNext()) {
			totalShapeCount++;
		}
	} catch(...) {
	}

	prtx::DefaultNamePreparator	mNamePreparator;
	prtx::NamePreparator::NamespacePtr nsMaterials = mNamePreparator.newNamespace();
	prtx::NamePreparator::NamespacePtr nsMeshes = mNamePreparator.newNamespace();
	prtx::EncodePreparatorPtr mEncodePreparator = prtx::EncodePreparator::create(true, mNamePreparator, nsMeshes, nsMaterials);

	uint32_t leafShapeCount = 0;
	try {
		prtx::LeafIteratorPtr li = prtx::LeafIterator::create(context, initialShapeIndex);
		for (prtx::ShapePtr shape = li->getNext(); shape.get() != 0; shape = li->getNext()) {
			mEncodePreparator->add(context.getCache(), shape, is->getAttributeMap());
			leafShapeCount++;
		}
	} catch(...) {
		mEncodePreparator->add(context.getCache(), *is, initialShapeIndex);
	}

	prtx::EncodePreparator::PreparationFlags prepFlags;
	prepFlags.instancing(false);
	prepFlags.mergeByMaterial(true);
	prepFlags.triangulate(true);
	prepFlags.mergeVertices(true);
	prepFlags.cleanupVertexNormals(true);
	prepFlags.cleanupUVs(true);
	prepFlags.processVertexNormals(prtx::VertexNormalProcessor::SET_ALL_TO_FACE_NORMALS);

	std::vector<prtx::EncodePreparator::FinalizedInstance> finalizedInstances;
	mEncodePreparator->fetchFinalizedInstances(finalizedInstances, prepFlags);

	std::string tmpSTL = "/tmp/bunny.stl";
	std::ofstream stl("/tmp/bunny.stl");
	toSTL(finalizedInstances, stl);
	stl.close();

	VoxelCallbacks* cb = dynamic_cast<VoxelCallbacks*>(getCallbacks());

	std::ostringstream binvox;
	binvox << "binvox -d " << mVoxelRes << " -cb " << tmpSTL;
	std::system(binvox.str().c_str());

	size_t voxelSize = 0;
	uint8_t* voxels = read_binvox("/tmp/bunny.binvox", voxelSize);
	cb->updateVoxels(voxels, voxelSize);
	delete [] voxels;

	double leafAttrProp = 0.0;
	// TODO: compute leaf attributes
	cb->updateStatistics(leafShapeCount, totalShapeCount-leafShapeCount, leafAttrProp);
}

void VoxelEncoder::finish(prtx::GenerateContext& /*context*/) {
}

VoxelEncoderFactory* VoxelEncoderFactory::createInstance() {
	prtx::EncoderInfoBuilder encoderInfoBuilder;

	encoderInfoBuilder.setID(VoxelEncoder::ID);
	encoderInfoBuilder.setName(VoxelEncoder::NAME);
	encoderInfoBuilder.setDescription(VoxelEncoder::DESCRIPTION);
	encoderInfoBuilder.setType(prt::CT_GEOMETRY);

	prtx::PRTUtils::AttributeMapBuilderPtr amb(prt::AttributeMapBuilder::create());
	amb->setInt(EO_I_RES, 128);
	encoderInfoBuilder.setDefaultOptions(amb->createAttributeMap());

	return new VoxelEncoderFactory(encoderInfoBuilder.create());
}
