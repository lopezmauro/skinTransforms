#include <maya/MTypeId.h> 
#include <maya/MMatrixArray.h>
#include <maya/MStringArray.h>
#include <maya/MPxSkinCluster.h> 
#include <maya/MItGeometry.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MArrayDataBuilder.h>

#include <maya/MFnMatrixData.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MDoubleArray.h>
#include <vector> // for value array operations 
//#include <triangles.h>
//#include <transformation.h>
//#include <eigenToMaya.h>

class transformSkin : public MPxNode

{
public:
						transformSkin();
	virtual             ~transformSkin();
	static  void*   creator();
	static  MStatus initialize();
	
	MStatus	compute(const MPlug& plug, MDataBlock& data) override;
	virtual MStatus     setDependentsDirty(const MPlug& plug, MPlugArray& plugArray);
	virtual MStatus     preEvaluation(const  MDGContext& context, const MEvaluationNode& evaluationNode);
public:

	static	MTypeId		id;
	static	MObject		matrix;
	static	MObject		bindPreMatrix;
	static	MObject		weights;
	static	MObject		points;
	static	MObject		preBind;
	static	MObject		weightsList;

	static	MObject		outputs;

private:
	bool cachedValueIsValid;
	std::vector<MDoubleArray> cachedWeights;
	std::vector<MDoubleArray> cachedPoints;
	std::vector<MMatrix> cachedOffsets;
};