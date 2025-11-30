/*
 * File:   xbpTest.cpp
 * Author: Xboxplayer<xboxplayer9889@gmail.com>
 *
 * Created on 2025. november 29., 10:58
 */

#include "xbpTest.h"

#include "../../CommonInterfaces/CommonRigidBodyBase.h"
#include "../../CommonInterfaces/CommonParameterInterface.h"

#include "../../Importers/ImportMeshUtility/b3ImportMeshUtility.h"
#include "../../Utils/b3BulletDefaultFileIO.h"
#include "../../OpenGLWindow/GLInstanceGraphicsShape.h"

//const char* groundfile = "examples/myProjects/3ddata/spin.obj";

struct xbpTest: public CommonRigidBodyBase
{
    int m_options;

    xbpTest(struct GUIHelperInterface* helper, int options);
    virtual ~xbpTest() {}
    virtual void initPhysics();
    virtual void renderScene();
    void resetCamera();
    
private:
    std::string selectedobj = "";
    void setSelectedObj(const char* fn);
    const char* getSelectedObj();
    static void buttonPressed( int buttonId, bool buttonState, void* userPointer );
    static void comboChanged(int combobox, const char* item, void* userPointer);
};


xbpTest::xbpTest(struct GUIHelperInterface* helper, int options) : CommonRigidBodyBase(helper) {
    
    this->m_options = options;
    
    // Setup parameters
    char* x[2] = { "examples/myProjects/3ddata/spin.obj", "data/samurai_monastry.obj" };
    setSelectedObj(x[0]);

    ComboBoxParams cbp;
    cbp.m_items = (const char**)&x;
    cbp.m_numItems = 2;
    cbp.m_startItem = 0;
    cbp.m_userPointer = this;
    cbp.m_callback = xbpTest::comboChanged;
    m_guiHelper->getParameterInterface()->registerComboBox(cbp);
    
    ButtonParams bp("Load & Restart",0,false);
    bp.m_userPointer = this;
    bp.m_callback = xbpTest::buttonPressed;
    m_guiHelper->getParameterInterface()->registerButtonParameter(bp);
    
    //increase default shadow map size because of smaurai_monastry is much bigger (TODO:calc the correct size)
    m_guiHelper->getRenderInterface()->setShadowMapWorldSize(100);
}

void xbpTest::buttonPressed( int buttonId, bool buttonState, void* userPointer ){
    if (userPointer) {
        xbpTest* self = ((xbpTest*)userPointer);
        self->exitPhysics();
        self->m_guiHelper->removeAllGraphicsInstances();
        self->initPhysics();
    }
}
void xbpTest::comboChanged(int combobox, const char* item, void* userPointer) {
    if (userPointer) {
        ((xbpTest*)userPointer)->setSelectedObj( item );
    }
}

void xbpTest::initPhysics() {
    this->m_guiHelper->setUpAxis(1.0f);
    this->createEmptyDynamicsWorld();
        
    float scale = 3.0f;
    btScalar mass = 0.0f;
    btVector3 position = btVector3( 0.0f,0.0f,0.0f );
    
    // import obj example: examples/Importers/ImportMeshUtility/b3ImportMeshUtility
    b3ImportMeshData meshdata;
    b3BulletDefaultFileIO fileio;
    if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal( getSelectedObj(), meshdata, &fileio ) ) {
        
        btTriangleMesh* mesh = new btTriangleMesh();
        for (int i=0; i<meshdata.m_gfxShape->m_numIndices; i+=3) {
            btVector3 v[3];
            for (int j=0; j<3; j++)
                v[j] = btVector3( 
                    meshdata.m_gfxShape->m_vertices->at( meshdata.m_gfxShape->m_indices->at(i+j) ).xyzw[0], 
                    meshdata.m_gfxShape->m_vertices->at( meshdata.m_gfxShape->m_indices->at(i+j) ).xyzw[1], 
                    meshdata.m_gfxShape->m_vertices->at( meshdata.m_gfxShape->m_indices->at(i+j) ).xyzw[2] 
                    );
            mesh->addTriangle(v[0], v[1], v[2]);
        }
        btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(mesh, true);
        
        shape->setLocalScaling(btVector3(scale, scale, scale));
        this->m_collisionShapes.push_back(shape);
        btTransform transf;
        transf.setIdentity();
        transf.setOrigin(position);
        btVector4 color(1.0f, 1.0f, 1.0f, 1.0f);
        btRigidBody* ground = createRigidBody(mass, transf, shape, color );
    }
    else {
        btBoxShape* shape = createBoxShape( btVector3(10.0f, 0.0f, 10.0f));
        shape->setLocalScaling(btVector3(scale, scale, scale));
        this->m_collisionShapes.push_back(shape);
        btTransform transf;
        transf.setIdentity();
        transf.setOrigin(position);
        btVector4 color(1.0f, 1.0f, 1.0f, 1.0f);
        btRigidBody* ground = createRigidBody(mass, transf, shape, color );
    }

    btSphereShape* shape = new btSphereShape(btScalar(1.7f));
    shape->setLocalScaling(btVector3(1.0f, 1.0f, 1.0f));
    this->m_collisionShapes.push_back(shape);
    btTransform transf;
    transf.setIdentity();
    transf.setOrigin(btVector3(-4.0f, 10.0f, 6.0f));
    btVector4 color(1.0f, 1.0f, 1.0f, 1.0f);
    mass = 1.0f;
    btRigidBody* body = createRigidBody(mass, transf, shape, color);
    
    this->m_guiHelper->autogenerateGraphicsObjects(this->m_dynamicsWorld);
}

void xbpTest::setSelectedObj(const char* fn) {
    this->selectedobj = fn;
}

const char* xbpTest::getSelectedObj() {
    return this->selectedobj.c_str();    
}

void xbpTest::renderScene(){
    CommonRigidBodyBase::renderScene();
}

void xbpTest::resetCamera(){
    m_guiHelper->resetCamera(12.0f, -90.0f,-20.0f, 0.0f,3.0f,0.0f);
}

CommonExampleInterface* XBP_myTestCreateFunc(CommonExampleOptions& options)
{
	return new xbpTest(options.m_guiHelper, options.m_option);
}

B3_STANDALONE_EXAMPLE(XBP_myTestCreateFunc)
