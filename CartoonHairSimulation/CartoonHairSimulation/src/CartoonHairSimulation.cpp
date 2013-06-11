/*
-----------------------------------------------------------------------------
Filename:    CartoonHairSimulation.cpp
-----------------------------------------------------------------------------


This source file is generated by the
   ___                   _              __    __ _                  _ 
  /___\__ _ _ __ ___    /_\  _ __  _ __/ / /\ \ (_)______ _ _ __ __| |
 //  // _` | '__/ _ \  //_\\| '_ \| '_ \ \/  \/ / |_  / _` | '__/ _` |
/ \_// (_| | | |  __/ /  _  \ |_) | |_) \  /\  /| |/ / (_| | | | (_| |
\___/ \__, |_|  \___| \_/ \_/ .__/| .__/ \/  \/ |_/___\__,_|_|  \__,_|
      |___/                 |_|   |_|                                 
      Ogre 1.8.x Application Wizard for VC10 (May 2012)
      https://bitbucket.org/jacmoe/ogreappwizards
-----------------------------------------------------------------------------
*/

#include "stdafx.h"

#include "CartoonHairSimulation.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "../res/resource.h"
#endif

//-------------------------------------------------------------------------------------
//taken from http://www.ogre3d.org/tikiwiki/tiki-index.php?page=RetrieveVertexData
//according to the terms listed on this webpage - the code is in the public domain
//this can be used to get the vertices from the mesh
void getMeshInformation(const Ogre::MeshPtr mesh,
                        size_t &vertex_count,
                        Ogre::Vector3* &vertices,
                        size_t &index_count,
                        unsigned long* &indices,
                        const Ogre::Vector3 &position,
                        const Ogre::Quaternion &orient,
                        const Ogre::Vector3 &scale)
{
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;
 
    vertex_count = index_count = 0;
 
    // Calculate how many vertices and indices we're going to need
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }
        // Add the indices
        index_count += submesh->indexData->indexCount;
    }
 
    // Allocate space for the vertices and indices
    vertices = new Ogre::Vector3[vertex_count];
    indices = new unsigned long[index_count];
 
    added_shared = false;
 
    // Run through the submeshes again, adding the data into the arrays
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
 
        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
 
        if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }
 
            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
 
            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
 
            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
 
            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //Ogre::Real* pReal;
            float* pReal;
 
            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);
                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
                vertices[current_offset + j] = (orient * (pt * scale)) + position;
            }
 
            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }
 
        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
 
        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
 
        unsigned long* pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);
 
        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;
 
        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
                                          static_cast<unsigned long>(offset);
            }
        }
 
        ibuf->unlock();
        current_offset = next_offset;
    }
}

//http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Basic+Tutorial+7
CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID)
{
    switch (buttonID)
    {
    case OIS::MB_Left:
        return CEGUI::LeftButton;
 
    case OIS::MB_Right:
        return CEGUI::RightButton;
 
    case OIS::MB_Middle:
        return CEGUI::MiddleButton;
 
    default:
        return CEGUI::LeftButton;
    }
}


CartoonHairSimulation::CartoonHairSimulation(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mTrayMgr(0),
    mCameraMan(0),
    mDetailsPanel(0),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
	m_cameraControl(true),
	m_physicsEnabled(false)
{
	mWorld = NULL;
	mSoftBodySolver = NULL;
	mConstraintSolver = NULL;
	mBroadphase = NULL;
	mDispatcher = NULL;
	mCollisionConfig = NULL;
	m_hairModel = NULL;
	m_renderer = NULL;
	m_headRigidBody = NULL;
	m_edgeMaterial = NULL;
	m_bendingMaterial = NULL;
	m_torsionMaterial = NULL;
	m_idBufferListener = NULL;
}

//-------------------------------------------------------------------------------------
CartoonHairSimulation::~CartoonHairSimulation(void)
{
	if(m_renderer)
	{
		CEGUI::OgreRenderer::destroySystem();
	}

    if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;

    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);

	delete mRoot;

	if(m_hairModel)
	{
		delete m_hairModel;
	}

	if(m_idBufferListener)
	{
		delete m_idBufferListener;
	}

	//clean up physics
	if(mWorld)
	{
		if(m_headRigidBody)
		{
			mWorld->removeRigidBody(m_headRigidBody);
			delete m_headRigidBody->getCollisionShape();
			delete m_headRigidBody->getMotionState();
			delete m_headRigidBody;
		}

		if(m_edgeMaterial)
		{
			delete m_edgeMaterial;
			delete m_bendingMaterial;
			delete m_torsionMaterial;
			delete m_stictionMaterial;
		}

		delete mWorld;
		delete mSoftBodySolver;
		delete mConstraintSolver;
		delete mBroadphase;
		delete mDispatcher;
		delete mCollisionConfig;
	}

	//TO DO: add any clean up you need here
}

//-------------------------------------------------------------------------------------
bool CartoonHairSimulation::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "CartoonHairSimulation Render Window");

        // Let's add a nice window icon
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        HWND hwnd;
        mWindow->getCustomAttribute("WINDOW", (void*)&hwnd);
        LONG iconID   = (LONG)LoadIcon( GetModuleHandle(0), MAKEINTRESOURCE(IDI_APPICON) );
        SetClassLong( hwnd, GCL_HICON, iconID );
#endif
        return true;
    }
    else
    {
        return false;
    }
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(-19,-2.4,-12));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,0));
    mCamera->setNearClipDistance(0.1);
	mCamera->setFarClipDistance(1000);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
	mCameraMan->setTopSpeed(4.0f);
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createFrameListener(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->hideCursor();

    // create a params panel for displaying sample details
    Ogre::StringVector items;
    items.push_back("cam.pX");
    items.push_back("cam.pY");
    items.push_back("cam.pZ");
    items.push_back("");
    items.push_back("cam.oW");
    items.push_back("cam.oX");
    items.push_back("cam.oY");
    items.push_back("cam.oZ");
    items.push_back("");
    items.push_back("Filtering");
    items.push_back("Poly Mode");

    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");
    mDetailsPanel->hide();

    mRoot->addFrameListener(this);
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool CartoonHairSimulation::setup(void)
{
	//setup physics
	mCollisionConfig = new btSoftBodyRigidBodyCollisionConfiguration();
	mDispatcher = new btCollisionDispatcher(mCollisionConfig);
	mBroadphase = new btDbvtBroadphase();
	mConstraintSolver = new btSequentialImpulseConstraintSolver();
	mSoftBodySolver = new btDefaultSoftBodySolver();
	mWorld = new btSoftRigidDynamicsWorld(mDispatcher,mBroadphase,mConstraintSolver,mCollisionConfig,mSoftBodySolver);
	mWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

    mRoot = new Ogre::Root(mPluginsCfg);

    setupResources();

    bool carryOn = configure();
    if (!carryOn) return false;

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();

	// Create the scene
    createScene();

	createFrameListener();

    return true;
};
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createScene(void)
{
	//setup debug drawer
	m_debugDrawer = new DebugDrawer(mSceneMgr);
	mWorld->setDebugDrawer(m_debugDrawer);

	//setup the gui
	//http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Basic+Tutorial+7
	m_renderer = &CEGUI::OgreRenderer::bootstrapSystem();
	CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
	CEGUI::Font::setDefaultResourceGroup("Fonts");
	CEGUI::Scheme::setDefaultResourceGroup("Schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
	CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

	CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
	CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook","MouseArrow");

	m_guiRoot = CEGUI::WindowManager::getSingleton().loadWindowLayout("cartoonhair.layout"); 
	CEGUI::System::getSingleton().setGUISheet(m_guiRoot);

	//link up sliders
	m_edgeSlider = (CEGUI::Slider*) m_guiRoot->getChildRecursive("Root/springWindow/edgeSlider");
	m_bendingSlider = (CEGUI::Slider*) m_guiRoot->getChildRecursive("Root/springWindow/bendingSlider");
	m_torsionSlider = (CEGUI::Slider*) m_guiRoot->getChildRecursive("Root/springWindow/torsionSlider");
	m_stictionSlider = (CEGUI::Slider*) m_guiRoot->getChildRecursive("Root/springWindow/stictionSlider");

	m_aSlider = (CEGUI::Slider*) m_guiRoot->getChildRecursive("Root/curveWindow/aValue");
	m_bSlider = (CEGUI::Slider*) m_guiRoot->getChildRecursive("Root/curveWindow/bValue");
	m_cSlider = (CEGUI::Slider*) m_guiRoot->getChildRecursive("Root/curveWindow/cValue");

	//setup spring materials
	m_edgeMaterial = new btSoftBody::Material();
	m_bendingMaterial = new btSoftBody::Material();
	m_torsionMaterial = new btSoftBody::Material();
	m_stictionMaterial = new btSoftBody::Material();

	m_edgeMaterial->m_kLST = 1.0;
	m_bendingMaterial->m_kLST = 0.01;
	m_torsionMaterial->m_kLST = 0.01;

	m_edgeSlider->setCurrentValue(m_edgeMaterial->m_kLST);
	m_bendingSlider->setCurrentValue(m_bendingMaterial->m_kLST);
	m_torsionSlider->setCurrentValue(m_torsionMaterial->m_kLST);

	m_aSlider->setCurrentValue(-13.9+m_aSlider->getMaxValue()/2);
	m_bSlider->setCurrentValue(4.9+m_bSlider->getMaxValue()/2);
	m_cSlider->setCurrentValue(6.4+m_cSlider->getMaxValue()/2);

	//setup background colour
	if(mWindow->getNumViewports())
	{
		mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0.39,0.58,0.92));
	}

	Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

	//model by http://www.turbosquid.com/FullPreview/Index.cfm/ID/403363
	Ogre::Entity* head = mSceneMgr->createEntity("Head", "oldheadbust.mesh");
	Ogre::Entity* test = mSceneMgr->createEntity("Cube","cube.mesh");

	//based on http://www.ogre3d.org/tikiwiki/tiki-index.php?page=RetrieveVertexData
	size_t vertexCount, indexCount;
	Ogre::Vector3* vertices;
	unsigned long* indices;

	getMeshInformation(head->getMesh(),
		vertexCount,
		vertices,
		indexCount,
		indices,
		Ogre::Vector3::ZERO,
		Ogre::Quaternion::IDENTITY,
		Ogre::Vector3::UNIT_SCALE);

	//create collision rigid body - based upon https://bitbucket.org/alexeyknyshev/ogrebullet/src/555c70e80bf4/Collisions/src/Utils/OgreBulletCollisionsMeshToShapeConverter.cpp?at=master
	btConvexHullShape* complexHull = new btConvexHullShape((btScalar*) &vertices[0].x,vertexCount,sizeof(Ogre::Vector3));

	btDefaultMotionState *headMotionState = new btDefaultMotionState(
		btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));
	btRigidBody::btRigidBodyConstructionInfo headConstructionInfo(0,headMotionState,complexHull,btVector3(0,0,0));

	m_headRigidBody = new btRigidBody(headConstructionInfo);

	mWorld->addRigidBody(m_headRigidBody,BODY_GROUP, BODY_GROUP | HAIR_GROUP);

	//clean up
	delete[] vertices;
	delete[] indices;

	float a = m_aSlider->getCurrentValue()-(m_aSlider->getMaxValue()/2);
	float b = m_bSlider->getCurrentValue()-(m_bSlider->getMaxValue()/2);
	float c = m_cSlider->getCurrentValue()-(m_cSlider->getMaxValue()/2);

	//setup dynamic hair
	HairParameters param;
	param.directory = "../hair/";
	param.animation = "hairanimation.xml";
	param.sceneMgr = mSceneMgr;
	param.world = mWorld;
	param.edgeMaterial = m_edgeMaterial;
	param.bendingMaterial = m_bendingMaterial;
	param.torsionMaterial = m_torsionMaterial;
	param.stictionMaterial = NULL;
	param.camera = mCamera;
	param.a = a;
	param.b = b;
	param.c = c;
	param.maxStictionConnections = 5;
	param.stictionThreshold =  0.5f;
	param.window = mWindow;

	m_hairModel = new HairModel(param);
	m_idBufferListener = new IdBufferRenderTargetListener(mSceneMgr,m_hairModel,m_debugDrawer);

	//m_hairModel->getIdBufferTexture()->addListener(m_idBufferListener);

	//setup mini-screen so we can view the id buffer - http://www.ogre3d.org/tikiwiki/Intermediate+Tutorial+7#Creating_the_render_textures
	/*Ogre::Rectangle2D *smallScreen = new Ogre::Rectangle2D(true);
	smallScreen->setCorners(0.5f,-0.5f,1.0f,-1.0f);
	smallScreen->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));
	Ogre::SceneNode *smallScreenNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("smallScreenNode");
	smallScreenNode->attachObject(smallScreen);

	smallScreen->setMaterial("IETCartoonHair/SmallScreenMaterial");*/



	//if reduce to the correct size in the simulation - the collision becomes inaccurate - instead scaling the simulation
	//http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Scaling_The_World
	mWorld->setGravity(mWorld->getGravity()*m_hairModel->getSimulationScale());

	// Set ambient light
	//mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Ogre::Light* l = mSceneMgr->createLight("MainLight");
	l->setPosition(-19,-2.4,-12);

	Ogre::SceneNode *child = headNode->createChildSceneNode();
	child->attachObject(test);
	child->setPosition(l->getPosition());
	child->setScale(0.01,0.01,0.01);

	//line everything to the head node
	headNode->attachObject(head);
	headNode->attachObject(m_debugDrawer->getLinesManualObject());
	headNode->attachObject(m_hairModel->getHairManualObject());
	headNode->attachObject(m_hairModel->getNormalsManualObject());
	headNode->attachObject(m_hairModel->getEdgeManualObject());

	head->setRenderQueueGroupAndPriority(Ogre::RENDER_QUEUE_1,1);
	m_hairModel->getHairManualObject()->setRenderQueueGroupAndPriority(Ogre::RENDER_QUEUE_2,2);
	m_hairModel->getEdgeManualObject()->setRenderQueueGroupAndPriority(Ogre::RENDER_QUEUE_3,3);
	
	//headNode->attachObject(m_hairModel->getEdgeBillboardSet());
}
//-------------------------------------------------------------------------------------
bool CartoonHairSimulation::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

	CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);

    mTrayMgr->frameRenderingQueued(evt);

    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
        if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
        {
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }

	float timestep = evt.timeSinceLastFrame;
	//physics update - note:  must (timeStep < maxSubSteps*fixedTimeStep) == true according to http://bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_The_World
	//if odd physics problems arise - consider adding parameters for maxSubstep and fixedTimeStep

	if(m_physicsEnabled)
	{
		mWorld->stepSimulation(timestep);
		//m_hairModel->updateStictionSegments();
		m_hairModel->updateAnchors(timestep);
	}

	m_hairModel->updateManualObject();

	m_edgeMaterial->m_kLST = m_edgeSlider->getCurrentValue();
	m_bendingMaterial->m_kLST = m_bendingSlider->getCurrentValue();
	m_torsionMaterial->m_kLST = m_torsionSlider->getCurrentValue();

	m_hairModel->setCurveValues(
		m_aSlider->getCurrentValue()-(m_aSlider->getMaxValue()/2),
		m_bSlider->getCurrentValue()-(m_bSlider->getMaxValue()/2),
		m_cSlider->getCurrentValue()-(m_cSlider->getMaxValue()/2)
		);
	

	m_debugDrawer->begin();
	mWorld->debugDrawWorld();
	m_debugDrawer->end();

	//TO DO: add any methods you would like to be called

    return true;
}

//-------------------------------------------------------------------------------------

bool CartoonHairSimulation::keyPressed( const OIS::KeyEvent &arg )
{

    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
	else if (arg.key == OIS::KC_H)
	{
		if(m_hairModel->getHairManualObject()->isVisible())
		{
			m_hairModel->getHairManualObject()->setVisible(false);
		}
		else
		{
			m_hairModel->getHairManualObject()->setVisible(true);
		}
	}
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
	else if (arg.key == OIS::KC_P)
	{
		if(m_debugDrawer->getLinesManualObject()->isVisible())
		{
			m_debugDrawer->getLinesManualObject()->setVisible(false);
		}
		else
		{
			m_debugDrawer->getLinesManualObject()->setVisible(true);
		}
	}
	else if(arg.key == OIS::KC_N)
	{
		if(m_hairModel->getNormalsManualObject()->isVisible())
		{
			m_hairModel->getNormalsManualObject()->setVisible(false);
		}
		else
		{
			m_hairModel->getNormalsManualObject()->setVisible(true);
		}
	}
	else if(arg.key == OIS::KC_M)
	{
		if(m_cameraControl)
		{
			m_cameraControl = false;
			//mTrayMgr->showCursor();
		}
		else
		{
			m_cameraControl = true;
			//mTrayMgr->hideCursor();
		}
	}
	else if(arg.key == OIS::KC_L)
	{
		m_physicsEnabled = !m_physicsEnabled;
	}

	if(m_cameraControl)
	{
		mCameraMan->injectKeyDown(arg);
	}
	else
	{
		CEGUI::System &sys = CEGUI::System::getSingleton();
		sys.injectKeyDown(arg.key);
		sys.injectChar(arg.text);
	}

    return true;
}

bool CartoonHairSimulation::keyReleased( const OIS::KeyEvent &arg )
{
	if(m_cameraControl)
	{
		mCameraMan->injectKeyUp(arg);
	}
	else
	{
		CEGUI::System::getSingleton().injectKeyUp(arg.key);
	}

    return true;
}

bool CartoonHairSimulation::mouseMoved( const OIS::MouseEvent &arg )
{
    //mTrayMgr->injectMouseMove(arg);
	if(m_cameraControl)
	{
		mCameraMan->injectMouseMove(arg);
	}
	else
	{
		CEGUI::System &sys = CEGUI::System::getSingleton();
		sys.injectMouseMove(arg.state.X.rel,arg.state.Y.rel);
		if(arg.state.Z.rel)
		{
			sys.injectMouseWheelChange(arg.state.Z.rel / 120.0f);
		}
	}
    return true;
}

bool CartoonHairSimulation::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    //mTrayMgr->injectMouseDown(arg, id);

	if(m_cameraControl)
	{
		mCameraMan->injectMouseDown(arg, id);
	}
	else
	{
		CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));
	}

    return true;
}

bool CartoonHairSimulation::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    //mTrayMgr->injectMouseUp(arg, id);

	if(m_cameraControl)
	{
		mCameraMan->injectMouseUp(arg, id);
	}
	else
	{
		CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));
	}

    return true;
}

//Adjust mouse clipping area
void CartoonHairSimulation::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void CartoonHairSimulation::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        CartoonHairSimulation app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
