#ifndef UTIL_H
#define UTIL_H

#include <limits>

#include <maya/MSelectionList.h>
#include <maya/MObjectArray.h>
#include <maya/MFnTransform.h>

template <typename T> int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}


/// \brief	This function is a simple utility function that retrieves the selected 
///		objects of the requested type. 
/// \param	objects	-	the returned list of objects
/// \param	type	-	the type of objects you want returned
///
inline void getSelected(MObjectArray& objects, MFn::Type type) {

	// get the current selection list from maya
	MSelectionList selected;
	MGlobal::getActiveSelectionList(selected);

	// iterate through all selected items
	for (unsigned int i = 0; i<selected.length(); ++i)
	{
		MObject obj;

		// returns the i'th selected dependency node
		selected.getDependNode(i, obj);

		// if the selected object is of the type we are looking for
		if (obj.hasFn(type)) {
			objects.append(obj);
		}
		else
			// if the selected object is a transform, check it's kids
			if (obj.hasFn(MFn::kTransform)) {

				MFnTransform fn(obj);

				// loop through each child of the transform
				for (unsigned int j = 0; j< fn.childCount(); ++j)
				{
					// retrieve the j'th child of the transform node
					MObject child = fn.child(j);

					// if the child is of the type we are looking for,
					// append it to the list
					if (child.hasFn(type))
						objects.append(child);
				}
			}
	}
}

inline float chop(float value)
{
	if (sgn(value) != sgn(value + FLT_EPSILON) || sgn(value) != sgn(value - FLT_EPSILON))
		return 0.0f;
	else
		return value;
}

#endif