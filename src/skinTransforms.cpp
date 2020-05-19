
#include "skinTransforms.h"

MTypeId     skinTransforms::id(0x00080030);
MObject		skinTransforms::matrix;
MObject		skinTransforms::bindPreMatrix;
MObject		skinTransforms::weights;
MObject		skinTransforms::points;
MObject		skinTransforms::preBind;

MObject		skinTransforms::weightsList;

MObject		skinTransforms::outputs;

MMatrix getMatrixFromTriangle(MVector vectorA, MVector vectorB, MVector vectorC)
{
	MVector ba = vectorB - vectorA;
	MVector ca = vectorC - vectorA;
	MVector normal_vector = (ca.normal() ^ ba.normal()).normal();
	MVector tangent_vector = (ba.normal() ^ normal_vector).normal();
	MVector cross_vector = (tangent_vector ^ normal_vector).normal();
	MVector position = vectorA;
	MMatrix matrix;
	matrix[0][0] = tangent_vector[0]; matrix[0][1] = tangent_vector[1]; matrix[0][2] = tangent_vector[2];
	matrix[1][0] = normal_vector[0];  matrix[1][1] = normal_vector[1];  matrix[1][2] = normal_vector[2];
	matrix[2][0] = cross_vector[0];   matrix[2][1] = cross_vector[1];   matrix[2][2] = cross_vector[2];
	matrix[3][0] = position[0];       matrix[3][1] = position[1];       matrix[3][2] = position[2];
	return matrix;
}

skinTransforms::skinTransforms()
	: cachedValueIsValid(false)
{
}
skinTransforms::~skinTransforms()
{
}

void* skinTransforms::creator()
{
	return new skinTransforms();
}
MStatus skinTransforms::initialize()
{
	MFnMatrixAttribute  mAttr;
	MFnNumericAttribute nAttr;
	MFnCompoundAttribute cmpAttr;
	MFnTypedAttribute tAttr;

	MStatus	stat;
	//output = mAttr.create("output", "out", MFnMatrixAttribute::kFloat);
	matrix = mAttr.create("matrix", "matrix");
	mAttr.setArray(true);
	//mAttr.setUsesArrayDataBuilder(true);
	mAttr.setConnectable(true);
	stat = addAttribute(matrix);

	bindPreMatrix = mAttr.create("bindPreMatrix", "bindPreMatrix");
	mAttr.setArray(true);
	//mAttr.setUsesArrayDataBuilder(true);
	mAttr.setConnectable(true);
	stat = addAttribute(bindPreMatrix);

	weights = tAttr.create("weights", "weights", MFnData::kDoubleArray);
	points = tAttr.create("points", "points", MFnData::kDoubleArray);
	preBind = mAttr.create("preBindMatrix", "pbm");

	weightsList = cmpAttr.create("weightsList", "weightsList", &stat);
	cmpAttr.setArray(true);
	cmpAttr.addChild(weights);
	cmpAttr.addChild(points);
	cmpAttr.addChild(preBind);
	cmpAttr.setReadable(true);
	cmpAttr.setUsesArrayDataBuilder(true);
	// cmpAttr.setIndexMatters(true);
	addAttribute(weightsList);


	outputs = mAttr.create("outputs", "outputs");
	mAttr.setWritable(false);
	mAttr.setArray(true);
	mAttr.setUsesArrayDataBuilder(true);
	mAttr.setConnectable(true);
	stat = addAttribute(outputs);

	stat = attributeAffects(matrix, outputs);
	stat = attributeAffects(bindPreMatrix, outputs);
	stat = attributeAffects(weightsList, outputs);
	return MStatus::kSuccess;
}
MStatus skinTransforms::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus stat;

	// get the influence transforms
	//
	MArrayDataHandle transformsHandle = data.inputArrayValue(matrix);
	int numTransforms = transformsHandle.elementCount();
	if (numTransforms == 0) {
		return MS::kSuccess;
	}
	MMatrixArray transforms;
	for (int i = 0; i < numTransforms; ++i) {
		transformsHandle.jumpToArrayElement(i);
		transforms.append(transformsHandle.inputValue().asMatrix());
	}
	MArrayDataHandle bindHandle = data.inputArrayValue(bindPreMatrix);
	if (bindHandle.elementCount() > 0) {
		for (int i = 0; i < numTransforms; ++i) {
			bindHandle.jumpToArrayElement(i);
			transforms[i] = bindHandle.inputValue().asMatrix() * transforms[i];
		}
	}
	if (!cachedValueIsValid)
	{
		cout << "getting weights ans points" << endl;
		cachedWeights.clear();
		cachedPoints.clear();
		cachedOffsets.clear();
		MArrayDataHandle weightListHandle = data.inputArrayValue(weightsList);
		int weightsCount = weightListHandle.elementCount();
		if (weightsCount == 0) {
			// no weights - nothing to do
			return MS::kSuccess;
		}
		for (int i = 0; i < weightsCount; i++)
		{
			weightListHandle.jumpToArrayElement(i);
			MDataHandle weightsh = weightListHandle.inputValue().child(weights);
			MDataHandle pointsh = weightListHandle.inputValue().child(points);
			MDataHandle preBindh = weightListHandle.inputValue().child(preBind);
			MFnDoubleArrayData weightsd(weightsh.data());
			MDoubleArray w = weightsd.array();
			MFnDoubleArrayData pointsd(pointsh.data());
			MDoubleArray p = pointsd.array();
			cachedWeights.push_back(w);
			cachedPoints.push_back(p);
			MMatrix preBindm = preBindh.asMatrix();
			MVector vectorA(p[0], p[1], p[2]);
			MVector vectorB(p[3], p[4], p[5]);
			MVector vectorC(p[6], p[7], p[8]);
			MMatrix baseMatrix = getMatrixFromTriangle(vectorA, vectorB, vectorC);
			cachedOffsets.push_back(preBindm*baseMatrix.inverse());
		}
		cachedValueIsValid = true;
		cout << "weights ans points cached" << endl;
	}
	MPointArray triangle;
	MArrayDataHandle outputs_h = data.outputArrayValue(outputs);
	MArrayDataBuilder builder = outputs_h.builder(&stat);
	MMatrix outMatrix;
	for (int i = 0; i < cachedWeights.size(); i++) {
		triangle.clear();
		int h = 0;
		for (int j = 0; j < 3; j++)
		{
			MPoint pt(cachedPoints[i][0 + (3 * j)], cachedPoints[i][1 + (3 * j)], cachedPoints[i][2 + (3 * j)], 1);
			MPoint skinned;
			// get the weights for this point
			// compute the skinning
			for (int k = 0; k < numTransforms; ++k) {
				skinned += (pt * transforms[k]) * cachedWeights[i][h];
				h++;
			}
			triangle.append(skinned);
		}
		MVector vectorA = triangle[0];
		MVector vectorB = triangle[1];
		MVector vectorC = triangle[2];
		MDataHandle outHandle = builder.addElement(i);
		outMatrix = getMatrixFromTriangle(vectorA, vectorB, vectorC);
		outHandle.setMMatrix(cachedOffsets[i]*outMatrix);

	}
	stat = outputs_h.set(builder);
	outputs_h.setAllClean();
	return stat;
}

MStatus skinTransforms::setDependentsDirty(const MPlug& plug, MPlugArray& plugArray)
{
	if (plug == points || plug == weights)
	{
		cachedValueIsValid = false;
	}
	return MPxNode::setDependentsDirty(plug, plugArray);
}
// Turn this define on to see the evaluation work in Serial/Parallel modes
// #define DO_PRE_EVAL
MStatus skinTransforms::preEvaluation(const  MDGContext& context, const MEvaluationNode& evaluationNode)
{
#ifdef DO_PRE_EVAL
	MStatus status;
	// We use m_CachedValueIsValid only for normal context
	if (!context.isNormal())
		return MStatus::kFailure;
	if ((evaluationNode.dirtyPlugExists(points, &status) && status) ||
		(evaluationNode.dirtyPlugExists(weights, &status) && status))
	{
		cachedValueIsValid = false;
	}
#endif
	return MS::kSuccess;
}

