#include "myTest.h"

#include "../../CommonInterfaces/CommonRigidBodyBase.h"

struct myTest: public CommonRigidBodyBase
{
	int m_options;

	myTest(struct GUIHelperInterface* helper, int options)
		: CommonRigidBodyBase(helper),
		  m_options(options)
	{
	}
	virtual ~myTest() {}
	virtual void initPhysics() {}
	virtual void renderScene() {}
	void resetCamera() {}
};

CommonExampleInterface* ET_myTestCreateFunc(CommonExampleOptions& options)
{
	return new myTest(options.m_guiHelper, options.m_option);
}

B3_STANDALONE_EXAMPLE(ET_myTestCreateFunc)
