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


const int BTNID_Load = 0;
const int BTNID_ResetCam = 1;

const float DEFAULT_CamDistance = 12.0f;
const float DEFAULT_Restitution = 0.0f;

struct xbpTest: public CommonRigidBodyBase
{
    int m_options;

    xbpTest(struct GUIHelperInterface* helper, int options);
    virtual ~xbpTest();
    virtual void initPhysics();
    virtual void renderScene();
    void resetCamera();
    
private:
    std::string selectedobj = "";
    btScalar zoomvalue;
    btScalar groundRvalue;
    btScalar ballRvalue;

    btRigidBody* ground;
    btRigidBody* ball;

    void setSelectedObj(const char* fn);
    const char* getSelectedObj();
    static void buttonPressed( int buttonId, bool buttonState, void* userPointer );
    static void comboChanged(int combobox, const char* item, void* userPointer);
    static void zoomChanged(float newVal, void* userPointer);
    void updatePhysicsSettings();
};


xbpTest::xbpTest(struct GUIHelperInterface* helper, int options) : CommonRigidBodyBase(helper) {
    
    this->m_options = options;
    ground = NULL;
    ball = NULL;
    // Setup parameters
    int arraylength =  0;
    char* x[] = {
        "examples/myProjects/3ddata/spin.obj", 
        "data/samurai_monastry.obj", 
        NULL };
    setSelectedObj(x[0]);
    do {
        arraylength++;
    } while (x[arraylength]!=NULL);

    ComboBoxParams cbp;
    cbp.m_items = (const char**)&x;
    cbp.m_numItems = arraylength;
    cbp.m_startItem = 0;
    cbp.m_userPointer = this;
    cbp.m_callback = xbpTest::comboChanged;
    m_guiHelper->getParameterInterface()->registerComboBox(cbp);
    
    ButtonParams bp("Load & Restart",BTNID_Load,false);
    bp.m_userPointer = this;
    bp.m_callback = xbpTest::buttonPressed;
    m_guiHelper->getParameterInterface()->registerButtonParameter(bp);
    
    this->zoomvalue = DEFAULT_CamDistance;
    SliderParams sp("Zoom", &zoomvalue);
    sp.m_minVal=0.5f;
    sp.m_maxVal=150.0f;
    sp.m_userPointer = this;
    sp.m_callback = xbpTest::zoomChanged;
    m_guiHelper->getParameterInterface()->registerSliderFloatParameter(sp);
    
    ButtonParams bp2("Reset Camera",BTNID_ResetCam,false);
    bp2.m_userPointer = this;
    bp2.m_callback = xbpTest::buttonPressed;
    m_guiHelper->getParameterInterface()->registerButtonParameter(bp2);

    this->ballRvalue = DEFAULT_Restitution;
    SliderParams sp2("Ground restitution", &groundRvalue);
    sp2.m_minVal=0.0f;
    sp2.m_maxVal=1.0f;
    m_guiHelper->getParameterInterface()->registerSliderFloatParameter(sp2);

    this->ballRvalue = DEFAULT_Restitution;
    SliderParams sp3("Ball restitution", &ballRvalue);
    sp3.m_minVal=0.0f;
    sp3.m_maxVal=1.0f;
    m_guiHelper->getParameterInterface()->registerSliderFloatParameter(sp3);

    //increase default shadow map size because of smaurai_monastry is much bigger (TODO:calc the correct size)
    m_guiHelper->getRenderInterface()->setShadowMapWorldSize(100);
}

xbpTest::~xbpTest() {
    exitPhysics();
    m_guiHelper->removeAllGraphicsInstances();
}

void xbpTest::buttonPressed( int buttonId, bool buttonState, void* userPointer ){
    if (userPointer) {
        xbpTest* self = ((xbpTest*) userPointer);
        switch (buttonId) {
            case BTNID_Load:
                self->exitPhysics();
                self->m_guiHelper->removeAllGraphicsInstances();
                self->initPhysics();
                break;
            case BTNID_ResetCam:
                self->zoomvalue=DEFAULT_CamDistance;
                self->resetCamera();
                break;
            default:
                break;
        }
    }
}
void xbpTest::comboChanged(int combobox, const char* item, void* userPointer) {
    if (userPointer) {
        ((xbpTest*)userPointer)->setSelectedObj( item );
    }
}
// TODO:may handle mousewheel too, to sync the zoom value
void xbpTest::zoomChanged(float newVal, void* userPointer) {
    if (userPointer) {
        xbpTest* self = ((xbpTest*)userPointer);
        self->m_guiHelper->getRenderInterface()->getActiveCamera()->setCameraDistance(newVal);
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
        ground = createRigidBody(mass, transf, shape, color );
    }
    else {
        btBoxShape* shape = createBoxShape( btVector3(10.0f, 0.0f, 10.0f));
        shape->setLocalScaling(btVector3(scale, scale, scale));
        this->m_collisionShapes.push_back(shape);
        btTransform transf;
        transf.setIdentity();
        transf.setOrigin(position);
        btVector4 color(1.0f, 1.0f, 1.0f, 1.0f);
        ground = createRigidBody(mass, transf, shape, color );
    }

    btSphereShape* shape = new btSphereShape(btScalar(1.7f));
    shape->setLocalScaling(btVector3(1.0f, 1.0f, 1.0f));
    this->m_collisionShapes.push_back(shape);
    btTransform transf;
    transf.setIdentity();
    transf.setOrigin(btVector3(-4.0f, 10.0f, 6.0f));
    btVector4 color(1.0f, 1.0f, 1.0f, 1.0f);
    mass = 1.0f;
    ball = createRigidBody(mass, transf, shape, color);
    
    this->m_guiHelper->autogenerateGraphicsObjects(this->m_dynamicsWorld);
}

void xbpTest::setSelectedObj(const char* fn) {
    this->selectedobj = fn;
}

const char* xbpTest::getSelectedObj() {
    return this->selectedobj.c_str();    
}

void xbpTest::updatePhysicsSettings(){
    //update restitutions
    if (this->ground)
        this->ground->setRestitution(this->groundRvalue);
    if (this->ball)
        this->ball->setRestitution(this->ballRvalue);
}

void xbpTest::renderScene(){
    updatePhysicsSettings();
    CommonRigidBodyBase::renderScene();
}

void xbpTest::resetCamera(){
    m_guiHelper->resetCamera(DEFAULT_CamDistance, -90.0f,-20.0f, 0.0f,3.0f,0.0f);
}

CommonExampleInterface* XBP_myTestCreateFunc(CommonExampleOptions& options)
{
	return new xbpTest(options.m_guiHelper, options.m_option);
}

B3_STANDALONE_EXAMPLE(XBP_myTestCreateFunc)
