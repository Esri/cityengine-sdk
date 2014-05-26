#include "VoxelCallbacks.h"

#include "prt/API.h"
#include "prt/FileOutputCallbacks.h"
#include "prt/LogHandler.h"
#include "prt/FlexLicParams.h"

#include "boost/filesystem.hpp"
#include "boost/foreach.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/assign.hpp"
#include "boost/locale/encoding.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/scoped_array.hpp"
#include "boost/random.hpp"

#include <iostream>


namespace {

const char*		FILE_FLEXNET_LIB		= "flexnet_prt";
const char* 	FILE_LOG				= "bunny.log";
const wchar_t*	FILE_CGA_ERROR			= L"CGAErrors.txt";
const wchar_t*	FILE_CGA_PRINT			= L"CGAPrint.txt";

const wchar_t*	ENCODER_ID_CGA_ERROR	= L"com.esri.prt.core.CGAErrorEncoder";
const wchar_t*	ENCODER_ID_CGA_PRINT	= L"com.esri.prt.core.CGAPrintEncoder";
const wchar_t*	ENCODER_ID_VOX			= L"VoxelEncoder";

namespace UnitQuad {
const double 	vertices[]				= { 0, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 0 };
const size_t 	vertexCount				= 12;
const uint32_t	indices[]				= { 0, 1, 2, 3 };
const size_t 	indexCount				= 4;
const uint32_t 	faceCounts[]			= { 4 };
const size_t 	faceCountsCount			= 1;
}


namespace Loggers {

// TODO: optimize if log level is not output
template<prt::LogLevel L> struct PRTLogger {
	PRTLogger() { }
	virtual ~PRTLogger() { prt::log(wstr.str().c_str(), L); }
	PRTLogger<L>& operator<<(std::wostream&(*x)(std::wostream&)) { wstr << x; return *this; }
	template<typename T> PRTLogger<L>& operator<<(const T& x) { wstr << x;  return *this; }
	PRTLogger<L>& operator<<(const std::string& x) {
		std::copy(x.begin(), x.end(), std::ostream_iterator<char, wchar_t>(wstr));
		return *this;
	}
	std::wostringstream wstr;
};

template<prt::LogLevel L> using LoggerType = Loggers::PRTLogger<L>;
typedef LoggerType<prt::LOG_DEBUG> _LOG_DBG;
typedef LoggerType<prt::LOG_INFO> _LOG_INF;
typedef LoggerType<prt::LOG_WARNING> _LOG_WRN;
typedef LoggerType<prt::LOG_ERROR> _LOG_ERR;

} // namespace Loggers

#define LOG_DBG Loggers::_LOG_DBG()
#define LOG_INF Loggers::_LOG_INF()
#define LOG_WRN Loggers::_LOG_WRN()
#define LOG_ERR Loggers::_LOG_ERR()


struct PRTContext {
	PRTContext(
			const boost::filesystem::path& root,
			const boost::filesystem::path& prtRoot,
			const std::string& licType,
			const std::string& licHost,
			prt::LogLevel logLvl
	)
	: mRoot(root)
	, mPRTRoot(prtRoot)
	, mLogHandler(0)
	, mFileLogHandler(0)
	, mLicHandle(0)
	, mLicType(licType)
	, mLicHost(licHost)
	, mLogLevel(logLvl)
	, mCache(0)
	{
		init();
	}

	~PRTContext() {
		cleanup();
	}

	void init() {
		mLogHandler = prt::ConsoleLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT);
		prt::addLogHandler(mLogHandler);

		boost::filesystem::path fileLogPath = boost::filesystem::temp_directory_path() / FILE_LOG;
		mFileLogHandler = prt::FileLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT, fileLogPath.wstring().c_str());
		prt::addLogHandler(mFileLogHandler);

		if (mLogLevel <= prt::LOG_DEBUG) {
			LOG_DBG << "prt root: " << mPRTRoot;
			LOG_DBG << "lic type: " << mLicType;
			LOG_DBG << "lic host: " << mLicHost;
		}

		boost::filesystem::path prtLicLib =  mPRTRoot / "bin" / "libflexnet_prt.so"; // TODO: use FILE_FLEXNET_LIB

		prt::FlexLicParams flp;
		flp.mActLibPath	= prtLicLib.c_str();
		flp.mFeature	= mLicType.c_str();
		flp.mHostName	= mLicHost.c_str();

		std::wstring wDefExtPath = (mPRTRoot / "lib").wstring();
		std::wstring wExtPath = (mRoot / "lib").wstring();

		prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
		const wchar_t* cExtPath[2] = { wDefExtPath.c_str(), wExtPath.c_str() };
		mLicHandle = prt::init(cExtPath, 2, mLogLevel, &flp, &status);

		if (!good()) {
			LOG_ERR << "failed to initialize prt: " << prt::getStatusDescription(status);
		}

		mCache = prt::CacheObject::create(prt::CacheObject::CACHE_TYPE_DEFAULT);
	}

	const prt::AttributeMap* createValidatedOptions(const wchar_t* encID, const prt::AttributeMap* unvalidatedOptions) const {
			const prt::EncoderInfo* encInfo = prt::createEncoderInfo(encID);
			const prt::AttributeMap* validatedOptions = 0;
			const prt::AttributeMap* optionStates = 0;
			encInfo->createValidatedOptionsAndStates(unvalidatedOptions, &validatedOptions, &optionStates);
			optionStates->destroy();
			encInfo->destroy();
			return validatedOptions;
	}

	void generate(prt::Callbacks* cb, const boost::filesystem::path& targetModelPath) {
		std::wstring rpkURI = L"file:" + (mRoot.parent_path() / "identity.rpk").wstring(); // FIXME

        prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
        const prt::ResolveMap* resolveMap = prt::createResolveMap(rpkURI.c_str(), 0, &status);
		if (status != prt::STATUS_OK) {
			LOG_ERR << L"getting resolve map from '" << rpkURI << L"' failed, aborting.";
			return;
		}

		prt::InitialShapeBuilder* isb = prt::InitialShapeBuilder::create();
		if (boost::filesystem::exists(targetModelPath)) {
				LOG_DBG << "trying to read initial shape geometry from " << targetModelPath;
				isb->resolveGeometry(targetModelPath.wstring().c_str(), resolveMap, mCache);
		}
		else {
			LOG_WRN << "falling back to unit quad";
			isb->setGeometry(
				UnitQuad::vertices,
				UnitQuad::vertexCount,
				UnitQuad::indices,
				UnitQuad::indexCount,
				UnitQuad::faceCounts,
				UnitQuad::faceCountsCount,
				0,
				0
			);
		}

		std::wstring shapeName  = L"TheInitialShape";
		std::wstring ruleFile = L"rules/rule.cgb";
		std::wstring startRule = L"default$init";
		int32_t seed = 666;

		prt::AttributeMapBuilder* isAttrBuilder = prt::AttributeMapBuilder::create();
		const prt::AttributeMap* isAttrs = isAttrBuilder->createAttributeMapAndReset();
		isAttrBuilder->destroy();
		isb->setAttributes(
				ruleFile.c_str(),
				startRule.c_str(),
				seed,
				shapeName.c_str(),
				isAttrs,
				resolveMap
		);

		std::vector<const prt::InitialShape*> initialShapes = boost::assign::list_of(isb->createInitialShapeAndReset());
		isb->destroy();

		// -- setup options for helper encoders
		prt::AttributeMapBuilder* optionsBuilder = prt::AttributeMapBuilder::create();
		optionsBuilder->setInt(L"voxelResolution", 128);
		const prt::AttributeMap* voxOptions = optionsBuilder->createAttributeMapAndReset();
		optionsBuilder->setString(L"name", FILE_CGA_ERROR);
		const prt::AttributeMap* errOptions = optionsBuilder->createAttributeMapAndReset();
		optionsBuilder->setString(L"name", FILE_CGA_PRINT);
		const prt::AttributeMap* printOptions = optionsBuilder->createAttributeMapAndReset();
		optionsBuilder->destroy();

		const prt::AttributeMap* validatedEncOpts = createValidatedOptions(ENCODER_ID_VOX, voxOptions);
		const prt::AttributeMap* validatedErrOpts = createValidatedOptions(ENCODER_ID_CGA_ERROR, errOptions);
		const prt::AttributeMap* validatedPrintOpts = createValidatedOptions(ENCODER_ID_CGA_PRINT, printOptions);

		const prt::AttributeMap* encoderOpts[] = { validatedEncOpts, validatedErrOpts, validatedPrintOpts };
        const wchar_t* encoders[] = { ENCODER_ID_VOX, ENCODER_ID_CGA_ERROR, ENCODER_ID_CGA_PRINT };

		status = prt::generate(&initialShapes[0], initialShapes.size(), 0, encoders, 3, encoderOpts, cb, mCache, 0);
		if (status != prt::STATUS_OK)
			LOG_ERR << "prt::generate() failed with status: '" << prt::getStatusDescription(status) << "' (" << status << ")";

		voxOptions->destroy();
		errOptions->destroy();
		printOptions->destroy();
		validatedEncOpts->destroy();
		validatedErrOpts->destroy();
		validatedPrintOpts->destroy();
		isAttrs->destroy();
		for (const prt::InitialShape* is : initialShapes) is->destroy();
		if (resolveMap) resolveMap->destroy();
	}

	void cleanup() {
		if (mCache)
			mCache->destroy();

		if (mLicHandle)
			mLicHandle->destroy();

		if (mLogHandler) {
			prt::removeLogHandler(mLogHandler);
			mLogHandler->destroy();
		}
		if (mFileLogHandler) {
			prt::removeLogHandler(mFileLogHandler);
			mFileLogHandler->destroy();
		}

		if (mLogLevel <= prt::LOG_DEBUG) {
			LOG_DBG << "destroyed prt context";
		}
	}

	bool good() const { return (mLicHandle != 0); }

	boost::filesystem::path mRoot;
	boost::filesystem::path mPRTRoot;

	prt::ConsoleLogHandler*	mLogHandler;
	prt::FileLogHandler*	mFileLogHandler;

	prt::Object const * 	mLicHandle;
	std::string				mLicType;
	std::string				mLicHost;

	prt::LogLevel			mLogLevel;

	prt::CacheObject*		mCache;
};

PRTContext* prtCtx = 0;


const size_t VOXELRES = 128;

class Model {
public:
	Model() { }
	virtual ~Model() { }

	virtual void move() { assert(false); }
	virtual void jump() { assert(false); }

	virtual double getLogLikelihood() const { return 0.0; }
	virtual double getLogNonLeafCount() const { return 0.0; }
	virtual double getLogLeafAttrProp() const { return 0.0; }
};

class TargetModel;
typedef std::shared_ptr<TargetModel> TargetModelPtr;
class TargetModel : public Model, private VoxelCallbacks {
public:
	TargetModel(const boost::filesystem::path& p) {
		prtCtx->generate(this, p);
	}

	virtual ~TargetModel() {
		delete [] mVoxels;
	}

	virtual void updateVoxels(const uint8_t* voxels, size_t voxelsSize) {
		mVoxelsSize = voxelsSize;
		if (mVoxels != 0)
			delete [] mVoxels;
		mVoxels = new uint8_t[mVoxelsSize];
		std::memcpy(mVoxels, voxels, mVoxelsSize);
	}

	virtual void updateStatistics(uint32_t leafCount, uint32_t nonLeafCount, double leafAttrProp) {
	}

	double getVoxelSquaredErrorSum(const uint8_t* voxels, size_t voxelsSize) {
		assert(mVoxelsSize == voxelsSize);
		double es = 0.0;
		for (size_t i = 0; i < mVoxelsSize; i++) {
			es += static_cast<double>( (voxels[i] - mVoxels[i]) * (voxels[i] - mVoxels[i]) );
		}
		return es;
	}

private:
	uint8_t* mVoxels = 0;
	size_t mVoxelsSize = 0;
};

class PRTModel : public Model, private VoxelCallbacks { // TODO: rename to RJMCMCModel?
public:
	PRTModel(const TargetModelPtr& targetModel) : mTargetModel(targetModel) {
	}

	virtual ~PRTModel() { }

	virtual void move() {
			// TODO: select random leaf shape
			// TODO: resample its parameters
			// TODO: regenerate
	}

	virtual void jump() {
			// TODO: select random non-leaf shape
			// TODO: change seed for this non-leaf shape and regenerate
	}

	virtual double getLogLikelihood() const {
		return mLogLikelihood;
	}

	virtual double getLogNonLeafCount() const {
		return mLogNonLeafCount;
	}

	virtual double getLogLeafAttrProp() const {
		return mLogLeafAttrProp;
	}

	virtual void updateVoxels(const uint8_t* voxels, size_t voxelsSize) {
		double sqErrSum = mTargetModel->getVoxelSquaredErrorSum(voxels, voxelsSize);
		static const double twoSqSigma = 0.22 * std::pow(voxelsSize, 1.0/3.0); // see talton mpm paper, sec 11.1
		mLogLikelihood = sqErrSum / twoSqSigma;

	}

	virtual void updateStatistics(uint32_t leafCount, uint32_t nonLeafCount, double leafAttrProp) {
		mLeafCount = leafCount;
		mLogNonLeafCount = std::log(static_cast<double>(nonLeafCount));
	}

private:
	const TargetModelPtr mTargetModel;

	uint32_t mLeafCount = 0;
	double mLogLikelihood = 0.0;
	double mLogNonLeafCount = 0.0;
	double mLogLeafAttrProp = 0.0;
};

} // namespace anonymous


void run(const boost::filesystem::path& targetModelPath) {
	LOG_INF << "starting simulation with target " << targetModelPath;

	boost::random::mt19937 rng;
	boost::random::uniform_real_distribution<double> rnd01(0.0, 1.0);

	TargetModelPtr targetModel(new TargetModel(targetModelPath));

	Model* cm = new PRTModel(targetModel);
	Model* bm = nullptr;

	const size_t N = 1;

	double maxLikelihood = 0.0;
	for (size_t i = 0; i < N; i++) {
		LOG_INF << "-- iteration " << i;
		Model* nm = new PRTModel(static_cast<const PRTModel&>(*cm));

		double acceptance = 0.0;
		if (rnd01(rng) < 0.5) {
			LOG_INF << "   MOVE";
			nm->move();
			double dl = nm->getLogLikelihood() - cm->getLogLikelihood();
			acceptance = std::min(1.0, std::exp(dl));
			LOG_INF << "   acceptance = " << acceptance;
		}
		else {
			LOG_INF << "   JUMP";
			nm->jump();
			double dl = nm->getLogLikelihood() - cm->getLogLikelihood();
			double dnlc = nm->getLogNonLeafCount() - cm->getLogNonLeafCount();
			double dlap = nm->getLogLeafAttrProp() - cm->getLogLeafAttrProp();
			acceptance = std::min(1.0,  std::exp(dl + dnlc + dlap));
			LOG_INF << "   acceptance = " << acceptance;
		}

		if (rnd01(rng) < acceptance) {
			delete cm;
			cm = nm;
		}
		else
			delete nm;

		if (cm->getLogLikelihood() > maxLikelihood) {
			maxLikelihood = cm->getLogLikelihood();
			delete bm;
			bm = new PRTModel(static_cast<const PRTModel&>(*cm));
		}
	}

	// TODO: output bm
	delete bm;

	LOG_INF << "end";
}

int main(int argc, char **argv) {
	// TODO: proper argument handling
	boost::filesystem::path targetModel = "/tmp/wtf.obj";
	boost::filesystem::path executablePath = boost::filesystem::system_complete(boost::filesystem::path(argv[0]));
	boost::filesystem::path rootPath = executablePath.parent_path().parent_path();

	prtCtx = new PRTContext(rootPath, PRT_INSTALL_PATH, "CityEngAdvFx", "", prt::LogLevel::LOG_DEBUG);
	if (prtCtx->good())
		run(targetModel);
	delete prtCtx;
    return 0;
}
