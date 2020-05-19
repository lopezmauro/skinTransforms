#include "transformSkin.h"
#include <maya/MStreamUtils.h>
#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{
    MStatus status;

    MFnPlugin fnPlugin( obj, "None", "1.1", "Any" );
	std::cout.set_rdbuf(MStreamUtils::stdOutStream().rdbuf());
	std::cerr.set_rdbuf(MStreamUtils::stdErrorStream().rdbuf());
    status = fnPlugin.registerNode( "transformSkin",
        transformSkin::id,
        transformSkin::creator,
        transformSkin::initialize);
    CHECK_MSTATUS_AND_RETURN_IT( status );
    return MS::kSuccess;
}


MStatus uninitializePlugin( MObject obj )
{
    MStatus status;

    MFnPlugin fnPlugin( obj );

    status = fnPlugin.deregisterNode( transformSkin::id );
    CHECK_MSTATUS_AND_RETURN_IT( status );

    return MS::kSuccess;
}